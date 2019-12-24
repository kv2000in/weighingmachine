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

 * Pin 1 (VDD) -> +3.3V (rightmost, when facing the display head-on)
 * Pin 2 (SCLK)-> Arduino digital pin 3
 * Pin 3 (SI)-> Arduino digital pin 4
 * Pin 4 (D/C)-> Arduino digital pin 5
 * Pin 5 (CS)-> Arduino digital pin 7
 * Pin 6 (GND)-> Ground
 * Pin 7 (Vout)-> 10uF capacitor -> Ground
 * Pin 8 (RESet)-> Arduino digital pin 6
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
*********************************************************************/
#include "HX711.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//#include <Fonts/FreeMonoBold24pt7b.h> //too big for nokia display even with text size 1
#include <Fonts/FreeSansBold12pt7b.h>
// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
//Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 7, 6);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 9;
const int LOADCELL_SCK_PIN = 10;

HX711 scale;


void setup()   {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(65);

  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer



}


void loop() {

   if (scale.is_ready()) {

    display.clearDisplay();
    long reading = scale.read();
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
  display.print("XXX");
  display.display();
  }
  delay(2000);
}
