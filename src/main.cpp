#include <Arduino.h>
#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <SPI.h>

#define CLK 11 // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A11
#define OE 9
#define A A8
#define B A9
#define C A10
#define POTA A0
#define POTB A1
#define POTC A2
#define POTD A3
#define POTE A4
#define POTF A5
#define NUM_READS 1000

int val = 0;

int mappedVal = 0;
int curVal = 0;
int avgVal = 0;
int hue = 0;
float temperatureRead = 0.0;

int smoothShower = 0;
int smoothToilet = 0;
int smoothWater = 0;
int smoothDishes = 0;
int smoothLaundry = 0;
int smoothLawn = 0;

float reduceNoise(int mappedValue)
{
  // read multiple values and sort them to take the mode
  int sortedValues[NUM_READS];
  for (int i = 0; i < NUM_READS; i++)
  {
    int value = mappedValue;
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

int smooth(int data, float filterVal, float smoothedVal)
{

  if (filterVal > 1)
  { // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0)
  {
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal * filterVal);

  return (int)smoothedVal;
}

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

void setup()
{
}

void loop()
{
  // read analog values

  matrix.begin();

  int shower = (analogRead(POTA));
  int toilet = (analogRead(POTB));
  int water = (analogRead(POTC));
  int dishes = (analogRead(POTF));
  int laundry = (analogRead(POTE));
  int lawn = (analogRead(POTD));

  smoothShower = smooth(shower, .9, smoothShower);
  smoothToilet = smooth(toilet, .9, smoothToilet);
  smoothWater = smooth(water, .9, smoothWater);
  smoothDishes = smooth(dishes, .9, smoothDishes);
  smoothLaundry = smooth(laundry, .9, smoothLaundry);
  smoothLawn = smooth(lawn, .9, smoothLawn);

  int mappedShower = (map(shower, 50, 950, 40, 0));

  if (mappedShower >= 43)
  {
    mappedShower = 43;
  }
  else if (mappedShower < 4)
  {
    mappedShower = 0;
  }

  int mappedToilet = (map(smoothToilet, 118, 940, 50, 0));

  int mappedWater = (map(smoothWater, 540, 930, 40, 0));

  int mappedDishes = (map(smoothDishes, 69, 949, 8, 0));

  if (mappedDishes < 2)
  {
    mappedDishes = 0;
  }
  else if (mappedDishes > 8)
  {
    mappedDishes = 8;
  }

  int mappedLaundry = (map(smoothLaundry, 80, 800, 900, 0));

  if (mappedLaundry < 10)
  {
    mappedLaundry = 0;
  }
  else if (mappedLaundry > 900)
  {
    mappedLaundry = 900;
  }
  int mappedLawn = (map(smoothLawn, 51, 786, 28, 0));
  if (mappedLawn < 2)
  {
    mappedLawn = 0;
  }
  else if (mappedLawn > 28)
  {
    mappedLawn = 28;
  }

  mappedVal = (mappedShower + (mappedToilet * 0.16) + mappedWater + mappedDishes + mappedLaundry + mappedLawn);
  //mappedVal= mappedWater;
  // mappedVal= water;

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
