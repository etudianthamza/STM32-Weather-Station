# 🌦️ Embedded Weather Station Based on STM32F746G-Discovery
## 📌 Project Overview

This project is a fully embedded weather monitoring system developed using the STM32F746G-DISCOVERY board and X-NUCLEO-IKS01A3.

The system acquires real-time environmental data from multiple sensors on X-NUCLEO-IKS01A3, processes the measurements using STM32 peripherals, and displays them on an interactive touchscreen interface.

This project highlights competencies in:

- Embedded firmware development

- Sensor interfacing (I2C, ADC, GPIO interrupts)

- Real-time data acquisition

- GUI design for embedded systems

---

## 🎯 Objectives

- Monitor local atmospheric conditions in real time
- Sensor interfacing (I2C, ADC, GPIO)
- Touchscreen graphical interface
- Modular embedded firmware architecture
- Implement energy-efficient screen management

---

## 🧠 System Architecture
### 🔧 Main Board
- STM32F746G-DISCOVERY
  - MCU: STM32F746NGH6 (ARM Cortex-M7 @ 216 MHz)
  - 4.3” TFT LCD (480x272)
  - Capacitive touch panel
  - Integrated microSD support
### 🌡️ Environmental Sensors
## Environmental Sensors

| Measurement              | Sensor                    | Interface        |
|--------------------------|---------------------------|------------------|
| Temperature & Humidity   | HTS221                    | I2C              |
| Barometric Pressure      | LPS22HH                   | I2C              |
| Wind Speed               | Cup Anemometer            | GPIO Interrupt   |
| Wind Direction           | 16-position Wind Vane     | ADC              |
| Rainfall                 | Tipping Bucket Gauge      | GPIO Counter     |

---

## 🔧 Hardware

- STM32F746G-Discovery (ARM Cortex-M7)
- HTS221 (Temperature & Humidity)
- LPS22HH (Pressure)
- Anemometer (Wind Speed)
- Wind Vane (Direction)
- Tipping Bucket Rain Gauge
<p align="center">
  <img src="weather_sensors.png" width="600"/>
</p>

---

## 💻 Software & Technologies

- Embedded C
- STM32 HAL Drivers
- I2C Communication
- GPIO Interrupts
- ADC Sampling
- RTC Timekeeping
- FatFs File System
- Custom Touchscreen GUI

---

## 📊 Features

- Real-time sensor monitoring
- 60-minute historical graph display
- CSV data logging
- Automatic screen sleep/wake

---

## 🚀 Getting Started

### Requirements
- STM32CubeIDE
- ST-LINK Debugger

### Installation

```bash
git clone https://github.com/yourusername/yourrepo.git
