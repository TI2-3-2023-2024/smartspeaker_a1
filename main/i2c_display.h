#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO 23        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 18        /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define MPU9250_SENSOR_ADDR 0x04       /*!< Slave address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR 0x75 /*!< Register addresses of the "who am I" register */

#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B /*!< Register addresses of the power managment register */
#define MPU9250_RESET_BIT 7

static const char *TAG = "i2c-simple-example";

esp_err_t write_coordinates(uint8_t x, uint8_t y);
esp_err_t write_brightness_value(uint8_t value);
esp_err_t write_animation_start();
esp_err_t i2c_master_init(void);