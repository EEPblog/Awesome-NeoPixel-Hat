//MIT License
//
//Copyright (c) 2017 Angelo Pescarini (aka EEPblog)
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
//===========================================================================================================
#include <Adafruit_NeoPixel.h>

#define STAT 8  -  -  -  -  -  -  -  -  -  -  -  - //status pin from the Bluetooth module

int mode = 100;                                    //define everything global
int spd, red, grn, blu, showVolts;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(36, 6, NEO_GRB + NEO_KHZ800); //Start the strip with 36 LEDs on pin 6

//===========================================================================================================
void setup() {
  strip.begin();                                    //begin the NeoPixel strip
  strip.show();
  Serial.begin(9600);

  bootAnimation();                                  //play a little boot animation
  
  for (int i = 0; i <= 25; i++) {                   //fade in a damp white color
    for (int j = 0; j < strip.numPixels(); j++) {   //so we know it hasn't connected
      strip.setPixelColor(j, i, i, i);              //to a device yet.
    }
    strip.show();
    delay(50);
  }
  while (!digitalRead(STAT)) {}                     //wait for the Bluetooth module
  for (int x = 25; x >= 0; x--) {                   //to report, that a device has
    for (int j = 0; j < strip.numPixels(); j++) {   //connected to the hat, and if yes,
      strip.setPixelColor(j, x, x, x);              //fade out the current color.
    }
    strip.show();
    delay(50);
  }
}
//===========================================================================================================

void loop() {
  receive();                                        //receive the data from terminal
  if (showVolts) {                                  //if there was a request for voltage
    float voltage = (analogRead(A0) / 1024.0) * 5.0;  //measure it, convert it to a whole
    byte val = floor(voltage * 10);                 //number and send it. I'm multiplying
    Serial.println(val);                            //it by ten, so i can than get one
                                                    //decimal point in the app later.
  }
  if (showVolts == 2) {                             //if there was a request for debug info,
    Serial.print("current setting: ");              //send it.
    Serial.print(mode); Serial.print(", ");
    Serial.print(spd); Serial.print(", ");
    Serial.print(red); Serial.print(", ");
    Serial.print(grn); Serial.print(", ");
    Serial.println(blu);
  }
  showVolts = 0;                                    //resets the request integer so it doesn't flood the terminal
  
  strip.setBrightness(255);                         //resets the strip brightness
  
  switch (mode) {                                   //checks the current mode and sets up the animations accordingly
    default:
      for (int j = 0; j < strip.numPixels(); j++) { //this animation shows, that something's not quite right :/
        strip.setPixelColor(j, 0, 0, 0); //clears the strip
      }
      ring(8, 0, 0);
      break;
    case 100:                                        //this is the "first power-on" animation, it is saying, that
      spd = 20;                                      //it is definitely ready to receive commands
      blu = 10;
      breathe(20, 0, 0, 10);
      break;
    case 1:                                          //mode 1: spectrum effect (or hue effect), ignores R, G, B values
      hue(spd);
      break;
    case 2:                                          //mode 2: static color, ignores the speed (spd) value
      ring(red, grn, blu);
      break;
    case 3:                                          //mode 3: breathing effect
      breathe(spd, red, grn, blu);
      break;
    case 4:                                          //mode 4: beacon effect
      cop(spd, red, grn, blu);
      break;
    case 5:                                          //mode 5: theater style lights, changes colors throughout, ignores R, G, B values
      theaterChaseRainbow(spd);
      break;
    case 6:                                          //mode 6: gradually changes the colors, not all colors of the spectrum are shown at the same time,
      hue2(spd);                                     //you'll se what i mean... ignores R, G, B values; currently unsupported in the official app.
      break;
    case 7:                                          //mode 7: same as mode 5, except it has user definable color; currently unsupported in the official app.
      theaterChase(spd, red, grn, blu);
      break;
//    case 8:                                        //mode 8: Knight Rider, this was just a test, was't thoroughly thought through and is not currently supported in the official app anyway...
//      knightRider(spd, 8);
//      break;
  }
}
//===========================================================================================================

