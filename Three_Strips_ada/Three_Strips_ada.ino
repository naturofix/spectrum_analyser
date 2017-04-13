#include <Adafruit_NeoPixel.h>


#define NUM_LEDS 200


Adafruit_NeoPixel strip_a = Adafruit_NeoPixel(NUM_LEDS, 5);
Adafruit_NeoPixel strip_b = Adafruit_NeoPixel(NUM_LEDS, 6);
Adafruit_NeoPixel strip_c = Adafruit_NeoPixel(NUM_LEDS, 7);

void setup() {
  delay( 3000 );
  
  strip_a.begin();
  strip_a.setBrightness(25);
  strip_a.show();
  
  
  strip_b.begin();
  strip_b.setBrightness(25);
  strip_b.show();
  

  strip_c.begin();
  strip_c.setBrightness(25);
  strip_c.show();
  delay( 3000 );

}


void loop() {
  uint8_t cd = 100;
  uint8_t turn_off = 1;
//  Serial.begin(57600);
//  uint8_t led_nums = NUM_LEDS;
  uint8_t delay_num = 5;
  uint8_t i;
//  unit8_t i;
  for(i=0; i<NUM_LEDS; i++) {
    strip_a.setPixelColor(i,255,0,0); 
    strip_b.setPixelColor(i,0,255,0);
    strip_c.setPixelColor(i,0,0,255);
  }
  strip_a.show();
  strip_b.show();
  strip_c.show();
//  if(turn_off == 0){
//    strip_a.show();
//  }
//  delay(cd);
//   for(i=0; i<led_nums; i++) {
//    strip_b.setPixelColor(i,0,255,0); 
//  }
//    if(turn_off == 0){
//    strip_b.show();
//  }
//  delay(cd);
//  for(i=0; i<led_nums; i++) {
//    strip_c.setPixelColor(i,0,0,255); 
//  }
//
//  if(turn_off == 0){
//    strip_c.show();
//  }
//  delay(cd);
//  
//    for(i=0; i<led_nums; i++) {
//    strip_a.setPixelColor(i,0,255,0); 
//  }
//  if(turn_off == 0){
//    strip_a.show();
//  }
//  delay(cd);
//   for(i=0; i<led_nums; i++) {
//    strip_b.setPixelColor(i,0,0,255); 
//  }
//    if(turn_off == 0){
//    strip_b.show();
//  }
//  delay(cd);
//  for(i=0; i<led_nums; i++) {
//    strip_c.setPixelColor(i,255,0,0); 
//  }
//
//  if(turn_off == 0){
//    strip_c.show();
//  }
//  delay(cd);
//  
//    for(i=0; i<led_nums; i++) {
//    strip_a.setPixelColor(i,0,0,255); 
//  }
//  if(turn_off == 0){
//    strip_a.show();
//  }
//  delay(cd);
//   for(i=0; i<led_nums; i++) {
//    strip_b.setPixelColor(i,255,0,0); 
//  }
//    if(turn_off == 0){
//    strip_b.show();
//  }
//  delay(cd);
//  for(i=0; i<led_nums; i++) {
//    strip_c.setPixelColor(i,0,255,0); 
//  }
//
//  if(turn_off == 0){
//    strip_c.show();
//  }
//  delay(cd);
  
}
