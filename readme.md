# ESP32 PLC Touch HMI (ESP32-2432S028 + LVGL)

## Overview

This project implements a **touchscreen Human Machine Interface (HMI)** for monitoring and controlling a PLC using an **ESP32-2432S028 (2.8" 240x320 TFT touchscreen)**.

**This GUI is designed to control and monitor the [SmartPLC_atmel328_unit](https://github.com/AlonsoVallejo/SmartPLC_atmel328_unit) Arduino-based PLC board.**
The ESP32 acts as the HMI front-end, while the Arduino board handles relay outputs and digital inputs.

The interface is built using **LVGL** and provides:

* Real-time monitoring of **6 PLC inputs** (status indicators and text)
* Manual control of **6 PLC outputs** (touch switches)
* **Serial communication** with Arduino PLC board (custom protocol)
* **Communication status indicator** (header bar)
* **System status bar** (footer)
* Modular architecture using **FreeRTOS tasks**

The project is designed as a **starting point for industrial HMI development** with ESP32.

---

# Features

## GUI (LVGL)

![Full Panel Example](/board/integration.jpg)

**Inputs Panel:**
- Shows the state of 6 PLC inputs (LED dot + ON/OFF text)
- Green = ON, Gray = OFF

**Outputs Panel:**
- Touch switches for 6 relay outputs
- State synced with Arduino PLC

**Communication Indicator:**
- Header bar shows COM status (online/offline)
- Green = communication OK

**Status Bar:**
- Footer displays system status or errors

---

# Hardware

Recommended hardware:

* **ESP32-2432S028** (2.8" touchscreen display)
* **ILI9341 TFT controller**
* **XPT2046 touch controller**

# Software Architecture

The system uses **FreeRTOS tasks**.

```
+-------------------+
| GUI Task (LVGL)   |
| - Rendering       |
| - Touch input     |
+-------------------+

+-------------------+
| PLC Task          |
| - UART            |
| - Read inputs     |
| - Send outputs    |
+-------------------+

+-------------------+
| Main Loop         |
| - LVGL handler    |
+-------------------+
```

# Project Structure

```
```
---

# Dependencies

Libraries used:

* **LVGL 8.x**
* **TFT_eSPI**

---

# Build Environment

Recommended tools:

* **PlatformIO (VSCode)**
* **ESP32 Arduino Framework**

Example configuration:

```
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

---

# Running the Project

1. Install **VSCode + PlatformIO**
2. Clone or copy this project
3. Open the folder in PlatformIO
4. Build the project

```
PlatformIO: Build
```

5. Upload firmware

```
PlatformIO: Upload
```

6. Open serial monitor

```
115200 baud
```

---

# PLC Communication

src/plc_comm.cpp
This project implements **real serial communication** between ESP32 and Arduino PLC board.

**Protocol:**
- ESP32 sends output states: `OUT:xx` (hex byte for 6 relays)
- ESP32 requests state: `REQ`
- Arduino responds: `STATE:in:xx` (hex bytes for inputs and outputs)

See `src/plc_comm.cpp` and `src/serial_protocol.cpp` for protocol details.

**This HMI is the GUI front-end for [SmartPLC_atmel328_unit](https://github.com/AlonsoVallejo/SmartPLC_atmel328_unit).**

---

# Future Improvements

Recommended upgrades:

### Touch Driver

Add support for the **XPT2046 touch controller**.

### Alarm System

Add popup windows for faults.

### Manual / Auto Mode

Prevent manual control when PLC is in automatic mode.

### Diagnostics Screen

Display:

* RX packets
* TX packets
* Communication errors

---

# Example Use Cases

This HMI can be used for:

* Small industrial machines
* Educational PLC projects
* Lab automation
* Custom control panels
* IoT gateway for PLC systems
* As a GUI for [SmartPLC_atmel328_unit](https://github.com/AlonsoVallejo/SmartPLC_atmel328_unit)

---

# License

Open-source example project for educational and prototyping purposes.

---

# Author

Designed as a starting point for **embedded engineers building ESP32 industrial HMIs using LVGL**.

Optimized for developers familiar with:

* Embedded C/C++
* FreeRTOS
* Serial protocols
* Industrial control systems
