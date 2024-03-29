/* 
* Bestandsnaam: LED_DISPLAY.ino 
* Auteur: Niels en Thijme
* Datum: 5 maart 2024 
* Beschrijving: Dit bestand bevat de functionaliteit van de esp32 om een LED display aan kan sturen en I2C communicatie ontvangen
*/


#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <WireSlave.h>
#include "driver/i2c.h"

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04

#define PIN_WS2812B 12  // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 256  // The number of LEDs (pixels) on WS2812B LED strip

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

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

  while(Wire.available()) // loop through all but the last
  {
    c = Wire.read(); // read the command of what to do
    Serial.print("c: ");
    Serial.print(c);         // print the character
    Serial.println("");

    switch (c){
      case 'b':
        x = Wire.read(); // receive byte as a character
        Serial.print("x: ");
        Serial.print(x);         // print the character
        Serial.println("");
        set_brightness_of_leds(x);
        break;

      case 'c':
        x = Wire.read(); // receive byte as a character
        Serial.print("x: ");
        Serial.print(x);         // print the character
        Serial.println("");

        y = Wire.read(); // receive byte as a character
        Serial.print("y: ");
        Serial.print(y);         // print the character
        Serial.println("");
        turn_below_point_on(x, y, 0, 255, 0);
        break;

      case 's':
        x = Wire.read(); //receive byte as a character
        Serial.print("starting-upt");
        startup_animation();
        break;

    }
  }
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
  ws2812b.clear();
  ws2812b.show();

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

    ws2812b.setPixelColor(pixel, ws2812b.Color(r, g, b));  // it only takes effect if pixels.show() is called
    ws2812b.show();     
}

void set_brightness_of_leds(uint8_t value){
  ws2812b.setBrightness(value);
  ws2812b.show();
}

void turn_on_row(int y, int r, int g, int b){
  for(int i = 0; i < 32; i++){
    turn_on_led(i, y, r, g, b);
  }
}

void turn_below_point_on(uint8_t x, uint8_t y, int r, int g, int b){
  for(int i = y; i >= 0; i--){
    turn_on_led(x, i, r, g, b);
  }
}

void loop() {
}