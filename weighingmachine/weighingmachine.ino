/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

84x48 individual pixels

 * Pin 1 (VDD) -> +3.3V (rightmost, when facing the display head-on) connected to Arduino digital pin 8.
 * Pin 2 (SCLK)-> Arduino digital pin 3
 * Pin 3 (SI)-> Arduino digital pin 4
 * Pin 4 (D/C)-> Arduino digital pin 6
 * Pin 5 (CS)-> Arduino digital pin 7
 * Pin 6 (GND)-> Ground
 * Pin 7 (Vout)-> 10uF capacitor -> Ground
 * Pin 8 (RESet)-> Arduino digital pin 5
 * 
 *   3      SI   serial data input of LCD
 *   2      SCLK serial clock line of LCD
 *   4      D/C  (or sometimes named A0) command/data switch
 *   8      /RES active low reset
 *   9      Backlight (optional, not on display)
 *   6      GND  Ground for printer port and VDD
 *   1      VDD  +V (? mA) Chip power supply
 *   5      /CS   active low chip select (connected to GND)
 *   7      Vout output of display-internal dc/dc converter
 * 
 * 
 Hardwired D8 for LED Backlight
 Vsupply ---470 ohms---LED----C--E--GND
                                B
                                1k
                                |
                                D8
                                
 D9 - Hx711 data, D10 Hx711 clock
 All switches - connect to ground when pressed
 D2 - "Unit" switch - probably will use it as "Power" switch
 D10 - "Down" switch
 D11- "Set" switch
 D12-"UP" switch.
       3.3V ----PDR--- |^^^^^10k^^^^----GND
                       |
                       |
                       A5
                       
https://github.com/bogde/HX711
How to calibrate your load cell
Call set_scale() with no parameter.
Call tare() with no parameter.
Place a known weight on the scale and call get_units(10).
Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale().
Adjust the parameter in step 4 until you get an accurate reading.

*********************************************************************/


#include "HX711.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//#include <Fonts/FreeMonoBold24pt7b.h> //too big for nokia display even with text size 1
#include <Fonts/FreeSansBold12pt7b.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 6, 7, 5); //Hardwired this way - don't change
const int LED_BACKLIGHT_PIN =  8; //Hardwired via a transistor in order to run on supply voltage rather than on regulated 3.3 V.
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 9; //Hardwired
const int LOADCELL_SCK_PIN = 10; //Hardwired
const int LIGHT_LEVEL_PIN = A5;
const int SWITCH_UNIT_PIN = 2;
const int SWITCH_DOWN_PIN = 11;
const int SWITCH_SET_PIN = 12;
const int SWITCH_UP_PIN = 13;
int light_level =0;
HX711 scale;


void setup()   {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  pinMode(LED_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(LED_BACKLIGHT_PIN, LOW);
  delay(5000);
  digitalWrite(LED_BACKLIGHT_PIN, HIGH);
  display.begin();
  Serial.println("Serial Test");
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(65);

  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer
  digitalWrite(LED_BACKLIGHT_PIN, LOW);
  scale.set_scale(554.f);                      // this value is obtained by calibrating the scale with known weights; see above and the https://github.com/bogde/HX711 README for details
  scale.tare();  
  

}


void loop() {
    light_level = analogRead(LIGHT_LEVEL_PIN);
   if (scale.is_ready()) {

    display.clearDisplay();
    //long reading = scale.read();
      long reading = scale.get_units(10);
    //Serial.print("HX711 reading: ");
    //Serial.println(reading);
    //for (int i=0; i<mynum; i++) {
  //display.setFont(); to reset back to default font
  display.setFont(&FreeSansBold12pt7b);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(0,32);
  display.print(reading);
  display.display();
  
  //display.clearDisplay();
//}
  } else {
  display.setCursor(0,32);
  display.print(light_level);
  display.display();
  
  }
  delay(5000);
}
