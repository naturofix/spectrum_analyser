#include <Adafruit_NeoPixel.h>


#define BPIN 5
#define MPIN 6
#define TPIN 7
#define NUM_LEDS    300


Adafruit_NeoPixel strip_a = Adafruit_NeoPixel(NUM_LEDS, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_b = Adafruit_NeoPixel(NUM_LEDS, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_c = Adafruit_NeoPixel(NUM_LEDS, 7, NEO_GRB + NEO_KHZ800);

void setup() {
  delay( 3000 );
  strip_a.setBrightness(255);
  strip_a.begin();
  strip_a.show();
  
  strip_b.setBrightness(255);
  strip_b.begin();
  strip_b.show();
  
  strip_c.setBrightness(255);
  strip_c.begin();
  strip_c.show();
  delay( 3000 );

}

void loop() {
  Serial.begin(57600);
  int led_nums = 300;
  int delay_num = 5;
  int i;
  for(i=0; i<led_nums; i++) {
    strip_a.setPixelColor(i,255,0,0); 
  }
  strip_a.show();
  delay( 3000 );
   for(i=0; i<led_nums; i++) {
    strip_b.setPixelColor(i,0,255,0); 
  }
  strip_b.show();
  delay( 3000 );
  for(i=0; i<led_nums; i++) {
    strip_c.setPixelColor(i,0,0,255); 
  }

  strip_c.show();
  delay( 3000 );
  
}
