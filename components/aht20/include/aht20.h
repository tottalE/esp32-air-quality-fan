#pragma once

#include <stdint.h>
#include "driver/i2c_master.h" 
#include "esp_err.h"

typedef struct
{
    i2c_master_dev_handle_t i2c_device;
} aht20_t;

typedef struct
{
    float temperature_c;
    float humidity_percent;
} aht20_data_t;

esp_err_t aht20_init(
    aht20_t *sensor,
    i2c_master_bus_handle_t bus,
    uint8_t address,
    uint32_t clock_speed_hz);

esp_err_t aht20_measure(
    aht20_t *sensor,
    aht20_data_t *measurement);