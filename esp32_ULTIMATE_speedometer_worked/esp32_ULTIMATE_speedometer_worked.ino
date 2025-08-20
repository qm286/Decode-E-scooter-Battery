
/*code mượt vãi l**
*/

#define HALL_PIN  21

#define SAMPLE_INTERVAL_US 1000       // Lấy mẫu mỗi 1ms
#define ANALYSIS_INTERVAL_MS 100      // Phân tích mỗi 100ms
#define BUFFER_SIZE (ANALYSIS_INTERVAL_MS * 1000 / SAMPLE_INTERVAL_US)
#define AVG_WINDOW 4                  // Số mẫu dùng để trung bình

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

  // [1] Lấy mẫu mỗi 1ms
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

  // [2] Phân tích mỗi 100ms
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

    // Tính vận tốc theo hệ số bạn đo được (mỗi 100ms)
    float velocity_kph = pulses * 3.66 / 2;

    // Cập nhật buffer trung bình trượt
    velocity_buffer[velocity_index] = velocity_kph;
    velocity_index = (velocity_index + 1) % AVG_WINDOW;

    if (velocity_index == 0) buffer_filled = true;

    // Tính trung bình
    float avg_velocity = 0;
    int count = buffer_filled ? AVG_WINDOW : velocity_index;
    for (int i = 0; i < count; i++) {
      avg_velocity += velocity_buffer[i];
    }
    avg_velocity /= count;

    // In ra
    Serial.print("Pulses: ");
    Serial.print(pulses);
    Serial.print(" -> Avg Speed: ");
    Serial.print(avg_velocity, 1);
    Serial.println(" km/h");
  }
}
