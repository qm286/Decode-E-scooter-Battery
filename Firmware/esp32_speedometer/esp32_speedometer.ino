/*
  Hall Sensor Speed Measurement Module
  ------------------------------------
  Purpose:
  This code is designed to measure the rotational activity detected by a Hall sensor
  connected to an ESP32, then estimate vehicle speed in km/h in near real time.

  What this code does:
  1. Reads the Hall sensor signal from a GPIO pin.
  2. Samples the signal at a fixed interval of 1 ms.
  3. For each 1 ms sampling window, measures how long the signal stays HIGH.
  4. Converts that 1 ms window into a single digital state:
       - 1 if the signal was HIGH for more than half of the window
       - 0 otherwise
  5. Stores these sampled states in a circular buffer representing the latest 100 ms.
  6. Every 100 ms, analyzes the buffer to count signal transitions.
  7. Converts transitions into pulse count.
  8. Converts pulse count into speed in km/h using an experimentally determined factor.
  9. Applies a sliding average over the most recent speed estimates to reduce noise.
 10. Prints the smoothed speed result to the Serial Monitor.

  Why this approach is used:
  The Hall sensor signal may be noisy, unstable, or too fast to interpret reliably
  with a simple occasional digitalRead(). Instead of reading the pin only once per
  interval, this code observes the signal continuously within each 1 ms window and
  estimates the dominant state of that window. This makes the measurement more robust
  against jitter, brief glitches, or fast toggling.

  Signal processing strategy:
  - Sampling layer:
      The code compresses each 1 ms time slice into one representative binary state.
  - Analysis layer:
      The last 100 ms of sampled states are examined to count transitions.
  - Estimation layer:
      Transition count is converted into pulse count, then into speed.
  - Smoothing layer:
      A moving average over several recent speed values improves display stability.

  Input:
  - Hall sensor digital signal on HALL_PIN

  Output:
  - Pulse count over the last 100 ms
  - Averaged speed in km/h through Serial output

  Important assumptions:
  - One full pulse corresponds to two signal transitions.
  - The speed conversion factor (3.66 / 2 in this code path) is based on your measured
    or calibrated system behavior.
  - The Hall sensor output is digital and readable through GPIO.
  - A 100 ms update interval provides a reasonable tradeoff between responsiveness
    and stability.

  Typical use case:
  This is suitable for embedded speedometer development, wheel rotation tracking,
  or any Hall-sensor-based motion measurement system where real-time speed feedback
  is needed and raw pulse timing may be noisy.

  Notes:
  - BUFFER_SIZE stores the sampled dominant states over the analysis window.
  - velocity_buffer stores recent speed estimates for moving average smoothing.
  - The code is optimized for continuous loop-based execution without interrupts.
*/

#define HALL_PIN  21

#define SAMPLE_INTERVAL_US 1000       // Sample every 1 ms
#define ANALYSIS_INTERVAL_MS 100      // Analyze every 100 ms
#define BUFFER_SIZE (ANALYSIS_INTERVAL_MS * 1000 / SAMPLE_INTERVAL_US)
#define AVG_WINDOW 4                  // Number of samples used for averaging

uint8_t buffer[BUFFER_SIZE];
int buffer_index = 0;

float velocity_buffer[AVG_WINDOW] = {0};
int velocity_index = 0;
bool buffer_filled = false;

unsigned long last_sample_time = 0;
unsigned long last_analysis_time = 0;

void setup() {
  pinMode(HALL_PIN, INPUT);
  Serial.begin(115200);
}

void loop() {
  unsigned long now = micros();

  // [1] Sample every 1 ms
  if (now - last_sample_time >= SAMPLE_INTERVAL_US) {
    last_sample_time = now;

    unsigned long t_start = micros();
    unsigned long t_high = 0;
    bool last = digitalRead(HALL_PIN);
    unsigned long t_last_change = t_start;

    while (micros() - t_start < SAMPLE_INTERVAL_US) {
      bool current = digitalRead(HALL_PIN);
      if (current != last) {
        unsigned long t_now = micros();
        if (last) t_high += t_now - t_last_change;
        t_last_change = t_now;
        last = current;
      }
    }

    if (last) t_high += micros() - t_last_change;

    bool state = (t_high > (SAMPLE_INTERVAL_US / 2)) ? 1 : 0;
    buffer[buffer_index] = state;
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;
  }

  // [2] Analyze every 100 ms
  if (millis() - last_analysis_time >= ANALYSIS_INTERVAL_MS) {
    last_analysis_time = millis();

    int transitions = 0;
    bool last_state = buffer[(buffer_index - BUFFER_SIZE + BUFFER_SIZE) % BUFFER_SIZE];
    for (int i = 1; i < BUFFER_SIZE; i++) {
      int idx = (buffer_index - BUFFER_SIZE + i + BUFFER_SIZE) % BUFFER_SIZE;
      if (buffer[idx] != last_state) {
        transitions++;
        last_state = buffer[idx];
      }
    }

    int pulses = transitions / 2;

    // Calculate speed using the measured conversion factor (per 100 ms)
    float velocity_kph = pulses * 3.66 / 2;

    // Update sliding average buffer
    velocity_buffer[velocity_index] = velocity_kph;
    velocity_index = (velocity_index + 1) % AVG_WINDOW;

    if (velocity_index == 0) buffer_filled = true;

    // Calculate average
    float avg_velocity = 0;
    int count = buffer_filled ? AVG_WINDOW : velocity_index;
    for (int i = 0; i < count; i++) {
      avg_velocity += velocity_buffer[i];
    }
    avg_velocity /= count;

    // Print result
    Serial.print("Pulses: ");
    Serial.print(pulses);
    Serial.print(" -> Avg Speed: ");
    Serial.print(avg_velocity, 1);
    Serial.println(" km/h");
  }
}
