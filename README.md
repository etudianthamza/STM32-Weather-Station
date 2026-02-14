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

- Real-time environmental monitoring
- Sensor interfacing (I2C, ADC, GPIO)
- Touchscreen graphical interface
- Modular embedded firmware architecture

---

## 🔧 Hardware

- STM32F746G-Discovery (ARM Cortex-M7)
- HTS221 (Temperature & Humidity)
- LPS22HH (Pressure)
- Anemometer (Wind Speed)
- Wind Vane (Direction)
- Tipping Bucket Rain Gauge
![weather_sensors](weather_sensors.png)
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
