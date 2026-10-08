#include "aht20.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define APP_I2C_PORT I2C_NUM_0
#define APP_I2C_SDA_GPIO GPIO_NUM_8
#define APP_I2C_SCL_GPIO GPIO_NUM_9

#define APP_AHT20_ADDRESS 0x38
#define APP_I2C_CLOCK_HZ 100000
#define APP_MEASUREMENT_INTERVAL_MS 1000

static const char *TAG = "air_quality";

void app_main(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = APP_I2C_PORT,
        .sda_io_num = APP_I2C_SDA_GPIO,
        .scl_io_num = APP_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(
        i2c_new_master_bus(&bus_config, &bus));

    aht20_t sensor = {0};

    ESP_ERROR_CHECK(
        aht20_init(
            &sensor,
            bus,
            APP_AHT20_ADDRESS,
            APP_I2C_CLOCK_HZ));

    while (true)
    {
        aht20_data_t measurement;

        esp_err_t error =
            aht20_measure(&sensor, &measurement);

        if (error == ESP_OK)
        {
            ESP_LOGI(
                TAG,
                "Temperature: %.2f C, Humidity: %.2f %%",
                measurement.temperature_c,
                measurement.humidity_percent);
        }
        else
        {
            ESP_LOGE(
                TAG,
                "AHT20 measurement failed: %s",
                esp_err_to_name(error));
        }

        vTaskDelay(
            pdMS_TO_TICKS(APP_MEASUREMENT_INTERVAL_MS));
    }
}