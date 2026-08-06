# Non-Invasive-Optical-Sensing-Kit-for-Glu-and-Hb-level-Estimation

## Overview

This project presents a portable, non-invasive optical sensing system for the simultaneous estimation of blood glucose and hemoglobin levels using multi-wavelength optical sensing technology. The system is built around the ESP32 microcontroller, which acquires, processes, and analyzes optical sensor data in real time. The measured values are displayed on an OLED display and can also be monitored remotely through an IoT dashboard, making the system suitable for continuous health monitoring and point-of-care diagnostic applications.


## Features

* Non-invasive measurement of blood glucose and hemoglobin.
* Multi-wavelength optical sensing for improved accuracy.
* Real-time signal acquisition and processing using ESP32.
* Instant result display on a 0.96-inch OLED display.
* IoT-based remote monitoring dashboard.
* Portable, compact, and low-power design.
* Real-time health data visualization.


## Objectives

* Design and develop a portable, non-invasive optical sensing kit capable of simultaneously estimating blood glucose and hemoglobin levels using multi-wavelength optical sensing.
* Implement signal acquisition, signal conditioning, and real-time data processing using the ESP32 microcontroller for accurate measurements.
* Integrate an OLED display and an IoT-based dashboard for continuous health monitoring.
* Evaluate system performance, identify challenges, and propose future enhancements for practical deployment in point-of-care diagnostics.



## Hardware Components

* ESP32 Development Board
* Multi-Wavelength Optical Sensor Module
* OLED Display (0.96" I2C)
* Signal Conditioning Circuit
* Finger Clip Sensor Housing
* USB Power Supply
* Connecting Wires and PCB/Breadboard


## Software Requirements

* Arduino IDE
* ESP32 Board Package
* C/C++
* IoT Platform (e.g., Blynk, ThingSpeak, Firebase)
* Git
* GitHub


## System Architecture

1. Optical sensors emit multiple wavelengths of light.
2. Light passes through the user's fingertip.
3. The reflected/transmitted light is captured by the photodetector.
4. The signal conditioning circuit amplifies and filters the sensor output.
5. ESP32 processes the signals and estimates glucose and hemoglobin levels.
6. Results are displayed on the OLED screen.
7. Data is transmitted to the IoT dashboard via Wi-Fi.


## Usage

1. Power on the device.
2. Place your fingertip on the optical sensing module.
3. Wait for signal acquisition and processing.
4. View the estimated glucose and hemoglobin values on the OLED display.
5. Monitor historical data and trends through the IoT dashboard.

## Results

The developed prototype successfully demonstrates:

* Real-time optical signal acquisition.
* Simultaneous estimation of glucose and hemoglobin.
* OLED-based live display.
* Wireless transmission of health data to an IoT dashboard.
* Portable operation suitable for prototype-level testing.




