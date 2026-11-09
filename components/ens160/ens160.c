/*
 * ENS160.c
 *
 *  Created on: Oct 8, 2026
 *      Author: tottale
 */

/*
ENS160 internal register map

0x00 → PART_ID
0x10 → OPMODE
0x20 → DEVICE_STATUS
0x21 → AQI
0x22 → TVOC
0x24 → eCO₂
*/
#include <stddef.h>

#include "ens160.h"

// Maximum time ESP-IDF may wait for one I2C transaction.
#define ENS160_I2C_TIMEOUT_MS 1000

// ENS160 register addresses from the official datasheet.
#define ENS160_REGISTER_PART_ID 0x00
#define ENS160_REGISTER_OPMODE 0x10

// Expected identity value returned by the ENS160.
#define ENS160_EXPECTED_PART_ID 0x0160

// OPMODE value that starts normal gas sensing.
#define ENS160_OPMODE_STANDARD 0x02

// Read one or more consecutive ENS160 registers.
static esp_err_t ens160_read_registers(
    ens160_t *sensor,
    uint8_t register_address,
    uint8_t *data,
    size_t data_size);

// Write one byte to an ENS160 register.
static esp_err_t ens160_write_register(
    ens160_t *sensor,
    uint8_t register_address,
    uint8_t value);

esp_err_t ens160_init(
    ens160_t *sensor,
    i2c_master_bus_handle_t bus,
    uint8_t address,
    uint32_t clock_speed_hz)
{
    // The function cannot continue without valid output storage
    // and an existing I2C bus.
    if (sensor == NULL || bus == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Describe the ENS160 device to the ESP-IDF I2C driver.
    //
    // The SDA and SCL pins are not selected here because they belong
    // to the shared bus, which main.c created earlier.
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = clock_speed_hz,
    };

    // Register the ENS160 on the existing I2C bus.
    //
    // ESP-IDF creates a device handle and stores it inside
    // sensor->i2c_device.
    esp_err_t error = i2c_master_bus_add_device(
        bus,
        &device_config,
        &sensor->i2c_device);

    if (error != ESP_OK)
    {
        return error;
    }

    // PART_ID contains two bytes:
    //
    // part_id_bytes[0] = least-significant byte, expected 0x60
    // part_id_bytes[1] = most-significant byte, expected 0x01
    uint8_t part_id_bytes[2];

    // Begin at register 0x00 and receive two consecutive bytes.
    error = ens160_read_registers(
        sensor,
        ENS160_REGISTER_PART_ID,
        part_id_bytes,
        sizeof(part_id_bytes));

    if (error != ESP_OK)
    {
        return error;
    }

    // Reconstruct the 16-bit part ID from the two little-endian bytes.
    //
    // Expected bytes: [0x60, 0x01]
    //
    // 0x01 << 8 = 0x0100
    // 0x0100 | 0x0060 = 0x0160
    uint16_t part_id =
        ((uint16_t)part_id_bytes[1] << 8) |
        (uint16_t)part_id_bytes[0];

    // Reject the device if it does not identify itself as an ENS160.
    if (part_id != ENS160_EXPECTED_PART_ID)
    {
        return ESP_ERR_NOT_FOUND;
    }

    // Write 0x02 to the OPMODE register.
    // This starts the ENS160 standard gas-sensing mode.
    error = ens160_write_register(
        sensor,
        ENS160_REGISTER_OPMODE,
        ENS160_OPMODE_STANDARD);

    if (error != ESP_OK)
    {
        return error;
    }

    return ESP_OK;
}

static esp_err_t ens160_read_registers(
    ens160_t *sensor,
    uint8_t register_address,
    uint8_t *data,
    size_t data_size)
{
    // Perform one combined register-read operation:
    //
    // 1. Send the register address.
    // 2. Generate a repeated START.
    // 3. Read data_size bytes into data.
    return i2c_master_transmit_receive(
        sensor->i2c_device,
        &register_address,
        sizeof(register_address),
        data,
        data_size,
        ENS160_I2C_TIMEOUT_MS);
}

static esp_err_t ens160_write_register(
    ens160_t *sensor,
    uint8_t register_address,
    uint8_t value)
{
    // The first byte selects the register.
    // The second byte is the value written into that register.
    uint8_t message[] = {
        register_address,
        value,
    };

    // Send both bytes in one continuous I2C transaction.
    return i2c_master_transmit(
        sensor->i2c_device,
        message,
        sizeof(message),
        ENS160_I2C_TIMEOUT_MS);
}