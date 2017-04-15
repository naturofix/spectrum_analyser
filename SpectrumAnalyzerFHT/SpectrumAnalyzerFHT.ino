#include <avr/pgmspace.h>
#define FHT_N 256
#define LIN_OUT 1
#include <FHT.h>
#include <Adafruit_NeoPixel.h>

//
// Spectrum analyzer
// Pete Reiter
// Spectrum analyzer with a Adafruit neopixel strip as output. Unlike an ordinary 2-dimensional spectrum
// analyzer that uses Y-axis is display intensity of each frequency band, this uses color and brightness of
// the LEDs to indicate the intensity. This code was originally adapted from the PICCOLO tiny music visualizer
// on the Adafruit web site.
// 
// Software requirements:
// - FHT library for Arduino
// - Adafruit Neopixel library.

#define PIN 12 // digital pin for programming neopixels
#define NUM_PIXELS 144 // this is the size of my neopixel strip           

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Microphone connects to Analog Pin 0.  Corresponding ADC channel number
// varies among boards...it's ADC0 on Uno and Mega, ADC7 on Leonardo.
// Other boards may require different settings; refer to datasheet.
#ifdef __AVR_ATmega32U4__
 #define ADC_CHANNEL 7
#else
 #define ADC_CHANNEL 0
#endif

volatile uint32_t samplePos = 0;     // Buffer position counter

static const uint8_t PROGMEM
  // This is low-level noise that's subtracted from each FHT output column
  // This was experimentally determined in a quiet room.
  noise[128]={ 
    50, 12, 10, 8, 7, 6, 6, 5, // 0
    5, 5, 4, 4, 4, 4, 4, 4,    // 8
    4, 4, 4, 4, 4, 4, 4, 4,    // 16
    4, 4, 4, 4, 4, 4, 4, 4,    // 24
    4, 4, 4, 4, 4, 4, 4, 4,    // 32
    3, 3, 3, 4, 3, 3, 3, 3,    // 40
    3, 3, 3, 3, 3, 3, 3, 3,    // 48    
    3, 3, 3, 3, 3, 3, 3, 3,    // 56
    3, 3, 3, 3, 3, 3, 3, 3,    // 64
    3, 3, 3, 3, 3, 3, 3, 3,    // 72
    3, 3, 3, 3, 3, 3, 3, 3,    // 80
    3, 3, 3, 3, 3, 3, 3, 3,    // 88
    3, 3, 3, 3, 3, 3, 3, 3,    // 96
    3, 3, 3, 3, 3, 3, 3, 3,    // 104
    3, 3, 3, 3, 3, 3, 3, 3,    // 112
    3, 3, 3, 3, 3, 3, 3, 3     // 120
};

static const uint8_t PROGMEM
  calibration[128] = {
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,  
    3, 3, 3, 3, 3, 3, 3, 3,  
    3, 3, 3, 3, 3, 3, 3, 3,  
    3, 3, 3, 3, 3, 3, 3, 3  
  } ;
  
static const byte  PROGMEM slots[160] =
  // combine the 128 FFT frequency output slots into buckets based on octaves.
  // for the lower frequencies multiple leds display a single frequency slot.
  // for the higher frequencies multiple frequency slots are combined into a aingle led 
   {
    1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 
    9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15,
   16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127
   };

// This is the mapping from the values in the buckets to the colors representing those values. These numbers were generated by some C code that I've pasted into the bottom of this
// file. The colors go from blue->green->red with an increase in intensity as the values increase. There's also a log10 based response curve figured in.
#define NUM_COLORS 64
static const uint32_t PROGMEM colors[NUM_COLORS] = { 
0x42, 0x1c37, 0x332e, 0x4823, 0x5b1a, 0x6c11, 0x7c08, 0x8c00, 0x88900, 0x108600, 0x178300, 0x1e8100, 0x267e00, 0x2d7b00, 0x347700, 0x3a7400, 0x407100, 0x466e00, 0x4d6b00, 0x526800, 0x586500, 0x5e6200, 0x636000, 0x685d00, 0x6d5a00, 0x725700, 0x775400, 0x7c5100, 0x804f00, 0x864c00, 0x8a4900, 0x8f4600, 0x924400, 0x974100, 0x9b3f00, 0x9f3c00, 0xa33a00, 0xa83700, 0xab3500, 0xaf3200, 0xb33000, 0xb72e00, 0xbb2b00, 0xbe2900, 0xc12700, 0xc52400, 0xc92200, 0xcc2000, 0xd01e00, 0xd31c00, 0xd71900, 0xda1700, 0xdc1500, 0xe01300, 0xe31100, 0xe60f00, 0xe90d00, 0xec0b00, 0xf00800, 0xf30600, 0xf60400, 0xf90200, 0xfc0000, 0xff0000
}; 

#define THRESHOLD 1
// The prescaler settings determine the frequency of audio sampling. We can sample higher
// frequencies with a lower prescaler value, but it will also raise the lowest frequency that
// we can sample. With this setup, I seem to be getting around 300Hz-9.6KHz response. There is
// some aliasing going on, meaning that frequencies > 9.6KHz will show up in lower frequency
// response.
void setup() {
  // Init ADC free-run mode; f = ( 16MHz/prescaler ) / 13 cycles/conversion 
  ADMUX  = ADC_CHANNEL; // Channel sel, right-adj, use AREF pin
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADSC)  | // ADC start
           _BV(ADATE) | // Auto trigger
           _BV(ADIE)  | // Interrupt enable
           // select the prescaler value. Note that the max frequency our FFT will
           // display is half the sample rate.
