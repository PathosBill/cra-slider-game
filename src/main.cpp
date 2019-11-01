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
#define POTF A3
#define POTD A4
#define POTE A5

//define button
#define BUTTON 31

//define number of reads for reduceNoise function
#define NUM_READS 10

// global variables
int mappedVal = 0;
int hue = 0;

//calibration variables
unsigned long countdown = 30000; //countdown time IN MSEC!!!
unsigned long endTime;
unsigned long startTime;

//button variables
//button is debounced manually, no debounce library used
int buttonState =LOW;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 100;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

//reduceNoise variables
const int numReadings = 200; 
long readings[numReadings];      // the readings from mappedVal
int readIndex = 0;              // the index of the current reading
long total = 0;                  // the running total
long average = 0;                // the average
int prevLawn = 500;


//calibration variables store low and high points of softpots
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


// we store and retrieve calibration variables on the Arduino's EEPROM
// using a struct so that we can read and write from address 0 w/o
// counting addresses
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

//init the struct
config_t config;

//init the RGBPanel matrix
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

//declare calibrate and reduceNoise functions
void calibrate();
int reduceNoise(int mappedValue);

void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON, INPUT_PULLUP);

  // load config struct from eeprom
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
  laundryLo = config.laundryLo;
  laundryHi = config.laundryHi;

  // initialize all the  reduceNoise readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) 
  {
    readings[thisReading] = 0;
  }

  
}

