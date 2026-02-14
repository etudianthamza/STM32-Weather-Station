# 🌦️ Embedded Weather Station Based on STM32F746G-Discovery
## 📌 Project Overview
## 📌 Project Overview

This project implements a fully embedded environmental monitoring system using the STM32F746G-Discovery board.

The system collects real-time environmental data (temperature, humidity, pressure, wind, rainfall), processes the measurements using STM32 peripherals, displays them on a touchscreen interface, and logs historical data to a microSD card.

---

## 🎯 Objectives

- Real-time environmental monitoring
- Sensor interfacing (I2C, ADC, GPIO)
- Touchscreen graphical interface
- Data logging on microSD
- Modular embedded firmware architecture

---

## 🔧 Hardware

- STM32F746G-Discovery (ARM Cortex-M7)
- HTS221 (Temperature & Humidity)
- LPS22HH (Pressure)
- Anemometer (Wind Speed)
- Wind Vane (Direction)
- Tipping Bucket Rain Gauge
- microSD Card

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
