/*
cra-slider-game, 2014-2019 Musework Exhibits, LLC
created for Colorado River Alliance's Mobile River traveling exhibit.
This code runs on an Arduino Mega attached to 6 linear softpots.
This allows a user to estimate their weekly water consumption by changing the slider / softpot
values.

MIT-license
*/

#include <Arduino.h>
#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <SPI.h>            // print to serial for debug. Adafruit_GFX wants it as a dependency
#include <EEPROM.h>         // EEPROM allows us to save and load variables between sessions
#include "EEPROMAnything.h"

//define output pins for RGBmatrix
#define CLK 11 // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A11
#define OE 9
#define A A8
#define B A9
#define C A10
// define pot input
#define POTA A0
#define POTB A1
#define POTC A2
#define POTD A3
#define POTE A4
#define POTF A5

// global variables
int mappedVal = 0;
int hue = 0;
unsigned long countdown = 30000; //countdown time IN MSEC!!!
unsigned long endTime;
unsigned long startTime;

//variables for smoothing output
const int numReadings = 100; 
int readings[numReadings];      // the readings from mappedVal
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average


//calibration variables store raw readouts of softpots
int showerLo;
int showerHi;
int toiletLo;
int toiletHi;
int sinkLo;
int sinkHi;
int dishesLo;
int dishesHi;
int lawnLo;
int lawnHi;
int laundryLo;
int laundryHi;

struct config_t
{
  //calibration variables store raw readouts of softpots
  int showerLo;
  int showerHi;
  int toiletLo;
  int toiletHi;
  int sinkLo;
  int sinkHi;
  int dishesLo;
  int dishesHi;
  int lawnLo;
  int lawnHi;
  int laundryLo;
  int laundryHi;
};

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

void setup()
{
  Serial.begin(9600);
  // load saved calibration values from eeprom
  config_t config; // = {950, 0, 940, 0, 930, 0, 900, 0, 940, 0, 930, 0};
  EEPROM.get(0, config);
  Serial.println("read from EEPROM");
 
  //load values from config struct
  showerLo = config.showerLo;
  showerHi = config.showerHi;
  toiletLo = config.toiletLo;
  toiletHi = config.toiletHi;
  sinkLo = config.sinkLo;
  sinkHi = config.sinkHi;
  dishesLo = config.dishesLo;
  dishesHi = config.dishesHi;
  lawnLo = config.lawnLo;
  lawnHi = config.lawnHi;

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) 
  {
    readings[thisReading] = 0;
  }

  

  //begin calibration code
  
  //set defaults
  showerLo = 500;
  showerHi = 500;
  toiletLo = 500;
  toiletHi = 500;
  sinkLo = 500;
  sinkHi = 500;
  dishesLo = 500;
  dishesHi = 500;
  lawnLo = 500;
  lawnHi = 500;

  startTime = millis();
  endTime = startTime;
  while ((endTime - startTime) <= countdown)
  {
    matrix.begin();
    int showerRaw = (analogRead(POTA));
    if (showerRaw < showerLo)
    {
      showerLo = showerRaw;
      Serial.print("\n showerLo is now:");
      Serial.print(showerLo);
    }
    if (showerRaw > showerHi)
    {
      showerHi = showerRaw;
      Serial.print("\n showerHi is now:");
      Serial.print(showerHi);
    }

    int toiletRaw = (analogRead(POTB));
    if (toiletRaw < toiletLo)
    {
      toiletLo = toiletRaw;
      Serial.print("\n toiletLo is now:");
      Serial.print(toiletLo);
    }
    if (toiletRaw > toiletHi)
    {
      toiletHi = toiletRaw;
      Serial.print("\n toiletHi is now:");
      Serial.print(toiletHi);
    }

    int sinkRaw = (analogRead(POTC));
    if (sinkRaw < sinkLo)
    {
      sinkLo = sinkRaw;
      Serial.print("\n sinkLo is now:");
      Serial.print(sinkLo);
    }
    if (sinkRaw > sinkHi)
    {
      sinkHi = sinkRaw;
      Serial.print("\n sinkHi is now:");
      Serial.print(sinkHi);
    }

    int dishesRaw = (analogRead(POTF));
    if (dishesRaw < dishesLo)
    {
      dishesLo = dishesRaw;
      Serial.print("\n dishesLo is now:");
      Serial.print(dishesLo);
  
    }
    if (dishesRaw > dishesHi)
    {
      dishesHi = dishesRaw;
      Serial.print("\n dishesHi is now:");
      Serial.print(dishesHi);
     
    }

    int laundryRaw = (analogRead(POTE));
    if (laundryRaw < laundryLo)
    {
      laundryLo = laundryRaw;
      Serial.print("\n laundryLo is now:");
      Serial.print(laundryLo);
      
    }
    if (laundryRaw > laundryHi)
    {
      laundryHi = laundryRaw;
      Serial.print("\n laundryHi is now:");
      Serial.print(laundryHi);
     
    }

    int lawnRaw = (analogRead(POTD));
    if (lawnRaw < lawnLo)
    {
      lawnLo = lawnRaw;
      Serial.print("\n lawnLo is now:");
      Serial.print(lawnLo);
      
    }
    if (lawnRaw > lawnHi)
    {
      lawnHi = lawnRaw;
      Serial.print("\n lawnHi is now:");
      Serial.print(lawnHi);
    
    }

    endTime = millis();
    int seconds = (endTime - startTime) / 1000;
    mappedVal = 30 - seconds;
    
    
    char strIn[3];
    String tempStr;

    tempStr = String(mappedVal);   //convert int to string
    tempStr.toCharArray(strIn, 4); // pass values to char array

    char digit1 = strIn[0];
    char digit2 = strIn[1];
    char digit3 = strIn[2];

    //if(abs(mappedVal-curVal > 3)
    // fill the screen with 'black'
    matrix.fillScreen(matrix.Color333(0, 0, 0));

    // draw some text!

    matrix.setTextSize(1); // size 1 == 8 pixels high
    matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
    // matrix.setCursor(0,8);
    // matrix.print(mappedVal);
    //matrix.print(digit1);
    // matrix.print(digit2);
    // matrix.print(digit3);

    //matrix.fillScreen(matrix.Color333(0, 0, 0));
    //delay (100);
    if (mappedVal < 10)
    {
      matrix.setCursor(22, 0); //set right digit
      matrix.print(digit1);

      //matrix.setCursor(18, 0); //set mid digit
      //matrix.print(0);

      //matrix.setCursor(0, 0); //set left digit
      //matrix.print(0);
    }

    else if (mappedVal > 9 && mappedVal < 100)
    {

      matrix.setCursor(22, 0); //set right digit
      matrix.print(digit2);

      matrix.setCursor(16, 0); //set mid digit
      matrix.print(digit1);

      //matrix.setCursor(0, 0); //set left digit
      //matrix.print(0);
    }
    else
    {
      matrix.setCursor(22, 0); //set right digit
      matrix.print(digit3);

      matrix.setCursor(16, 0); //set mid digit
      matrix.print(digit2);

      matrix.setCursor(10, 0); //set left digit
      matrix.print(digit1);

      //curVal = mappedVal;
      //matrix.swapBuffers(true);
      //delay(1000);
    }
    //delay(250);

    matrix.setCursor(1, 9);
    matrix.print("calibrate");

    matrix.swapBuffers(false);
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    matrix.swapBuffers(false);
    
  }

  //write to config
  config = {showerLo, showerHi, toiletLo, toiletHi, sinkLo, sinkHi, dishesLo, dishesHi, laundryLo, laundryHi, lawnLo, lawnHi};
  EEPROM.put(0, config);
  Serial.print("writing config to EEPROM");

}

