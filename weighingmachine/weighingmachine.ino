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

 Pin to interrupt map:
 D0-D7 = PCINT 16-23 = PCIR2 = PD = PCIE2 = pcmsk2
 D8-D13 = PCINT 0-5 = PCIR0 = PB = PCIE0 = pcmsk0
 A0-A5 (D14-D19) = PCINT 8-13 = PCIR1 = PC = PCIE1 = pcmsk1


*********************************************************************/

#include "PinChangeInterrupt.h"
#include "HX711.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <avr/sleep.h>
#include <avr/power.h>
//#include <Fonts/FreeMonoBold24pt7b.h> //too big for nokia display even with text size 1
#include <Fonts/FreeSansBold12pt7b.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 6, 7, 5); //Hardwired this way - don't change
int myVersion = 1;
const int LED_BACKLIGHT_PIN =  8; //Hardwired via a transistor in order to run on supply voltage rather than on regulated 3.3 V.
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 9; //Hardwired
const int LOADCELL_SCK_PIN = 10; //Hardwired
const int LIGHT_LEVEL_PIN = A5;
const int SWITCH_UNIT_PIN = 2; // External interrupt
const int SWITCH_DOWN_PIN = 11; // Pin Change Interrupt Request 0 (pins D8 to D13) (PCINT0_vect)
const int SWITCH_SET_PIN = 12;  // Pin Change Interrupt Request 0 (pins D8 to D13) (PCINT0_vect)
const int SWITCH_UP_PIN = 13;   // Pin Change Interrupt Request 0 (pins D8 to D13) (PCINT0_vect)
int light_level =0;
int cutoffLightLevel = 512;

unsigned long previousMillis = 0;     
const long timeoutinterval = 15000; 

unsigned long DisplaypreviousMillis = 0;
const long DisplayRefreshRate= 100;

volatile boolean UNIT=false;
volatile boolean SET = false;
volatile boolean UP = false;
volatile boolean DOWN = false;

boolean isNavigatingMenu = false;

int contrast=65;

int menuitem = 1;
int page = 1;

int SETbuttonState = 0;
int UPbuttonState = 0;  
int DOWNbuttonState = 0;

int lastSETbuttonState=0;          
int lastUPbuttonState = 0;
int lastDOWNbuttonState = 0;

HX711 scale;

void enterSleep(void)
{
  //attachInterrupt(0, UNITpinInterrupt, LOW);
  attachInterrupt (digitalPinToInterrupt (SWITCH_UNIT_PIN), UNITpinInterrupt, LOW);
  delay(100);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  scale.power_down();
  sleep_enable();
  digitalWrite(LED_BACKLIGHT_PIN, LOW);
  display.clearDisplay();   //write 0 to shadow buffer
  display.display();        // copy buffer to display memory
  display.command( PCD8544_FUNCTIONSET | PCD8544_POWERDOWN);
  UNIT=false;
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
  scale.power_up();
  display.initDisplay();
  previousMillis = millis();
  Serial.println("awake now");
}
//ISR (PCINT2_vect)
// {
//  //Serial.println(PCMSK0,BIN);
// } 

void UNITpinInterrupt(void)
{
  /* This will bring us back from sleep. */
  
  /* We detach the interrupt to stop it from 
   * continuously firing while the interrupt pin
   * is low.
   */
  //detachInterrupt(0);
  detachInterrupt (digitalPinToInterrupt (SWITCH_UNIT_PIN));
  UNIT=true;
  
}
void SETpinInterrupt(void)
{
  SET=true;
  detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_SET_PIN));
  
}
void UPpinInterrupt(void)
{
  UP=true;
  detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_UP_PIN));
  
}
void DOWNpinInterrupt(void)
{
  DOWN=true;
  detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_DOWN_PIN));
  
  
}

