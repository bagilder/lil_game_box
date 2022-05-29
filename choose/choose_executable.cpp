/* choose your own executable 
bgilder 2022 mar 16
read structs from rom and display text on an LCD. listen for buttons. profit */

/* 
rev 0 initial commit
*/

/*
a story-agnostic UI routine for interpreting state machine data stored in rom

declare i/o.
initialize lcd.
pull title card data from rom and push it to lcd.
listen for a button.

ask for a struct by int index.
pull strings from rom, push them to lcd.
listen for buttons. single button continues until narrative screens are exhausted and choices are reached.
(if there are multiple narrative screens, animate a "continue" arrow or ellipses or something).
push choice strings to lcd.
encoder changes selection options.
animate selection highlighting.
button confirms choice.
choice's corresponding next-state int tells which array index to pull strings from next.

maybe eventually play some music?
*/

#include <SPI.h>
#include <avr/pgmspace.h>
#include <Strings.h>
#include <SSD1322_for_Adafruit_GFX.h>

#include <choose_dummy_struct_array.h>  //this will eventually be our text file struct array



#define SCREEN_WIDTH 256  // in pixels
#define SCREEN_HEIGHT 64  // in pixels
#define GRAY_BLACK 0x0
#define GRAY_1 0x1
#define GRAY_2 0x2
#define GRAY_3 0x3
#define GRAY_4 0x4
#define GRAY_5 0x5
#define GRAY_6 0x6
#define GRAY_7 0x8
#define GRAY_8 0xC
#define GRAY_WHITE 0xF

// use hardware SPI
#define OLED_DC     7
#define OLED_CS     9
#define OLED_RESET  8
Adafruit_SSD1322 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);

bool donezo = false;  //temporary sound testing flag

unsigned long currentTime  = 0;  //for sound //millis
unsigned long stopTime = 1;   //for sound //millis
unsigned int noise = 0;     //for sound

#define buttPin 2
#define encButtPin 3   //may not be implemented. pushbutton on the encoder
#define encApin 4
#define encBpin 5
#define soundPin 19       //for sound

volatile bool bButtFlag = false;
volatile bool bEncButtFlag = false;
volatile bool bCWflag = false;
volatile bool bCCflag = false;
volatile bool bResetFlag = false;
#define lResetStall 10000;  //ms. how long to wait before the reset button does something
unsigned long lResetTimer = 0;


void butt_isr()
{  bButtFlag = true;
}

void enc_butt_isr()
{  bEncButtFlag = true;
}

void enc_a_isr()
{
  if(digitalRead(encBpin) == digitalRead(encApin))
  {
    bCWflag = false;
    bCCflag = true;

    /////////
    //program specific logic for CC rotation

    /////////
  }
  else
  {
    bCWflag = true;
    bCCflag = false;
    
    /////////
    //program specific logic for CW rotation

    /////////
  }
}


void setup()   
{
//  Serial.begin(9600);
  
  pinMode(buttPin, INPUT_PULLUP);
  pinMode(encButtPin, INPUT_PULLUP);
  pinMode(encApin, INPUT_PULLUP);
  pinMode(encBpin, INPUT_PULLUP);
  attachInterrupt(buttPin, butt_isr, FALLING);
  attachInterrupt(encButtPin, enc_butt_isr, FALLING);
  attachInterrupt(encApin, enc_a_isr, CHANGE);
  //attachInterrupt(encBpin, enc_b_isr, CHANGE);


  if ( ! display.begin(0x3D) ) 
  {
     Serial.print("Unable to initialize OLED");
     while (1) yield();
  }
  display.cp437(true);    //proper extended character set
  display.setRotation(0);   //landscape
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(SSD1322_WHITE);
  display.setCursor(0,0);


}


void music(unsigned int song[], unsigned int melodyLength)
{
  //plays music hopefully
  //tone() plays a square wave in the form tone(pin, unsigned int hertz, unsigned long duration - optional)
  
  currentTime = millis();
  if (currentTime >= stopTime)
  {
    noTone(soundPin);
    if(noise+1 < melodyLength)
    {
      stopTime = currentTime + (unsigned long)song[noise+1];
      if(song[noise])
      {   tone(soundPin,song[noise]);
      display.print(song[noise]);
      }
      else
      {   noTone(soundPin);
      }
      noise += 2;
    }
    else
    { 
      display.print("done");
      donezo = true;
    }
    display.display();
  }
}

void drawArt(char xPos, char yPos, char xWidth, char yHeight, char *imageName)  
{
  //displays a pretty picture

  int  counter = 0;
    for (int y = 0; y < yHeight; y++) 
    {
      for (int x = 0; x < xWidth; x++) 
      {
          display.drawPixel(xPos+x,yPos+y,pgm_read_word_near(imageName + counter));
          counter++;
      }
    }
  display.display();
}

void loop()
{
  if(!donezo)
    music(sounds[0],soundLengths[0]);
}