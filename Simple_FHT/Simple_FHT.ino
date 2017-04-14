#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fht

#include <FHT.h> // include the library

#define PIXELS 900  // Number of pixels in the string


// These values depend on which pin your string is connected to and what board you are using 
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation

// These values are for the pin that connects to the Data Input pin on the LED strip. They correspond to...

// Arduino Yun:     Digital Pin 8
// DueMilinove/UNO: Digital Pin 12
// Arduino MeagL    PWM Pin 4

// You'll need to look up the port/bit combination for other boards. 

// Note that you could also include the DigitalWriteFast header file to not need to to this lookup.

#define PIXEL_PORT  PORTB  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRB// Port of the pin the pixels are connected to
#define PIXEL_BIT   4      // Bit of the pin the pixels are connected to

// These are the timing constraints taken mostly from the WS2812 datasheets 
// These are chosen to be conservative and avoid problems rather than for maximum throughput 

#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns

#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns

#define RES 6000    // Width of the low gap between bits to cause a frame to latch

// Here are some convience defines for using nanoseconds specs to generate actual CPU delays

#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives

#define CYCLES_PER_SEC (F_CPU)

#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

// Actually send a bit to the string. We must to drop to asm to enusre that the complier does
// not reorder things and make it so the delay happens in the wrong place.




inline void sendBit( bool bitVal ) {
  
    if (  bitVal ) {        // 0 bit
      
    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"                                // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),    // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      [offCycles]   "I" (NS_TO_CYCLES(T1L) - 2)     // Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

    );
                                  
    } else {          // 1 bit

    // **************************************************************************
    // This line is really the only tight goldilocks timing in the whole program!
    // **************************************************************************


    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"        // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
      "nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T0H) - 2),
      [offCycles] "I" (NS_TO_CYCLES(T0L) - 2)

    );
      
    }
    
    // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)
    // Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
    // This has thenice side effect of avoid glitches on very long strings becuase 

    
}  

  
inline void sendByte( unsigned char byte ) {
    
    for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {
      
      sendBit( bitRead( byte , 7 ) );                // Neopixel wants bit in highest-to-lowest order
                                                     // so send highest bit (bit #7 in an 8-bit byte since they start at 0)
      byte <<= 1;                                    // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
      
    }           
} 

/*

  The following three functions are the public API:
  
  ledSetup() - set up the pin that is connected to the string. Call once at the begining of the program.  
  sendPixel( r g , b ) - send a single pixel to the string. Call this once for each pixel in a frame.
  show() - show the recently sent pixel on the LEDs . Call once per frame. 
  
*/


// Set the specified pin up as digital out

void ledsetup() {
  
  bitSet( PIXEL_DDR , PIXEL_BIT );
  
}

inline void sendPixel( unsigned char r, unsigned char g , unsigned char b )  {  
  
  sendByte(g);          // Neopixel wants colors in green then red then blue order
  sendByte(r);
  sendByte(b);
  
}


// Just wait long enough without sending any bots to cause the pixels to latch and display the last sent frame

