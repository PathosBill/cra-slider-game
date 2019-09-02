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

//define button
#define BUTTON 31

#define NUM_READS 100

// global variables
int mappedVal = 0;
int hue = 0;
unsigned long countdown = 30000; //countdown time IN MSEC!!!
unsigned long endTime;
unsigned long startTime;

//button variables
int buttonState =HIGH;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

//variables for smoothing output
const int numReadings = 200; 
long readings[numReadings];      // the readings from mappedVal
int readIndex = 0;              // the index of the current reading
long total = 0;                  // the running total
long average = 0;                // the average
int prevLawn = 500;


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

//init
config_t config;

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

void calibrate();
float reduceNoise(int mappedValue);

void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON, INPUT_PULLUP);

  // load saved calibration values from eeprom
   // = {950, 0, 940, 0, 930, 0, 900, 0, 940, 0, 930, 0};
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

  //inititalize prev laundry
}

void loop()
{
  matrix.begin();

  // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTON);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // trigger calibration
      calibrate();
      
      }
    }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = LOW;
  

 
  



  int showerRaw = (analogRead(POTA));
  int toiletRaw = (analogRead(POTB));
  int sinkRaw = (analogRead(POTC));
  int dishesRaw = (analogRead(POTF));
  int laundryRaw = (analogRead(POTE));
  int lawnRaw = (analogRead(POTD));

  //mapped values overshoot actual range (0,40) to allow the conditionals to
  //"lock down" the values
  int mappedShower = map(showerRaw, showerHi , (showerLo + 50)  , 0, 40 ); 
  if (mappedShower < 0)
  {
    mappedShower = 0;
  }
  if (mappedShower > 40)
  {
    mappedShower = 40;
  }
  
  int mappedToilet = map(toiletRaw, toiletHi , toiletLo, -2, 52 );
  if (mappedToilet < 0)
  {
    mappedToilet = 0;
  }
  if (mappedToilet > 50)
  {
    mappedToilet = 50;
  }
  
  int mappedSink = map(sinkRaw, sinkHi , sinkLo , -2 , 42);
  if (mappedSink < 0)
  {
    mappedSink = 0;
  }
  if (mappedSink > 40)
  {
    mappedSink = 40;
  }
  
  int mappedDishes = map(dishesRaw, dishesHi, dishesLo  , -2  , 9);
  if (mappedDishes < 0)
  {
    mappedDishes = 0;
  }
  if (mappedDishes > 8)
  {
    mappedDishes = 8;
  }
  
  int mappedLaundry = map(laundryRaw, laundryHi , laundryLo , -2 , 35);
  if (mappedLaundry < 0)
  {
    mappedLaundry = 0;
  }
  if (mappedLaundry > 30)
  {
    mappedLaundry = 30;
  }

  
  
  int mappedLawn = map(lawnRaw, lawnHi , lawnLo , -10 , 1000);
  
  // b/c the range of the lawn slider is so great, the input appears
  // noisier than the others, with ranges jumping up and down by about
  // 1-5. This statement only changes the value for mappedLawn if
  // it's over a threshold of 5.

  if (abs(mappedLawn - prevLawn) > 5)
  {
    prevLawn = mappedLawn;
  } else
  {
    mappedLawn = prevLawn;
  }
  
  
  

  if (mappedLawn < 10)
  {
    mappedLawn = 0;
  }
  if (mappedLawn > 850)
  {
    mappedLawn = 900;
  }
  
  

  int rawMapped = (mappedShower + mappedToilet + mappedSink + mappedDishes + mappedLaundry + mappedLawn);
  
  mappedVal = rawMapped;
  
  if (mappedVal < 0)
  {
    mappedVal = 0;
  }

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

void calibrate()
{
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

float reduceNoise(long mappedValue)
{
    // read multiple values and sort them to take the mode
    long sortedValues[NUM_READS];
    for (int i = 0; i < NUM_READS; i++)
    {
        long value = mappedValue;
        int j;
        if (value < sortedValues[0] || i == 0)
        {
            j = 0; //insert at first position
        }
        else
        {
            for (j = 1; j < i; j++)
            {
                if (sortedValues[j - 1] <= value && sortedValues[j] >= value)
                {
                    // j is insert position
                    break;
                }
            }
        }
        for (int k = i; k > j; k--)
        {
            // move all values higher than current reading up one position
            sortedValues[k] = sortedValues[k - 1];
        }
        sortedValues[j] = value; //insert current reading
    }
    //return scaled mode of 10 values
    float returnval = 0;
    for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++)
    {
        returnval += sortedValues[i];
    }
    returnval = returnval / 10;
    return returnval * 1100 / 1023;
}