void backlightcontrol(){
  light_level = analogRead(LIGHT_LEVEL_PIN);
  if (light_level<cutoffLightLevel){
    digitalWrite(LED_BACKLIGHT_PIN, HIGH);}
    else
    {
      digitalWrite(LED_BACKLIGHT_PIN, LOW);
      }
}


  void handleMenu(){
//  DOWNbuttonState = digitalRead(SWITCH_DOWN_PIN);
//  SETbuttonState = digitalRead(SWITCH_SET_PIN);
//  UPbuttonState =   digitalRead(SWITCH_UP_PIN);
 
//  checkIfDOWNbuttonIsPressed();
//  checkIfUPbuttonIsPressed();
//  checkIfSETbuttonIsPressed();
  //

  if ((UP || DOWN || SET) == true){
    previousMillis = millis();
    }

  if ((UP) && (page == 1 )) {
    UP = false;
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_UP_PIN), UPpinInterrupt, FALLING);
    menuitem--;
    if (menuitem==0)
    {
      menuitem=3;
    }      
  }else if ((UP) && (page == 2 )) {
    UP = false;
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_UP_PIN), UPpinInterrupt, FALLING);
    contrast--;
    setContrast();
  }
  if ((DOWN) && (page == 1)) {
    DOWN = false;
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_DOWN_PIN), DOWNpinInterrupt, FALLING);

    menuitem++;
    if (menuitem==4) 
    {
      menuitem=1;
    }      
  }else if ((DOWN) && (page == 2 )) {
    DOWN = false;
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_DOWN_PIN), DOWNpinInterrupt, FALLING);
    contrast++;
    setContrast();
  }
  if (SET) {
    SET = false;
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_SET_PIN), SETpinInterrupt, FALLING);
    
    if (page == 1 && menuitem==2) 
    {

    }
    if(page == 1 && menuitem ==3)
    {
      resetDefaults();
    }
else if (page == 1 && menuitem==1) {
      page=2;
     }
else if (page == 2) {
      page=1;
     }
   }
   
  }


  void drawMenu()
  {
    
  if (page==1) 
  {    
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print("MAIN MENU");
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(0, 15);
   
    if (menuitem==1) 
    { 
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }
    display.print(">Contrast");
    display.setCursor(0, 25);
   
    if (menuitem==2) 
    {
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }    
    display.print(">Unit: ");
    

    
    if (menuitem==3) 
    { 
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }  
    display.setCursor(0, 35);
    display.print(">Reset");
    display.display();
  }
    
 
  else if (page==2) 
  {
    
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print("CONTRAST");
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(5, 15);
    display.print("Value");
    display.setTextSize(2);
    display.setCursor(5, 25);
    display.print(contrast);
 
    display.setTextSize(2);
    display.display();
  }
  
  }

 void resetDefaults()
  {
    contrast = 65;
    setContrast();
  }
  void setContrast()
  {
    display.setContrast(contrast);
    display.display();
  }
void setup()   {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(554.f);                      // this value is obtained by calibrating the scale with known weights; see above and the https://github.com/bogde/HX711 README for details
  scale.tare(); 
  
  pinMode(LED_BACKLIGHT_PIN, OUTPUT);
  pinMode(SWITCH_UNIT_PIN, INPUT_PULLUP);
  pinMode(SWITCH_SET_PIN, INPUT_PULLUP);
  pinMode(SWITCH_UP_PIN, INPUT_PULLUP);
  pinMode(SWITCH_DOWN_PIN, INPUT_PULLUP);

  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_UP_PIN), UPpinInterrupt, FALLING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_SET_PIN), SETpinInterrupt, FALLING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SWITCH_DOWN_PIN), DOWNpinInterrupt, FALLING);

  
  display.begin();
  display.clearDisplay();
  display.setContrast(contrast);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.print("Ver =");
  display.println(myVersion);
  display.println("POST");
  display.display();
  
  Serial.println("All good in setup");

}


void loop() {
/*   
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
  
  //display.print(light_level);
  display.display();
  
  }
  delay(5000);

  

*/

unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= timeoutinterval) {
     previousMillis = currentMillis;
     isNavigatingMenu=false;
      Serial.println("Entering Sleep");
      enterSleep();
  }
   else{
    backlightcontrol();
//  if (currentMillis - DisplaypreviousMillis >= DisplayRefreshRate) {
//     DisplaypreviousMillis = currentMillis;
//     drawMenu();
//  }
    //weigh();
   // showWeight();
  handleMenu();
    

  drawMenu();




  

 


}
}