//           _BV(ADPS2) | _BV(ADPS0); // 32:1 / 13 = 38,460 Hz
           _BV(ADPS2) | _BV(ADPS1); // 64:1 / 13 = 19,230 Hz
//           _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 128:1 / 13 = 9615 Hz
  ADCSRB = 0;                // Free run mode, no high MUX bit
  DIDR0  = 1 << ADC_CHANNEL; // Turn off digital input for ADC pin
  TIMSK0 = 0;                // Timer0 off

  // Neopixels setup
  // Initialize all pixels to 'off'
  strip.setBrightness(255);
  strip.begin(); // Initialize all pixels to 'off'
  cli();         // disable interrupts when writing neopixels   
  strip.show();
  sei();         // Enable interrupts
  Serial.begin(9600);  // set up Serial library at 9600 bps for debugging purposes

}


void loop() {
  uint16_t  x, L;
  while(ADCSRA & _BV(ADIE)); // Wait for audio sampling to finish

  fht_window();
  fht_reorder();
  fht_run();
  fht_mag_lin();
  samplePos = 0;                   // Reset sample counter
  ADCSRA |= _BV(ADIE);             // Resume sampling interrupt
  
  // Remove noise
  for(x=0; x<FHT_N/2; x++) {
//    L = (pgm_read_byte(&noise[x]) << 6);
//    fht_lin_out[x] = (fht_lin_out[x] <= L) ? 0 : (fht_lin_out[x] - L);
  }
  for(int i = 0;i < 5;i++){
    Serial.print(fht_lin_out[i]);
    Serial.print(", ");
  }
  Serial.println();
  
  fht_lin_out[1] = (fht_lin_out[1] <= 500) ? 0 : (fht_lin_out[1] - 400);
  
  // Combine the frequency output slots into a smaller number of output
  // buckets
  int previous = -1, current = -1, next = -1;
  for (int i = 0; i < strip.numPixels(); i++)
  {
      int value = 0;
      previous = current;
      current = (next == -1) ? pgm_read_byte(&slots[i]) : next;
      next = pgm_read_byte(&slots[i+1]);
      if ((next - current) > 2)
      { // this led combines the results of multiple frequency slots
        for (int j = current; j < next; j++)
          value += fht_lin_out[j];
      }
      // if we have one slot going to multiple leds, do some averaging on the boundary so that the transition isn't so abrupt
      else if (current == previous && current != next)
        value = (fht_lin_out[current] + fht_lin_out[next])/2;
      else // just use the value
        value = fht_lin_out[current];

      value >>= 6;
      if (value < 6)
        value = 0;
      if (value >= NUM_COLORS)
        value = NUM_COLORS - 1;      
      strip.setPixelColor(i, pgm_read_dword(&colors[value]));
  }
  cli();        // no interrupts while writing the neopixels
  strip.show();
  sei();        // restore interrupts
}

// interrupt service routine. This gets called each time the ADC finishes 1 sample.
ISR(ADC_vect) { // Audio-sampling interrupt
  // shift the unsigned input to be centered around 0. The 10-bit ADC is 
  // capable of producing values from 0 - 1023, but with a microphone that outputs
  // 2V max and a reference voltage of 3.3V we will never hit the max.
  fht_input[samplePos] = (ADC - 512) << 6;
//  Serial.println(fht_input[samplePos]);
  if(++samplePos >= FHT_N) ADCSRA &= ~_BV(ADIE); // Buffer full, interrupt off
}

//
// Some C code I used to generate the values->colors map. I didn't run this code on the Arduino. I used ideone.com
// and then pasted the output into my arduino code.
#if 0
#include <iostream>
#include <stdint.h>
#include <math.h>
using namespace std;


  
const int numValues = 64; // number of colors in our output array. This should correspond 
                          // to the max value you want to display.
double breakPoint = log10(numValues)/2.0;

//
// Blend two colors together based on the ratio. ratio of 0.0 will be 100% color a and
// ratio of 1.0 will be 100% color b.
uint32_t blend (uint32_t ina, uint32_t inb, double ratio)
{
  int r = (((ina >> 16) & 0xff) * (1.0-ratio)) + (((inb >> 16) & 0xff) * ratio);
  int g = (((ina >> 8) & 0xff) * (1.0-ratio)) + (((inb >> 8) & 0xff) * ratio);
  int b = (((ina >> 0) & 0xff) * (1.0-ratio)) + (((inb >> 0) & 0xff) * ratio);
  return ((r << 16) | (g << 8) | b);
}

//
// Scale the intensity of the passed in color. I am using max brightness colors and 
// 0.0 - 1.0 as the scale value.
uint32_t scale (uint32_t ina, double scale_value)
{
  int r = ((ina >> 16) & 0xff) * scale_value;
  int g = ((ina >> 8) & 0xff) * scale_value;
  int b = ((ina >> 0) & 0xff) * scale_value;
  return ((r << 16) | (g << 8) | b);  
}

//  
// Fade blue -> green -> red. I've built in a logarithmic response to make it more of a
// dB meter.
int main() { 
  for (int i = 1; i <= numValues; i++)
  {
    double logValue = log10(i);
    double scaleValue = log10(i+2) / log10(numValues+2);
    double ratio = (logValue < breakPoint) ? logValue / breakPoint : (logValue - breakPoint) / breakPoint;
    uint32_t color = 0;

    if (logValue < breakPoint)
      color = blend (0x0000ff, 0x00ff00, ratio);
    else
      color = blend (0x00ff00, 0xff0000, ratio);
    color = scale (color, scaleValue);
    cout <<  "0x" << hex << color << ", ";
  }
  return 0; 
}
#endif
