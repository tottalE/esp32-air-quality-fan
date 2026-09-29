#include <stdbool.h>
#include "aht20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define AHT20_STATUS_CALIBRATED_BIT (1U << 3)
#define AHT20_I2C_TIMEOUT_MS 1000
#define AHT20_I2C_POWER_DELAY_MS 40
#define AHT20_I2C_INIT_DELAY_MS 10

static bool aht20_is_calibrated(uint8_t status);
static esp_err_t aht20_read_status(aht20_t *sensor, uint8_t *status);
static esp_err_t aht20_send_initialization(aht20_t *sensor);

esp_err_t aht20_init(
    aht20_t *sensor,
    i2c_master_bus_handle_t bus,
    uint8_t address,
    uint32_t clock_speed_hz)
{
    uint8_t status_bits;

    // Validate required handles.
        if (sensor == NULL || bus == NULL)
    {
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

    // Wait AHT20_I2C_POWER_DELAY_MS
    vTaskDelay(pdMS_TO_TICKS(AHT20_I2C_POWER_DELAY_MS));

    // Read the status register to check if the sensor is calibrated
    error = aht20_read_status(sensor, &status_bits);

    if (error != ESP_OK)
    {
        return error;
    }

    // If the sensor is not calibrated, send the initialization command
    if (!aht20_is_calibrated(status_bits))
    {
        error = aht20_send_initialization(sensor);
        if (error != ESP_OK)
        {
            return error;
        }
        // Wait AHT20_I2C_INIT_DELAY_MS
        vTaskDelay(pdMS_TO_TICKS(AHT20_I2C_INIT_DELAY_MS));
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

static esp_err_t aht20_send_initialization(aht20_t *sensor)
{
    uint8_t command_bytes[] = {0xBE, 0x08, 0x00};
    esp_err_t error = i2c_master_transmit(
        sensor->i2c_device,
        command_bytes,
        sizeof(command_bytes),
        AHT20_I2C_TIMEOUT_MS
    );

    if (error != ESP_OK)
    {
        return error;
    }

    return ESP_OK;
}