#include <Arduino.h>
#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <SPI.h>

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

  mappedVal = (shower);
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
