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

  turn_on_led(3,2, 255, 0, 0);
  turn_on_led(0,3, 0, 0, 255);
  turn_on_led(8,7, 0, 255, 0);
  turn_on_led(5,5, 0, 255, 0);

  turn_on_row(0, 255, 0, 0);
}

void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}

void turn_on_led(int x, int y, int r, int g , int b){
  int pixel = (8 * x);

  if (x % 2 == 0){
    pixel += 7;
    pixel -= y;
  }else {
    pixel += y;
  }
    ws2812b.setPixelColor(pixel, ws2812b.Color(r, g, b));  // it only takes effect if pixels.show() is called
    ws2812b.setBrightness(50); // set brightness of the pixels
    ws2812b.show();     
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