//-------------------------------------------------------------------------------------MODE 1: Color Spectrum
void hue() {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    receive();
    delay(spd);
  }
}
//-------------------------------------------------------------------------------------MODE 6: Color Spectrum 2
void hue2() {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    receive();
    delay(spd);
  }
}
//-------------------------------------------------------------------------------------Color changing FX generator
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
//-------------------------------------------------------------------------------------MODE 4: Beacon
void cop() {
  for (int i = 0; i < strip.numPixels(); i++) {
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, 0, 0, 0); //clears the strip
    }
    strip.setPixelColor(i, red, grn, blu);
    strip.setPixelColor(i - 1, red, grn, blu);
    strip.setPixelColor(i - 2, red, grn, blu);
    strip.setPixelColor(i - 3, red, grn, blu);
    strip.setPixelColor(i - 4, red, grn, blu);
    for (int j = 0; j < (4 - i); j++) {
      strip.setPixelColor(strip.numPixels() - j, red, grn, blu);
    }
    receive();
    strip.show();
    delay(spd);
  }
}
//-------------------------------------------------------------------------------------MODE 2: Static Color
void ring(int r, int g, int b) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, r, g, b);
    strip.show();
    delay(15);
  }
}
//-------------------------------------------------------------------------------------MODE 3: Breathing Effect
void breathe() {

  for (int i = 0; i <= 255; i++) {
    strip.setBrightness(i); //clears the strip
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, red, grn, blu);
    }
    strip.show();
    receive();
    delay(spd);
  }
  for (int i = 255; i >= 0; i--) {
    strip.setBrightness(i); //clears the strip
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, red, grn, blu);
    }
    strip.show();
    receive();
    delay(spd);
  }
}
//-------------------------------------------------------------------------------------MODE 5: Rainbow Theatre Effect
void theaterChaseRainbow(int wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(spd);
      receive();
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
//-------------------------------------------------------------------------------------MODE 7: Static Color Theatre Effect
void theaterChase(uint8_t sp, int r, int g, int b) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, red, grn, blu);  //turn every third pixel on
      }
      strip.show();
      receive();
      delay(spd);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
//-------------------------------------------------------------------------------------MODE 8: Knight Rider (NOT RECCOMENDED / ALPHA TEST)
void knightRider(uint16_t sp, uint8_t width) {                                         //note: was ported from somewhere else, but it doesn't
  width = red;                                                                         //      quite work out.
  uint32_t old_val[strip.numPixels()]; // up to 256 lights!
  // Larson time baby!
  uint32_t color = 0xFF1000;
  for (int count = 1; count < strip.numPixels(); count++) {
    strip.setPixelColor(count, color);
    old_val[count] = color;
    for (int x = count; x > 0; x--) {
      old_val[x - 1] = dimColor(old_val[x - 1], width);
      strip.setPixelColor(x - 1, old_val[x - 1]);
    }
    strip.show();
    receive();
    width = red;
    delay(spd);
  }
  for (int count = strip.numPixels() - 1; count >= 0; count--) {
    strip.setPixelColor(count, color);
    old_val[count] = color;
    for (int x = count; x <= strip.numPixels() ; x++) {
      old_val[x - 1] = dimColor(old_val[x - 1], width);
      strip.setPixelColor(x + 1, old_val[x + 1]);
    }
    strip.show();
    receive();
    width = red;
    delay(spd);
  }
}

uint32_t dimColor(uint32_t color, uint8_t width) {
  return (((color & 0xFF0000) / width) & 0xFF0000) + (((color & 0x00FF00) / width) & 0x00FF00) + (((color & 0x0000FF) / width) & 0x0000FF);
}
//-------------------------------------------------------------------------------------Data receive handler

bool receive() {
  bool newData = 1;
  while (Serial.available() > 0) {                            //read the incomming data form serial monitor in
    showVolts = Serial.parseInt();                            //the following data format:
    int amode = Serial.parseInt();                            //Request for send, Mode, Speed, Red, Green, Blue
    int aspd = Serial.parseInt();                             //Request for send - 0 = no request for batt voltage; 1 = just batt voltage; 2 = batt voltage and current settings
    int ared = Serial.parseInt();                             //Mode - Sets the animation mode
    int agrn = Serial.parseInt();                             //Speed - Sets the speed for the current mode, if applicable
    int ablu = Serial.parseInt();                             //Red, Green, Blue - color values for the strip, if applicable
                                                              //for example: 0,3,20,255,0,0 - will make the strip breathe with 20ms steps in red color
                                                              //             2              - will report the current battery voltage x10 and the current settings
    if (Serial.read() == '\n') {
      if ((amode == mode) && (aspd == spd) && (ared == red) && (agrn == grn) && (ablu == blu) && !showVolts) {
        newData = 0;
      }
      if (!showVolts) {    //it is supported not having to send the whole data format for data requests
        mode = amode;      //therefore this prevents the data to be overwritten by zeros, so the show can go on
        spd = aspd;
        red = ared;
        grn = agrn;
        blu = ablu;
      }

      return newData;
    }
  }
}
//-------------------------------------------------------------------------------------Boot Animation
void bootAnimation(){
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 255, 0);
    strip.show();
    delay(25);
  }
  Serial.println("Ready!");
  for (int i = 255; i >= 0; i--) {
    strip.setBrightness(i); //clears the strip
    strip.show();
    delay(2);
  }
  pinMode(STAT, INPUT);
  for (int j = 0; j < strip.numPixels(); j++) {
    strip.setPixelColor(j, 0, 0, 0); //clears the strip
  }
  strip.setBrightness(255);
}