void show() {
  _delay_us( (RES / 1000UL) + 1);       // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}


/*

  That is the whole API. What follows are some demo functions rewriten from the AdaFruit strandtest code...
  
  https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino
  
  Note that we always turn off interrupts while we are sending pixels becuase an interupt
  could happen just when we were in the middle of somehting time sensitive.
  
  If we wanted to minimize the time interrupts were off, we could instead 
  could get away with only turning off interrupts just for the very brief moment 
  when we are actually sending a 0 bit (~1us), as long as we were sure that the total time 
  taken by any interrupts + the time in our pixel generation code never exceeded the reset time (5us).
  
*/


// Display a single color on the whole string

void showColor( unsigned char r , unsigned char g , unsigned char b ) {
  
  cli();  
  for( int p=0; p<PIXELS; p++ ) {
    sendPixel( r , g , b );
  }
  sei();
  show();
  
}

// Fill the dots one after the other with a color
// rewrite to lift the compare out of the loop
void colorWipe(unsigned char r , unsigned char g, unsigned char b, unsigned  char wait ) {
  for(unsigned int i=0; i<PIXELS; i+= (PIXELS/60) ) {
    
    cli();
    unsigned int p=0;
    
    while (p++<=i) {
        sendPixel(r,g,b);
    } 
     
    while (p++<=PIXELS) {
        sendPixel(0,0,0);  
      
    }
    
    sei();
    show();
    delay(wait);
  }
}

// Theatre-style crawling lights.
// Changes spacing to be dynmaic based on string size

#define THEATER_SPACING (PIXELS/20)

void theaterChase( unsigned char r , unsigned char g, unsigned char b, unsigned char wait ) {
  
  for (int j=0; j< 3 ; j++) {  
  
    for (int q=0; q < THEATER_SPACING ; q++) {
      
      unsigned int step=0;
      
      cli();
      
      for (int i=0; i < PIXELS ; i++) {
        
        if (step==q) {
          
          sendPixel( r , g , b );
          
        } else {
          
          sendPixel( 0 , 0 , 0 );
          
        }
        
        step++;
        
        if (step==THEATER_SPACING) step =0;
        
      }
      
      sei();
      
      show();
      delay(wait);
      
    }
    
  }
  
}
        


// I rewrite this one from scrtach to use high resolution for the color wheel to look nicer on a *much* bigger string
                                                                            
void rainbowCycle(unsigned char frames , unsigned int frameAdvance, unsigned int pixelAdvance ) {
  
  // Hue is a number between 0 and 3*256 than defines a mix of r->g->b where
  // hue of 0 = Full red
  // hue of 128 = 1/2 red and 1/2 green
  // hue of 256 = Full Green
  // hue of 384 = 1/2 green and 1/2 blue
  // ...
  
  unsigned int firstPixelHue = 0;     // Color for the first pixel in the string
  
  for(unsigned int j=0; j<frames; j++) {                                  
    
    unsigned int currentPixelHue = firstPixelHue;
       
    cli();    
        
    for(unsigned int i=0; i< PIXELS; i++) {
      
      if (currentPixelHue>=(3*256)) {                  // Normalize back down incase we incremented and overflowed
        currentPixelHue -= (3*256);
      }
            
      unsigned char phase = currentPixelHue >> 8;
      unsigned char step = currentPixelHue & 0xff;
                 
      switch (phase) {
        
        case 0: 
          sendPixel( ~step , step ,  0 );
          break;
          
        case 1: 
          sendPixel( 0 , ~step , step );
          break;

        case 2: 
          sendPixel(  step ,0 , ~step );
          break;
          
      }
      
      currentPixelHue+=pixelAdvance;                                      
      
                          
    } 
    
    sei();
    
    show();
    
    firstPixelHue += frameAdvance;
           
  }
}

  
// I added this one just to demonstrate how quickly you can flash the string.
// Flashes get faster and faster until *boom* and fade to black.

void detonate( unsigned char r , unsigned char g , unsigned char b , unsigned int startdelayms) {
  while (startdelayms) {
    
    showColor( r , g , b );      // Flash the color 
    showColor( 0 , 0 , 0 );
    
    delay( startdelayms );      
    
    startdelayms =  ( startdelayms * 4 ) / 5 ;           // delay between flashes is halved each time until zero
    
  }
  
  // Then we fade to black....
  
  for( int fade=256; fade>0; fade-- ) {
    
    showColor( (r * fade) / 256 ,(g*fade) /256 , (b*fade)/256 );
        
  }
  
  showColor( 0 , 0 , 0 );
  
    
}

void setup() {
  delay(3000);   
  ledsetup(); 

  Serial.begin(9600); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  for(int i = 0;i < 10; i++){
    showColor(0, 0, 0);
    delay(3000);
    showColor(25, 0, 0);
    delay(3000);
    showColor(0, 25, 0);
    delay(3000);
    showColor(0, 0, 25);
    delay(3000);
    showColor(0, 0, 0);
    delay(3000);
  }
}


void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    sei();
//    Serial.println("start");
//    for (byte i = 0 ; i < FHT_N/2 ; i++) {
//      Serial.println(fht_log_out[i]); // send out the data
//    }
//  }
    byte print_test = 1; //1 to print variables
    if(print_test == 1){
      Serial.begin(9600);
    }
    byte colrange = 2; 
    // 1 red to green
    // 2 green to blue
    byte test = 1; //1 to turn all lights on, showing the rainbow disttribution
    byte dim = 1; // 1 to dim all light, by the percentage in brightness
    byte baseline = 120;// threshold for triggering lights
    long_sa(colrange,print_test,test,dim,baseline);
    show();
    delay(1);
    if(test != 1){
      base_color();
    }
}
//  }
  
  return;
  
}


void base_color(){
  showColor(0, 0, 0);
}
void first_try(){
  int base_line = 120;
  int base_led = 60;
    float led_nums = 0;
    for(int f = 2; (f < FHT_N/2)/3; f++){ //third of bins from red to green
      int bin = fht_log_out[f];
      if(bin>base_line-(f+2)){
        led_nums = bin/5;
        if(led_nums > 30){
          led_nums = 30;
        }
        if(led_nums < 2){
          led_nums = 0;
        }
        for(int i=0+(18*f); i<led_nums+(18*f); i++) {
  //        base_strip.setPixelColor(i,255-(16*f),16*f,0);
          sendPixel(255-(16*f),16*f,0);
  
  //        sendPixel(r,g,b)
      }
    }
  }
}


