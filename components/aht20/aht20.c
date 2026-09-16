#include <stdbool.h>
#include "aht20.h"

#define AHT20_STATUS_CALIBRATED_BIT (1U << 3)
#define AHT20_I2C_TIMEOUT_MS 1000

static bool aht20_is_calibrated(uint8_t status);
static esp_err_t aht20_read_status(aht20_t *sensor, uint8_t *status);


esp_err_t aht20_init(
    aht20_t *sensor,
    i2c_master_bus_handle_t bus,
    uint8_t address,
    uint32_t clock_speed_hz)
{
    // Validate required handles.
    if (sensor == NULL || bus == NULL) {
    return ESP_ERR_INVALID_ARG;
    }

    // Initialize the I2C device configuration
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = clock_speed_hz,
    };

    // Add the AHT20 device to the I2C bus
    esp_err_t error = i2c_master_bus_add_device(
        bus,
        &device_config,
        &sensor->i2c_device);

    if (error != ESP_OK) {
        return error;
    }

    return ESP_OK;
}

static bool aht20_is_calibrated(uint8_t status)
{
    return (status & AHT20_STATUS_CALIBRATED_BIT) != 0U; // Check if the calibration bit is set
}


static esp_err_t aht20_read_status(aht20_t *sensor, uint8_t *status)
{
    return i2c_master_receive(
        sensor->i2c_device,
        status,
        sizeof(*status),
        AHT20_I2C_TIMEOUT_MS);
}