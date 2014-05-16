  /* routerLift
  An Arduino program to moniter and display the height and anlge of a tilting
  router lift using a rotary encoder and softpot as inputs. This code is
  untested. 
  
  Written by John Segner, May 16 2014 
  
  
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD WR pin to digital pin 10
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 9
 * LCD D5 pin to digital pin 8
 * LCD D6 pin to digital pin 7
 * LCD D7 pin to digital pin 6
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * Encoder Channel A to digital pin 2
 * Encoder Channel B to digital pin 3
 * Zero button on pin 5
 * Enter button on pin 4
 */

#include <LiquidCrystal.h>
#include <Bounce.h>
#include <stdlib.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

//#define TILT //Comment out this line if you are not using a tilt softpot


#define ANGLE A4
#define ZERO 5
#define ENTER 4
#define INTERVAL 100

//encoder variables
int encoderPin1 = 2;
int encoderPin2 = 3;
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;

//Button object
Bounce zero = Bounce(ZERO, 10); //(pin number, debounce time)
Bounce enter = Bounce(ENTER, 10); 

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 10, 11, 9, 8, 7, 6);
     // LCD pins: RS  RW  EN  D4 D5 D6 D7
    byte degree[8] = { //Custom character
    B11100,
    B10100,
    B11100,
    B00000,
    B00000,
    B00000,
    B00000
  };
  static char dtostrfbuffer[7]; //for displaying floats
  
  //configuation data structure
  
  struct settings_t
{
  int flag;
  long enMax;
  long enMin;
  long anMax;
  long anMin;
} settings;
  
 
  
  //main variables
  int mode;
  int phase;
  float height;
  float angle;
  int rawAngle;
  int angleMax;
  int angleMin;
  long rawHeight;
  long encoderMax;
  long encoderMin;
  long nextTime;

////////////////////////////
//SETUP                   //
////////////////////////////

void setup(){
  
//Setup Encoder//
  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
 
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE); 

//Setup LCD
 // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2); //argument define lcd size (characters, lines)
  lcd.createChar(0, degree); //upload custom character to posistion 0
  

  //Setup Buttons
  pinMode(ZERO, INPUT);
  digitalWrite(ZERO, HIGH);
  pinMode(ENTER, INPUT);
  digitalWrite(ENTER, HIGH);
  
  #ifdef TILT
  //Setup Angle SoftPot
  pinMode(ANGLE, INPUT);
  digitalWrite(ENTER, HIGH);
  #endif
  
  //Load Calibration Values
  EEPROM_readAnything(0, settings);  
  if (settings.flag == 0xFF)   //brand new eeprom?
       {
        mode = 1; //it's config time!
        phase = 0;
      }
else
  {
  settings.enMax = encoderMax;
  settings.enMin = encoderMin;
  #ifdef TILT
  settings.anMax = angleMax;
  settings.anMin = angleMin;
  Msg(lcd, "Height: ", "Angle: ", 250);
  #else
  Msg(lcd, "Height: ", "", 250);
  #endif
  }

}

////////////////////////////
//MAIN LOOP               //
////////////////////////////

void loop(){
  
  updateButtons();
  
  switch(mode){
    case 0:  //
      if ((millis() + INTERVAL) > nextTime){ //Set LCD refresh rate (10hz for INTERVAL = 100)
         updateLCD();
         nextTime = millis() + INTERVAL; 
      }
      break;
     case 1: //Calibration routine
       switch(phase){
         case 0: //Show 1in height instructions
           Msg(lcd, "Set bit to 1in", "and press enter.", 0);
           phase++;
         case 1: //Accept max height input
           if(enter.fallingEdge()){
           encoderMax = encoderValue;
           settings.enMax = encoderMax; 
           phase++;
           break;
           }
           break;
         case 2: //Show zeroing instruction
           Msg(lcd, "Zero your bit", "to the table.", 0);
           phase++;
         case 3: //Accept zero input
           if(zero.fallingEdge()){
           encoderMin = encoderValue;
           settings.enMin = encoderMin; 
           phase++;
           break;
           }
           break;
         case 4: //Show 0 angle instuction
           #ifndef TILT
           phase = 8;
           #else
           Msg(lcd, "Set angle to 0", "and press enter.", 0);
           phase++;
           #endif
         case 5: //Accept 0 input
           if(enter.fallingEdge()){
           angleMin = analogRead(ANGLE);
           settings.anMin = angleMin; 
           phase++;
           break;
           }
           break;
         case 6: //Show 45 degree instuction
           Msg(lcd, "Set angle to 45", "and press enter.", 0);
           phase++;
         case 7: //Accept 45 degree input
           if(enter.fallingEdge()){
           angleMax = analogRead(ANGLE);
           settings.anMax = angleMax; 
           phase++;
           break;
           }
           break;
         case 8: //reset
           EEPROM_writeAnything(0, settings);
           Msg(lcd, "Calibration", "complete!", 2000);
           mode = 0;
           phase = 0;
         
       }
  
  
  }

}

/////////////////////////////
//MESSAGE DISPLAY FUNCTIONS//
/////////////////////////////

/* A helper function to display messages of a specified duration */
//void Msg(LiquidCrystal &lcd, const char *top, const char *bottom, unsigned long del)
void Msg(LiquidCrystal &lcd, const char *top, const char *bottom, unsigned long del)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(top);
  Serial.println(top);
  lcd.setCursor(0, 1);
  lcd.print(bottom);
  Serial.println(bottom);
  delay(del);
}

void UpdateMsg(LiquidCrystal &lcd, float height, float angle, unsigned long del){
        dtostrf(height, 5, 3, dtostrfbuffer); //format float to print
        lcd.setCursor(8, 0);
        lcd.print(dtostrfbuffer);
        lcd.print(" in");
        #ifdef TILT
        dtostrf(angle, 5, 2, dtostrfbuffer); //format float to print
        lcd.setCursor(8, 1);
        lcd.print(dtostrfbuffer);
        lcd.write(byte(0));
        #endif
}

void updateLCD(){
  #ifdef TILT
  rawAngle = analogRead(ANGLE);
  rawAngle = map(rawAngle, angleMin, angleMax, 0, 4500);
  angle = (float)rawAngle / 100.00;
  #endif
  rawHeight = map(encoderValue, encoderMin, encoderMax, 0, 20000);
  height = (float)rawHeight/10000.00;
  UpdateMsg(lcd, height, angle, 0);
  }
////////////////////////////
//BUTTON HANDLERS         //
////////////////////////////

void updateButtons(){
  zero.update();
  enter.update();
  if((enter.read() == LOW) && (enter.duration() > 1000)){ //press and hold handler
    mode = 1;
    phase = 0;
  }
 }


////////////////////////////
//ROTARY ENCODER FUNCTION//
////////////////////////////

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit
 
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
 
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
 
  lastEncoded = encoded; //store this value for next time
}

