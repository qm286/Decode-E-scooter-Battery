🚲 E-Scooter RS485 Data Decoder
📌 Overview

This project focuses on reverse engineering and decoding RS485 data frames from a commercial e-scooter. The goal was to monitor real-time system parameters such as speed, battery status, and throttle signals, then display them via a custom dashboard.

By combining embedded firmware, signal analysis, and protocol decoding, this project demonstrates practical skills in real-time systems, low-latency data processing, and hardware debugging.

🔧 Features

Reverse engineered proprietary RS485 communication protocol

Implemented a firmware decoder on ESP32 (C++)

Extracted key telemetry values:

Vehicle speed (km/h)

Battery voltage & current

Throttle & brake signals

Supported BLE notify to stream data to an Android app

Real-time dashboard visualization

🛠️ Tech Stack

Firmware: ESP32 (Arduino framework)

Protocol: RS485 UART (custom decoded frames)

Mobile App: Android (Kotlin, BLE Notify)

Tools: Logic analyzer, oscilloscope for signal capture

📂 Repository Structure
escooter-rs485-decoder/
│
├── firmware/         # ESP32 source code (UART + BLE)
├── captures/         # RS485 raw data logs, decoded frames
├── android-app/      # Kotlin app for BLE dashboard
├── docs/             # Protocol notes, timing diagrams
└── images/           # Photos, schematics, demo screenshots

📸 Demo

(Add real photos or screenshots here, e.g., scope captures, app dashboard, scooter setup)

📖 Learnings

Applied reverse engineering to undocumented protocols

Gained experience in real-time embedded development

Practiced signal integrity testing with mixed hardware/software debugging

Built a full pipeline from hardware interface → embedded firmware → mobile UI
