#include <stdbool.h>
#include "aht20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define AHT20_STATUS_CALIBRATED_BIT (1U << 3)
#define AHT20_I2C_TIMEOUT_MS 1000
#define AHT20_I2C_POWER_DELAY_MS 40
#define AHT20_I2C_INIT_DELAY_MS 10

#define AHT20_STATUS_BUSY_BIT       (1U << 7)
#define AHT20_MEASUREMENT_DELAY_MS  80
#define AHT20_RESPONSE_SIZE         7

#define AHT20_DATA_SIZE 6
#define AHT20_CRC_POLYNOMIAL 0x31U
#define AHT20_RAW_SCALE 1048576.0F

static bool aht20_is_calibrated(uint8_t status);
static esp_err_t aht20_read_status(aht20_t *sensor, uint8_t *status);
static esp_err_t aht20_send_initialization(aht20_t *sensor);
static esp_err_t aht20_read_raw_measurement(aht20_t *sensor, uint8_t data[AHT20_RESPONSE_SIZE]);
static uint32_t aht20_extract_raw_humidity(const uint8_t data[AHT20_RESPONSE_SIZE]);
static uint32_t aht20_extract_raw_temperature(const uint8_t data[AHT20_RESPONSE_SIZE]);
static uint8_t aht20_calculate_crc(const uint8_t data[AHT20_RESPONSE_SIZE]);


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

esp_err_t aht20_measure(
    aht20_t *sensor,
    aht20_data_t *measurement)
{
    if (sensor == NULL || measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[AHT20_RESPONSE_SIZE];

    esp_err_t error = aht20_read_raw_measurement(sensor, data);
    if (error != ESP_OK)
    {
        return error;
    }

    uint8_t calculated_crc = aht20_calculate_crc(data);

    if (calculated_crc != data[6])
    {
        return ESP_ERR_INVALID_CRC;
    }

    uint32_t raw_humidity =
        aht20_extract_raw_humidity(data);

    uint32_t raw_temperature =
        aht20_extract_raw_temperature(data);

    measurement->humidity_percent =
        ((float)raw_humidity / AHT20_RAW_SCALE) * 100.0F;

    measurement->temperature_c =
        ((float)raw_temperature / AHT20_RAW_SCALE) *
            200.0F -
        50.0F;

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

static esp_err_t aht20_read_raw_measurement(
    aht20_t *sensor,
    uint8_t data[AHT20_RESPONSE_SIZE])
{
    /* 1. Create the measurement command then transmit it*/
    uint8_t command_bytes[] = {0xAC, 0x33, 0x00};
    esp_err_t error = i2c_master_transmit(
        sensor->i2c_device,
        command_bytes,
        sizeof(command_bytes),
        AHT20_I2C_TIMEOUT_MS);

    /* 2. If transmission failed, return that error. */
    if (error != ESP_OK)
    {
        return error;
    }

    /* 4. Wait for the measurement delay. */
    vTaskDelay(pdMS_TO_TICKS(AHT20_MEASUREMENT_DELAY_MS));

    /* 5. Receive AHT20_RESPONSE_SIZE bytes into data. */
    error = i2c_master_receive(
        sensor->i2c_device,
        data,
        AHT20_RESPONSE_SIZE,
        AHT20_I2C_TIMEOUT_MS);

    /* 6. If reception failed, return that error. */
    if (error != ESP_OK)
    {
        return error;
    }

    /* 7. Examine the busy bit in data[0].
          If it is still 1, return ESP_ERR_TIMEOUT. */
    if (data[0] & AHT20_STATUS_BUSY_BIT)
    {
        return ESP_ERR_TIMEOUT;
    }

    /* 8. Otherwise, return ESP_OK. */
    return ESP_OK;
}

static uint32_t aht20_extract_raw_humidity(
    const uint8_t data[AHT20_RESPONSE_SIZE])
{
    return ((uint32_t)data[1] << 12) |
           ((uint32_t)data[2] << 4) |
           ((uint32_t)data[3] >> 4);
}

static uint32_t aht20_extract_raw_temperature(
    const uint8_t data[AHT20_RESPONSE_SIZE])
{
    return (((uint32_t)data[3] & 0x0FU) << 16) |
           ((uint32_t)data[4] << 8) |
           (uint32_t)data[5];
}

// Check the CRC of the received data to ensure data integrity
static uint8_t aht20_calculate_crc(
    const uint8_t data[AHT20_RESPONSE_SIZE])
{
    uint8_t crc = 0xFFU;

    for (uint8_t byte_index = 0;
         byte_index < AHT20_DATA_SIZE;
         ++byte_index)
    {
        crc ^= data[byte_index];

        for (uint8_t bit_index = 0; bit_index < 8U; ++bit_index)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1) ^
                                AHT20_CRC_POLYNOMIAL);
            }
            else
            {
                crc = (uint8_t)(crc << 1);
            }
        }
    }

    return crc;
}