void loop()
{

  matrix.begin();

  int showerRaw = (analogRead(POTA));
  int toiletRaw = (analogRead(POTB));
  int sinkRaw = (analogRead(POTC));
  int dishesRaw = (analogRead(POTF));
  int laundryRaw = (analogRead(POTE));
  int lawnRaw = (analogRead(POTD));

  int mappedShower = map(showerRaw, showerHi, showerLo, 0, 41 );
  int mappedToilet = map(toiletRaw, toiletHi, toiletLo, 0, 51 );
  int mappedSink = map(sinkRaw, sinkHi, sinkLo , 0 , 41);
  int mappedDishes = map(dishesRaw, dishesHi, dishesLo , 0 , 8);
  int mappedLaundry = map(laundryRaw, laundryHi, laundryLo, 0 , 29);
  int mappedLawn = map(lawnRaw, lawnHi,lawnLo, 0 , 900);

  int rawMapped = (mappedShower + mappedToilet + mappedSink + mappedDishes + mappedLaundry + mappedLawn);
  

  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = rawMapped;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  mappedVal = total / numReadings;

  
  


  hue = map(mappedVal, 0, 100, 400, 0);
  if (hue < 50)
  {
    hue = 50;
  }

  //read analog in

  //convert int into indivs
  char strIn[3];
  String tempStr;

  tempStr = String(mappedVal);   //convert int to string
  tempStr.toCharArray(strIn, 4); // pass values to char array

  char digit1 = strIn[0];
  char digit2 = strIn[1];
  char digit3 = strIn[2];

  //if(abs(mappedVal-curVal > 3)
  // fill the screen with 'black'
  matrix.fillScreen(matrix.Color333(0, 0, 0));

  // draw some text!

  matrix.setTextSize(1); // size 1 == 8 pixels high
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  // matrix.setCursor(0,8);
  // matrix.print(mappedVal);
  //matrix.print(digit1);
  // matrix.print(digit2);
  // matrix.print(digit3);

  //matrix.fillScreen(matrix.Color333(0, 0, 0));
  //delay (100);
  if (mappedVal < 10)
  {
    matrix.setCursor(22, 0); //set right digit
    matrix.print(digit1);

    //matrix.setCursor(18, 0); //set mid digit
    //matrix.print(0);

    //matrix.setCursor(0, 0); //set left digit
    //matrix.print(0);
  }

  else if (mappedVal > 9 && mappedVal < 100)
  {

    matrix.setCursor(22, 0); //set right digit
    matrix.print(digit2);

    matrix.setCursor(16, 0); //set mid digit
    matrix.print(digit1);

    //matrix.setCursor(0, 0); //set left digit
    //matrix.print(0);
  }
  else
  {
    matrix.setCursor(22, 0); //set right digit
    matrix.print(digit3);

    matrix.setCursor(16, 0); //set mid digit
    matrix.print(digit2);

    matrix.setCursor(10, 0); //set left digit
    matrix.print(digit1);

    //curVal = mappedVal;
    //matrix.swapBuffers(true);
    //delay(1000);
  }
  //delay(250);

  matrix.setCursor(10, 9);
  matrix.print("Gal");

  matrix.swapBuffers(false);
  //curVal=mappedVal;
  //delay(1);
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.swapBuffers(false);
  //delay(1);
}
