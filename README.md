# ESP32 Air Quality Ventilation Controller

An embedded ventilation system that monitors air quality and controls a fan using an ESP32 and a MOSFET-based driver circuit.

## Overview

Briefly explain:

- What problem does the project solve?
- Why did you start the project?
- Where could the system be used?

## Project Goals

- Read environmental data from an air-quality sensor
- Develop a sensor driver based on the device datasheet
- Control a ventilation fan through a MOSFET
- Adjust fan operation according to air-quality measurements
- Handle sensor and communication failures safely
- Verify the system through measurements and long-duration testing

## System Architecture

```text
Air-Quality Sensor
        │ I²C / UART
        ▼
      ESP32
        │ GPIO / PWM
        ▼
 MOSFET Driver ── Fan
```

## Repository Structure