void long_sa(byte colrange, byte print_test, byte test, byte dim,byte base_line){

  float brightness = 0.5; // 0 to 1
  int base_reduce = 1; // divide internsity to trigger less lights
  int col_len = 900/(FHT_N/2); // number of lights per bin
  float led_nums = 0;
  float divisions = (FHT_N/2)/3;
  float r = 255;
  float g = 0;
  float b = 0;
  byte start = 1;
  int cbin = 0;
  int x = 0; //total number of leds
  int rb;
  int gb;
  int bb;

  if(colrange == 1){
    for(byte f = start; f < start+divisions;f++){
      int bin = fht_log_out[f]; //intensity of frequency
      int i = 0; 
      led_nums = 0; //reset Led nums
      r = r - 255/divisions;
      if(r < 0){r = 0;}
      g = g + 255/divisions;
      if(g > 255){g = 255;}
      if(dim == 1){
        rb = r*brightness;
        gb = g*brightness;
        bb = b*brightness;
      }
      else{
        rb = r;
        gb = g;
        bb = b;
      }
  
      if( bin > base_line || test == 1){
        led_nums = bin/base_reduce;
        if(led_nums > col_len || test == 1){
          led_nums = col_len;
        }
        for(i; i < led_nums; i++){
          sendPixel(rb,gb,bb);
          x += 1;
          if(print_test == 1){
            Serial.print("x : ");
            Serial.print(x);
            Serial.print("    i : ");
            Serial.print(i);
            Serial.print("    rgb : ");
            Serial.print(r);
            Serial.print(", ");
            Serial.print(g);
            Serial.print(", ");
            Serial.println(b);
          }
          
        }
      }
      for(i;i < col_len;i++){
        base_color();
        x += 1;
       if(print_test == 1){
          Serial.print("x : ");
          Serial.print(x);
          Serial.print("    i : ");
          Serial.print(i);
          Serial.print("    rgb : ");
          Serial.print(0);
          Serial.print(", ");
          Serial.print(0);
          Serial.print(", ");
          Serial.println(5);
        }
      }
      
    cbin += 1;
    if(print_test == 1){
        Serial.print("cbin : ");
        Serial.println(cbin);
        Serial.print("x : ");
        Serial.print(x);
        Serial.print("         i : ");
        Serial.println(i);
        Serial.print("LED NUM :");
        Serial.println(led_nums);
        Serial.print("Divisions : ");
        Serial.println(divisions);
        Serial.println((255/divisions)*cbin);
        Serial.print(r);
        Serial.print(", ");
        Serial.print(g);
        Serial.print(", ");
        Serial.println(b);
        Serial.println();
      }
    }
  }

  if(colrange == 2){
  
    r = 0;
    g = 255;
    b = 0;
    for(byte f = start+divisions; f < f + divisions;f++){
      int bin = fht_log_out[f]; //intensity of frequency
      int i = 0; 
      led_nums = 0; //reset Led nums
      
      g = g - 255/divisions;
      if(r < 0){r = 0;}
      b = b + 255/divisions;
      if(g > 255){g = 255;}
      if(dim == 1){
        rb = r*brightness;
        gb = g*brightness;
        bb = b*brightness;
      }
      else{
        rb = r;
        gb = g;
        bb = b;
      }
  
      if( bin > base_line || test == 1){
        led_nums = bin/base_reduce;
        if(led_nums > col_len || test == 1){
          led_nums = col_len;
        }
        for(i; i < led_nums; i++){
          sendPixel(rb,gb,bb);
          x += 1;
          if(print_test == 1){
            Serial.print("x : ");
            Serial.print(x);
            Serial.print("    i : ");
            Serial.print(i);
            Serial.print("    rgb : ");
            Serial.print(r);
            Serial.print(", ");
            Serial.print(g);
            Serial.print(", ");
            Serial.println(b);
          }
          
        }
      }
      for(i;i < col_len;i++){
        base_color();
        x += 1;
       if(print_test == 1){
          Serial.print("x : ");
          Serial.print(x);
          Serial.print("    i : ");
          Serial.print(i);
          Serial.print("    rgb : ");
          Serial.print(0);
          Serial.print(", ");
          Serial.print(0);
          Serial.print(", ");
          Serial.println(5);
        }
      }
      
    cbin += 1;
    if(print_test == 1){
        Serial.print("cbin : ");
        Serial.println(cbin);
        Serial.print("x : ");
        Serial.print(x);
        Serial.print("         i : ");
        Serial.println(i);
        Serial.print("LED NUM :");
        Serial.println(led_nums);
        Serial.print("Divisions : ");
        Serial.println(divisions);
        Serial.println((255/divisions)*cbin);
        Serial.print(r);
        Serial.print(", ");
        Serial.print(g);
        Serial.print(", ");
        Serial.println(b);
        Serial.println();
      }
    }
  }
}