void loop()
{

  matrix.begin();  // b/c matrix is soldered to pins, there's no i2c address

  // button logic:
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

    

      // trigger calibration function
      calibrate();
      
      
    }
  // set lastButtonState to low to allow retriggering of calibration function
  lastButtonState = LOW;
  

  //read  raw values of sliders 
  int showerRaw = (analogRead(POTA));
  int toiletRaw = (analogRead(POTB));
  int sinkRaw = (analogRead(POTC));
  int dishesRaw = (analogRead(POTD));
  int laundryRaw = (analogRead(POTF));
  int lawnRaw = (analogRead(POTE));

  // map raw analog input to range.
  int mappedShower = map(showerRaw, showerHi , showerLo , 0, 40 ); 
  //run reduceNoise function on mapped input
  int smoothShower;
  smoothShower = reduceNoise(mappedShower);
  
  // set bounds for input. Forces proper min / max values
  if (smoothShower <= 0)
  {
    smoothShower = 0;
  }
  if (smoothShower >= 38)
  {
    smoothShower = 40;
  }
  
  

  
  int mappedToilet = map(toiletRaw, toiletHi , toiletLo, 0, 52 );
  int smoothToilet;
  smoothToilet =reduceNoise(mappedToilet);

  if (smoothToilet <= 3)
  {
    smoothToilet = 0;
  }
  if (smoothToilet >= 50)
  {
    smoothToilet = 50;
  }
 
  
  int mappedSink = map(sinkRaw, sinkHi , sinkLo , 0 , 40);
  int smoothSink;
  smoothSink = reduceNoise(mappedSink);

  if (smoothSink <= 1)
  {
    smoothSink = 0;
  }
  if (smoothSink >= 40)
  {
    smoothSink = 40;
  }
  
  // because mappedDishes is on such a small scale, we set max to 9
  // instead of 8 to increase the area of the slider that reports max
  int mappedDishes = map(dishesRaw, dishesHi, dishesLo  , 0  , 9);
  int smoothDishes;
  smoothDishes = reduceNoise(mappedDishes);

  //same here, we count a value of 1 as 0 in order to improve feel
  // it means there's a wider range that reports 0 on the slider
  if (smoothDishes <= 01)
  {
    smoothDishes = 0;
  }
  if (smoothDishes >= 8)
  {
    smoothDishes = 8;
  }
  

  
  int mappedLaundry = map(laundryRaw, laundryHi , laundryLo , 0 , 30);
  int smoothLaundry;
  smoothLaundry = reduceNoise(mappedLaundry);
  
  if (smoothLaundry <= 0)
  {
    smoothLaundry = 0;
  }
  if (smoothLaundry >= 30)
  {
    smoothLaundry = 30;
  }



  
  
  int mappedLawn = map(lawnRaw, lawnHi  , lawnLo , 0  , 950);
  
  // b/c the range of the lawn slider is so great, the input appears
  // noisier than the others, with ranges jumping up and down by about
  // 1-5. This statement only changes the value for mappedLawn if
  // it's over a threshold of 5.
  
  /*if (abs(mappedLawn - prevLawn) > 5)
  {
    prevLawn = mappedLawn;
  } else
  {
    mappedLawn = prevLawn;
  }
  */

  int smoothLawn;
  smoothLawn = reduceNoise(mappedLawn);

  if (smoothLawn <= 10)
  {
    smoothLawn = 0;
  }
  if (smoothLawn >= 950)
  {
    smoothLawn = 950;
  }

 
  // this is the actual constructor inc. smoothSink
  int smoothMapped = (smoothShower + smoothToilet + smoothSink + smoothDishes + smoothLaundry + smoothLawn);
  
  
  //temp remove sink b/c broken pot
 //int smoothMapped = (smoothShower + smoothToilet  + smoothDishes + smoothLaundry + smoothLawn);
  
  mappedVal = smoothMapped;

  //enforce zero floor
  if (mappedVal <= 1)
  {
    mappedVal = 0;
  }

  //enforce 999 ceiling
  if (mappedVal <= 999)
  {
    mappedVal = 999;
  }

  /*
  //diagnostics - uncomment to print individual vars to serial monitor
  Serial.print("smooth shower: ");
  Serial.print(smoothShower);
  Serial.print("\n");
  Serial.print("smooth toilet: ");
  Serial.print(smoothToilet);
  Serial.print("\n");
  Serial.print("smooth sink: ");
  Serial.print(smoothSink);
  Serial.print("\n");
  Serial.print("smooth dishes: ");
  Serial.print(smoothDishes);
  Serial.print("\n");
  Serial.print("smooth laundry: ");
  Serial.print(smoothLaundry);
  Serial.print("\n");
  Serial.print("smooth lawn: ");
  Serial.print(smoothLawn);
  //Serial.print("\n");
  // Serial.print("\n");
  //Serial.print("mapped lawn: ");
  //Serial.print(mappedLawn);
  Serial.print("\n");
  Serial.print("raw lawn: ");
  Serial.print(lawnRaw);
  */
  

  // colorize based on water usage.
  hue = map(mappedVal, 0, 100, 400, 0);
  if (hue < 50)
  {
    hue = 50;
  }

  //LED matrix display code

  //convert int into indivs
  char strIn[3];
  String tempStr;

  tempStr = String(mappedVal);   //convert int to string
  tempStr.toCharArray(strIn, 4); // pass values to char array

  char digit1 = strIn[0];
  char digit2 = strIn[1];
  char digit3 = strIn[2];

  // fill the screen with 'black'
  matrix.fillScreen(matrix.Color333(0, 0, 0));

  // setup text
  matrix.setTextSize(1); // size 1 == 8 pixels high
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  

  //in order to properly format the digits, we need
  // to be able draw from right to left.
  // this code splits the incoming int into
  // up to 3 individual chars that are then
  // manually positioned to create proper formatting

  // if mappedVal < 10, then only right digit used
  if (mappedVal < 10)
  {
    matrix.setCursor(22, 0); //set right digit
    matrix.print(digit1);
  }

  // if mappedVal is between 10 - 99, use right and middle digit
  else if (mappedVal > 9 && mappedVal < 100)
  {

    matrix.setCursor(22, 0); //set right digit
    matrix.print(digit2);

    matrix.setCursor(16, 0); //set mid digit
    matrix.print(digit1);
  }
  // all other cases get 3 digits, inc. left
  else
  {
    matrix.setCursor(22, 0); //set right digit
    matrix.print(digit3);

    matrix.setCursor(16, 0); //set mid digit
    matrix.print(digit2);

    matrix.setCursor(10, 0); //set left digit
    matrix.print(digit1);
  }
 
  //print gal label on bottom row
  matrix.setCursor(10, 9);
  matrix.print("Gal");
  
  matrix.swapBuffers(false);
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.swapBuffers(false);
}

void calibrate()
{
   //begin calibration code
  
  //set defaults
  showerLo = 100;
  showerHi = 800;
  toiletLo = 100;
  toiletHi = 800;
  sinkLo = 100;
  sinkHi = 800;
  dishesLo = 100;
  dishesHi = 800;
  lawnLo = 50;
  lawnHi = 829;
  laundryLo = 60;
  laundryHi = 800;

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

    //

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

int reduceNoise(int mappedValue)
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