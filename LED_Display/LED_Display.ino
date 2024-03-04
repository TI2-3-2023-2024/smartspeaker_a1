/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-ws2812b-led-strip
 */

#include <Adafruit_NeoPixel.h>

#define PIN_WS2812B 12  // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 256  // The number of LEDs (pixels) on WS2812B LED strip

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

void setup() {
  ws2812b.begin();  // initialize WS2812B strip object (REQUIRED)
  ws2812b.clear();  // set all pixel colors to 'off'. It only takes effect if pixels.show() is called

  turn_on_led(3,2, 255, 0, 0);
  turn_on_led(0,3, 0, 0, 255);
  turn_on_led(8,7, 0, 255, 0);
  turn_on_led(5,5, 0, 255, 0);

  turn_on_row(0, 255, 0, 0);
  turn_below_point_on(3, 6, 0, 0, 255);
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
    ws2812b.setBrightness(100); // set brightness of the pixels
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
