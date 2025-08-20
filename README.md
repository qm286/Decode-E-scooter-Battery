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


├── firmware/         # ESP32 source code (UART + BLE)

├── captures/         # RS485 raw data logs, decoded frames

├── android-app/      # Kotlin app for BLE dashboard

├── docs/             # Protocol notes, timing diagrams

└── images/           # Photos, schematics, demo screenshots

📸 Demo

![b77ef57c-0088-4ae8-9b3c-59b7c032640d](https://github.com/user-attachments/assets/5923353c-8529-4eac-a77f-7ff8662e99e8)
<img width="193" height="183" alt="Screenshot 2025-07-19 003134" src="https://github.com/user-attachments/assets/16d98935-5854-4e6d-936f-558953e72f1b" />
![a5ab57be-3d41-4ab5-a877-03de06aa1fcb](https://github.com/user-attachments/assets/99e1f9af-b47b-489b-ab96-cb49e81e03f9)
<img width="1227" height="390" alt="Screenshot 2025-07-07 210611 - Copy - Copy - Copy" src="https://github.com/user-attachments/assets/ed095427-aada-433f-bf04-894538500513" />


📖 Learnings

Applied reverse engineering to undocumented protocols

Gained experience in real-time embedded development

Practiced signal integrity testing with mixed hardware/software debugging

Built a full pipeline from hardware interface → embedded firmware → mobile UI
