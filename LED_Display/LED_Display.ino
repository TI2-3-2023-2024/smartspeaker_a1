/* 
* Bestandsnaam: LED_DISPLAY.ino 
* Auteur: Niels en Thijme
* Datum: 5 maart 2024 
* Beschrijving: Dit bestand bevat de functionaliteit van de esp32 om een LED display aan kan sturen en I2C communicatie ontvangen
*/


#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <WireSlave.h>
#include <FreeRTOSConfig.h>
#include "driver/i2c.h"

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04

#define PIN_WS2812B 12  // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 256  // The number of LEDs (pixels) on WS2812B LED strip

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

struct coordinate{
  uint8_t x;
  uint8_t y;
};


void setup() {
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output

  ws2812b.begin();  // initialize WS2812B strip object (REQUIRED)
  ws2812b.clear();  // set all pixel colors to 'off'. It only takes effect if pixels.show() is called

 // turn_on_led(3,2, 255, 0, 0);
  //turn_on_led(0,3, 0, 0, 255);
  //turn_on_led(8,7, 0, 255, 0);
  //turn_on_led(5,5, 0, 255, 0);

  //turn_on_row(0, 255, 0, 0);
  clear_display();
}

void clear_display(){
  for(int i = 0; i < 32; i++){
    for (int j = 0; j < 8; j++){
        turn_off_led(i, j);
    }
  }
}

/* 
* Functie: receiveEvent
* Beschrijving: zet de led aan van de coordinaten die worden ontvangen via I2C
* Parameters: Geen 
* Retourneert: Geen 
*/
void receiveEvent(int howMany)
{
  uint8_t x;
  uint8_t y;
  char c;
  struct coordinate* c1;

  while(Wire.available()) // loop through all but the last
  {
    c = Wire.read(); // read the command of what to do

    switch (c){
      case 'b':
        x = Wire.read(); // receive byte as a character
        break;

      case 'c':
        x = Wire.read(); // receive byte as a character
        y = Wire.read(); // receive byte as a character

        c1 = (struct coordinate*)malloc(sizeof(struct coordinate)); // Dynamically allocate memory
        if (c1 != NULL) {
          c1->x = x;
          c1->y = y;
          xTaskCreate(show_frequency, "show_frequency", 2048, (void *)c1, 5, NULL);
        }
       break;

      case 's':
        x = Wire.read(); //receive byte as a character
        Serial.print("starting-up");
        startup_animation();
        break;

    }
  }
}

void show_frequency(void* coordinates) {
  struct coordinate* c = (struct coordinate*)coordinates;

  Serial.print("x: ");
  Serial.print(c->x);
  Serial.print("");

  Serial.print("y: ");
  Serial.print(c->y);
  Serial.println("");

  vTaskDelay(pdMS_TO_TICKS(20));

  turn_off_column(c->x);
  turn_below_point_on(c->x, c->y, 0, 255, 0);

  free(c); // Free dynamically allocated memory
  vTaskDelete(NULL);
}

void fall_to_position(int x, int y){
  for (int i = 7; i >= y; i--) {
    turn_on_led(x, i, 0, 255, 0);
    if( x%2==0 && x == 7 ){
         continue;
    }
    turn_off_led(x, i + 1);
  }
}

void fall_full_row(int x, int y){
  for(int i = 0; i <= y; i++){
    fall_to_position(x, i);
  }

} 

void startup_animation(){
  
  int pixelArray[16];
  generateArray(pixelArray, 16);

  for (int i = 0; i<16; i++){
    fall_full_row(i, pixelArray[i]);
  }
  delay(1000);
  ws2812b.clear();
  ws2812b.show();
}

void generateArray(int array[], int length) {
  for (int i = 0; i < length; i++) {
    array[i] = random(2, 8); // Generates random numbers between 0 and 7
  }
}

void turn_off_led(int x, int y){
  int pixel = (8 * x);

  if (x % 2 == 0){
    pixel += 7;
    pixel -= y;
  }else {
    pixel += y;
  }
    ws2812b.setPixelColor(pixel, ws2812b.Color(0, 0, 0));  // it only takes effect if pixels.show() is called
    ws2812b.show();     
}

void turn_on_led(int x, int y, int r, int g , int b){
  int pixel = (8 * x);

  if (x % 2 == 0){
    pixel += 7;
    pixel -= y;
  }else {
    pixel += y;
  }

  r = (255/7) * y;
  g = 255 - ((255/7) * y);
  b = 0;
       
}
  ws2812b.setPixelColor(pixel, ws2812b.Color(r, g, b));  // it only takes effect if pixels.show() is called
  ws2812b.show();     
}

void set_brightness_of_leds(uint8_t value){
  ws2812b.setBrightness(value);
  ws2812b.show();
}

void turn_off_column(int x){
  for(int i = 0; i < 8; i++){
    turn_off_led(x, i);
  }
}

void turn_on_row(int y, int r, int g, int b){
  for(int i = 0; i < 32; i++){
    turn_on_led(i, y, r, g, b);
  }
}

void turn_below_point_on(int x, int y, int r, int g, int b){
  for(int i = y; i >= 0; i--){
    turn_on_led(x, i, r, g, b);
  }
}

void loop() {
}