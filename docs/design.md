# Air Quality and Fan Control System Design

## 1. Project Goal

Build an ESP32-C3 system that:

1. Measures indoor environmental conditions.
2. Reads air-quality information.
3. Decides whether the air quality is unacceptable.
4. Turns on a cooling or ventilation fan when necessary.

The fan is connected to a MOSFET, and the MOSFET is controlled by an ESP32 GPIO output.

## 2. Hardware

The sensor module contains two separate sensors:

- ENS160: air-quality sensor
- AHT2x: temperature and relative-humidity sensor

Although they are mounted on the same module, they are separate I²C devices with different responsibilities.

The exact AHT2x model must be confirmed before implementing its driver.

Current project assumptions:

- AHT20 I²C address: `0x38`
- ENS160 I²C address: `0x53`

## 3. System Data Flow

The intended data flow is:

```text
AHT2x
  │ temperature and relative humidity
  ▼
ENS160 environmental compensation
  │
  ▼
ENS160 AQI, TVOC, and eCO₂
  │
  ▼
Application fan-control policy
  │
  ▼
GPIO → MOSFET → fan