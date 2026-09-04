#include "aht20.h"

esp_err_t aht20_init(
    aht20_t *sensor,
    i2c_master_bus_handle_t bus,
    uint8_t address,
    uint32_t clock_speed_hz)
{
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = clock_speed_hz,
    };
    return i2c_master_bus_add_device(
        bus,
        &device_config,
        &sensor->i2c_device);
}