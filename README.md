# 🚲 E-Scooter RS485 Data Decoder

## 📌 Overview

This project focuses on reverse engineering and decoding RS485 data frames from a commercial e-scooter. The goal was to monitor real-time system parameters such as speed, battery status, and throttle signals, then display them through a custom dashboard.

By combining embedded firmware, signal analysis, and protocol decoding, this project demonstrates practical skills in real-time systems, low-latency data processing, and hardware debugging.

## 🔧 Features

- Reverse engineered a proprietary RS485 communication protocol
- Implemented a firmware decoder on **ESP32 (C++)**
- Extracted key telemetry values, including:
  - Vehicle speed (km/h)
  - Battery voltage and current
  - Throttle and brake signals
- Supported **BLE Notify** to stream data to an Android app
- Built a real-time dashboard visualization

## 📘 Project Documentation

To understand the overall project and the reverse-engineering process in detail, please read:

- `assets/docs/Project_Summary/Project_Summary.pdf`
- `assets/docs/Decoding_Data/jk_bms_reverse_engineering_note.pdf`

## 🔌 Hardware Setup

- Please refer to `asset/Figures/Figure 5 wiring diagram.png` for the hardware wiring details.

## 🧩 Code Structure

- **`esp_ble_basic/`**
  - Minimal ESP32 BLE example
  - Established a reliable BLE Notify pipeline with dummy data
  - Verified Android BLE client compatibility before integrating actual telemetry

- **`esp32_test_SoC_mah_Ampe_volt_battTemp/`**
  - Prototype for decoding and calculating battery-related parameters
  - Includes:
    - State of Charge (SoC)
    - mAh consumed
    - Current (A)
    - Voltage (V)
    - Temperature (°C)
  - Output was validated through the Serial Monitor

- **`esp32_speedometer/`**
  - Prototype focused on real-time speed calculation
  - Counted hall sensor pulses via GPIO with 1 ms sampling
  - Computed average speed (km/h) using a sliding window
  - First successful implementation of a stable and accurate speedometer

- **`ESP32_final/`**
  - Final integrated firmware
  - Includes:
    - RS485 frame decoding for speed, battery, throttle, and temperature
    - BLE notification streaming to the Android app
    - Real-time dashboard display
  - Represents the final working firmware of the project

## 🛠️ Tech Stack

- **Firmware:** ESP32 using the Arduino framework
- **Protocol:** **RS485 UART** with custom decoded frames
- **Mobile App:** Android (Kotlin, **BLE Notify**)
- **Tools:** Logic analyzer and oscilloscope for signal capture and debugging

## 📂 Repository Structure

```text
DECODE-E-SCOOTER-BATTERY/
├── assets/
│   ├── docs/
│   │   ├── Decoding_Data/
│   │   │   ├── 308-byte-decode.xlsx
│   │   │   ├── jk_bms_reverse_engineering_note.pdf
│   │   │   └── Raw data from Official tool.png
│   │   └── Project_Summary/
│   │       └── Project Summary.pdf
│   └── Figures/
│       ├── Figure 1 Hall-signal-monitoring.png
│       ├── Figure 2 JK-BMS-offical-app-data-logging.png
│       ├── Figure 3 Testing-on-android-app.jpg
│       ├── Figure 4 The device.png
│       └── Figure 5 wiring diagram.png
├── Firmware/
│   ├── esp_ble_basic/
│   │   └── esp_ble_basic.ino
│   ├── esp32_final/
│   │   └── esp_final.ino
│   ├── esp32_speedometer/
│   │   └── esp32_speedometer.ino
│   └── esp32_test_SoC_mah_Ampe_volt_battTemp/
│       └── esp32_test_SoC_mah_Ampe_volt_battTemp.ino
└── README.md
```

## 📖 Learnings

- Applied **reverse engineering** to undocumented communication protocols
- Gained hands-on experience in real-time **embedded development**
- Practiced signal capture, validation, and mixed hardware/software debugging
- Built a complete pipeline from:
  - hardware interface
  - embedded firmware
  - BLE communication
  - mobile dashboard visualization