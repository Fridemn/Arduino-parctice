#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_SDA_IO           4
#define I2C_MASTER_SCL_IO           5
#define I2C_MASTER_FREQ_HZ          100000
#define MPU6050_SENSOR_ADDR         0x68
#define MPU6050_WHO_AM_I_REG        0x75
#define MPU6050_PWR_MGMT_1_REG      0x6B
#define MPU6050_ACCEL_XOUT_H        0x3B


static const char *TAG = "MPU6050";
#define LED_GPIO 2
#define SHAKE_THRESHOLD 20000

esp_err_t i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

esp_err_t mpu6050_write_byte(uint8_t reg_addr, uint8_t data)
{
    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, &reg_addr, 1, 1000/portTICK_PERIOD_MS) == ESP_OK &&
           i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, &data, 1, 1000/portTICK_PERIOD_MS) == ESP_OK ? ESP_OK : ESP_FAIL;
}

esp_err_t mpu6050_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, &reg_addr, 1, data, len, 1000/portTICK_PERIOD_MS);
}


void mpu6050_test_task(void *arg)
{
    uint8_t who_am_i = 0;
    uint8_t accel_gyro_data[14];
    ESP_ERROR_CHECK(mpu6050_read_bytes(MPU6050_WHO_AM_I_REG, &who_am_i, 1));
    ESP_LOGI(TAG, "WHO_AM_I: 0x%02x", who_am_i);

    // Wake up MPU6050
    ESP_ERROR_CHECK(mpu6050_write_byte(MPU6050_PWR_MGMT_1_REG, 0x00));

    // LED 初始化
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO, 0);

    while (1) {
        int shake_count = 0;
        // 检测连续晃动1秒
        while (shake_count < 10) { // 100ms*10=1s
            if (mpu6050_read_bytes(MPU6050_ACCEL_XOUT_H, accel_gyro_data, 14) == ESP_OK) {
                int16_t ax = (accel_gyro_data[0] << 8) | accel_gyro_data[1];
                int16_t ay = (accel_gyro_data[2] << 8) | accel_gyro_data[3];
                int16_t az = (accel_gyro_data[4] << 8) | accel_gyro_data[5];
                int16_t gx = (accel_gyro_data[8] << 8) | accel_gyro_data[9];
                int16_t gy = (accel_gyro_data[10] << 8) | accel_gyro_data[11];
                int16_t gz = (accel_gyro_data[12] << 8) | accel_gyro_data[13];
                ESP_LOGI(TAG, "Accel: X=%d Y=%d Z=%d | Gyro: X=%d Y=%d Z=%d", ax, ay, az, gx, gy, gz);

                if (abs(ax) > SHAKE_THRESHOLD || abs(ay) > SHAKE_THRESHOLD || abs(az) > SHAKE_THRESHOLD) {
                    shake_count++;
                } else {
                    shake_count = 0;
                }
            } else {
                ESP_LOGE(TAG, "Failed to read sensor data");
                shake_count = 0;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        // 连续晃动1秒，LED长亮1秒
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_GPIO, 0);
        // 亮完再重新检测
    }
}


void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());
    xTaskCreate(mpu6050_test_task, "mpu6050_test", 2048, NULL, 5, NULL);
}
