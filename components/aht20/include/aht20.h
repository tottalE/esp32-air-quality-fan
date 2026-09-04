#pragma once

#include <stdint.h>
#include "driver/i2c_master.h" 
#include "esp_err.h"

typedef struct
{
    i2c_master_dev_handle_t i2c_device;
} aht20_t;

esp_err_t aht20_init(
    aht20_t *sensor,
    i2c_master_bus_handle_t bus,
    uint8_t address,
    uint32_t clock_speed_hz);