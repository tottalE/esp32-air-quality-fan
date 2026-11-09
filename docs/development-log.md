# Development Log

## 2026-09-04 — Design full process
### Objective
Design general flow, write header file and c file for aht20 initalization

### Work Completed

Completed:
✓ Product goal
✓ Main system components
✓ Responsibility boundaries
✓ Shared I²C bus ownership
✓ General startup/runtime flow
✓ aht20 initialization with I²C bus

Still open, but not blocking:
- Final fan-control formula
- Fan thresholds and hysteresis
- GPIO versus PWM fan control
- Detailed failure policy

### Next Step
- aht20 API and implementation

## 2026-09-11 — Full driver codes
### Objective
Write driver codes

### Work Completed

Completed:
✓ AHT20: Calibration check
✓ AHT20: Validation logic

### Next Step
- aht20 API and implementation

## 2026-09-16 — aht20 API and implementation
### Objective
Write driver codes

### Work Completed

Validate arguments
→ register AHT20 on the existing I²C bus
→ wait 40 ms
→ read the status byte
→ check calibration bit 3
→ send {0xBE, 0x08, 0x00} if needed
→ wait 10 ms
→ return ESP_OK or the encountered error

### Next Step
- Measurement command + raw-data reading

## 2026-10-03 — aht20 API
### Objective
Write function to 

### Work Completed

Validate arguments
→ register AHT20 on the existing I²C bus
→ wait 40 ms
→ read the status byte
→ check calibration bit 3
→ send {0xBE, 0x08, 0x00} if needed
→ wait 10 ms
→ return ESP_OK or the encountered error

### Next Step
- Measurement command + raw-data reading