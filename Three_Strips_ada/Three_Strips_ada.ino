#include <Adafruit_NeoPixel.h>


#define BPIN 5
#define MPIN 6
#define TPIN 7
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
  int cd = 100;
  int turn_off = 0;
//  Serial.begin(57600);
  int led_nums = NUM_LEDS;
  int delay_num = 5;
  int i;
  for(i=0; i<led_nums; i++) {
    strip_a.setPixelColor(i,255,0,0); 
  }
  if(turn_off == 0){
    strip_a.show();
  }
  delay(cd);
   for(i=0; i<led_nums; i++) {
    strip_b.setPixelColor(i,0,255,0); 
  }
    if(turn_off == 0){
    strip_b.show();
  }
  delay(cd);
  for(i=0; i<led_nums; i++) {
    strip_c.setPixelColor(i,0,0,255); 
  }

  if(turn_off == 0){
    strip_c.show();
  }
  delay(cd);
  
    for(i=0; i<led_nums; i++) {
    strip_a.setPixelColor(i,0,255,0); 
  }
  if(turn_off == 0){
    strip_a.show();
  }
  delay(cd);
   for(i=0; i<led_nums; i++) {
    strip_b.setPixelColor(i,0,0,255); 
  }
    if(turn_off == 0){
    strip_b.show();
  }
  delay(cd);
  for(i=0; i<led_nums; i++) {
    strip_c.setPixelColor(i,255,0,0); 
  }

  if(turn_off == 0){
    strip_c.show();
  }
  delay(cd);
  
    for(i=0; i<led_nums; i++) {
    strip_a.setPixelColor(i,0,0,255); 
  }
  if(turn_off == 0){
    strip_a.show();
  }
  delay(cd);
   for(i=0; i<led_nums; i++) {
    strip_b.setPixelColor(i,255,0,0); 
  }
    if(turn_off == 0){
    strip_b.show();
  }
  delay(cd);
  for(i=0; i<led_nums; i++) {
    strip_c.setPixelColor(i,0,255,0); 
  }

  if(turn_off == 0){
    strip_c.show();
  }
  delay(cd);
  
}
