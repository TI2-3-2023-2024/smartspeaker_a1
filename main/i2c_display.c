#include "i2c_display.h"


esp_err_t write_coordinates(uint8_t x, uint8_t y)
{
    int ret;
    uint8_t write_buf[3] = {'c', x, y};

    ESP_LOGI("i2c-display", "Coordinaten doorgestuurd: %d, %d", x, y);

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}


/*
 * Function: Schrijft de helderheid waarde naar het led-scherm.
 * Parameters: Waarde van de helderheid
 * Returns: esp_err_t om te controleren of de handeling correct is uitgevoerd.
 */
esp_err_t write_brightness_value(uint8_t value)
{
    int ret;
    uint8_t write_buf[2] = {'b', value}; // 'b' is de specifieke commando voor het aanpassen van de helderheid.

    ESP_LOGI("i2c-display", "Brightness aangepast naar het volgende %d", value);
    
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/* 
* Functie: i2c_master_init
* Beschrijving: Initialiseert dit device als een i2c master
* Parameters: Geen 
* Retourneert: Status van installatie i2c drivers
*/
esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}