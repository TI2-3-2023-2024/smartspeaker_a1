#include "buttonreader.h"


static const char* TAG = "IO Expander";
static mcp23x17_t dev; //handle for i2c

//button masks to set pins buttons are wired to
const int BUTTON_MASK = 0x7f; //0b01111111 

/*
* Function: Prints value in binary format; Used when debugging
* Parameters: Value in uint16_t to be printed
* Returns: None
*/
static void print_bits(uint16_t value) {
char binary_string[17]; // 16 bits + 1 for null terminator

    // Iterate over each bit starting from the most significant bit (MSB)
    for(int i = 15; i >= 0; i--) 
    {
        // Append the i-th bit to the binary string
        binary_string[15 - i] = ((value >> i) & 0x01) + '0';
    }
    binary_string[16] = '\0'; // Null-terminate the string

    // Print the binary representation as a single log message
    ESP_LOGI(TAG, "Binary representation of pin_value: %s", binary_string);
}

/*
* Function: Reads values of pins on io-expander 
* Parameters: None 
* Returns: Values read from pins on io-expander
*/
uint16_t read_button(void *pvParameters){
    uint16_t pin_value; // values of pins get written to this variable
    mcp23x17_port_read(&dev, &pin_value );
    pin_value = (pin_value ^ BUTTON_MASK) & 0x00ff; //mask to filter wired pins
    if (pin_value != 0)
    {
        ESP_LOGI(TAG, "retrieved value: %d", pin_value);
    }
    return pin_value;
}

/*
* Function: Initiates the button reader
* Parameters: None
* Returns: None
*/
void buttonreader_init()
{
    ESP_ERROR_CHECK(i2cdev_init());
    memset(&dev, 0, sizeof(mcp23x17_t));

    ESP_ERROR_CHECK(mcp23x17_init_desc(&dev, CONFIG_GPIO_EXPANDER_ADDR, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    // Setup PORTA0 as input
    mcp23x17_set_mode(&dev, 0, MCP23X17_GPIO_INPUT);
    // Setup PORTB0 as output
    mcp23x17_set_mode(&dev, 9, MCP23X17_GPIO_OUTPUT);
    
    //xTaskCreate(test, "test", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
}

/*
* Function: Starts the reader
* Parameters: None
* Returns: None
*/
void start_reader(){
    buttonreader_init();
    while(1)
    {
        read_button(NULL);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

