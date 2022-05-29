///////////////////////////
// stacker for er-oledm032
// bgilder 27 may 2022
///////////////////////////

#include <SPI.h>
#include <SSD1322_for_Adafruit_GFX.h>
//#include <Bounce2.h>

#define SCREEN_WIDTH 256 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
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


#define soundPin 19       //for sound
#define buttPin 2
#define encButtPin 3   //may not be implemented. pushbutton on the encoder
//#define encApin 4
//#define encBpin 5
//const char inputPins[] = 
//  { buttPin, encoderPinA, encoderPinB, encoder buttPin};

#define nBlockWidth 8   //must be even factor of screen width
#define nBlockHeight 8  //must be even factor of screen height

volatile bool bButtFlag = false;
volatile bool bEncButtFlag = false;
//volatile bool bCWflag = false;
//volatile bool bCCflag = false;
volatile bool bResetFlag = false;
#define lResetStall 10000;  //ms. how long to wait before the reset button does something
unsigned long lResetTimer = 0;



#define fDelayShift 0.89    //scaler for each subsequent row's jiggle speed
#define fSpeedDelayInit (SCREEN_WIDTH/nBlockHeight - 1)*30  //how fast the rows jiggle at the beginning. scales with screen size
uint16_t lSpeedDelay = fSpeedDelayInit;
uint8_t nCurrentRow = SCREEN_WIDTH/nBlockHeight - 1;
int8_t xPos = 0;  //signed so we can go out of bounds left
int8_t xPosPrev = 0;
#define nBlockCountInit 4
uint8_t nBlockCount = nBlockCountInit;
#define nWinPause 1500  //ms. before we start deleting rows on win screen
#define nWinEraseDelay 90 //ms. time between each row disappearing on win screen
#define nFailBlink 3  //literal. number of times screen flashes on fail screen
bool bAnimateDir = true;  //which way we're moving. t=right,f=left


void butt_isr()
{  bButtFlag = true;
}

void enc_butt_isr()
{  bEncButtFlag = true;
}


void setup()   
{
  //Serial.begin(115200);

  pinMode(buttPin, INPUT_PULLUP);
  pinMode(encButtPin, INPUT_PULLUP);
  attachInterrupt(buttPin, butt_isr, FALLING);
  attachInterrupt(encButtPin, enc_butt_isr, FALLING);
//  pinMode(encApin, INPUT_PULLUP);
//  pinMode(encBpin, INPUT_PULLUP);
  //attachInterrupt(encApin, enc_a_isr, CHANGE);
  //attachInterrupt(encBpin, enc_b_isr, CHANGE);

  if ( ! display.begin(0x3D) ) 
  {
    // Serial.print("Unable to initialize OLED");
     while (1) yield();
  }
  display.cp437(true);
  display.setRotation(3);	//portrait
  display.clearDisplay();
  //display.display();
  display.setTextSize(1);
  display.setTextWrap(false);

  display.setCursor(0,0);
  display.println("stacker\n\n");
  display.println("   bespoke");
  display.println("      oled");
  display.println("   version");
  display.println("by bgilder\n");
  display.println("2022may28\n\n\n");
  //display.setCursor(0,12*8);
  display.println("    press");
  display.println("   button");
  display.println(" to place");
  display.println("   blocks\n\n");
  //display.setCursor(0,18*8);
  display.println("try to");
  display.println("make the");
  display.println("tallest");
  display.println("stack");
  display.setCursor(0,25*8);
  display.println("hold");
  display.println("    to");
  display.print("      start");

  draw_slab(0, nCurrentRow, nBlockCount);
  display.display();
  delay(lSpeedDelay);
  while(digitalRead(buttPin))
  {  animate_row();           //for some reason this is giving bad button response. fix it. or don't - just say to hold on start.
  }
  display.fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH - nBlockHeight, GRAY_BLACK);   //clear everything but the bottom row
}


void loop()
{

  if(bButtFlag) //we check where we're situated, lop off any dead blocks, and determine if we've lost
  {
    bool bLose = false;
    if(nCurrentRow<SCREEN_WIDTH/nBlockHeight - 1) //anything but the bottom row
    {
      if(xPos != xPosPrev)  // oops our edges aren't the same
      {
        nBlockCount > abs(xPos - xPosPrev) ? nBlockCount -= abs(xPos - xPosPrev) : nBlockCount = 0;
        if(nBlockCount)
        {
          if(xPos < xPosPrev)  //we're too far to the left
          { xPos = xPosPrev;  //shift to align with stack
          }
          display.fillRect(0,  nCurrentRow*nBlockHeight,  SCREEN_HEIGHT,  nBlockHeight,  GRAY_BLACK); //cover up our mistakes
        }
        else
        { bLose = true;
        }
        draw_slab(xPos,nCurrentRow,nBlockCount);
      }
    }
    
    xPosPrev = xPos;
    lSpeedDelay *= fDelayShift;   //speed up that timer baybee

    if(bLose)
    {  lose_state();
    }
    else
    {
      if(nCurrentRow)
      {  nCurrentRow--;
      }
      else
      {  win_state();
      }
    }
    bButtFlag = false;
  }
  
  animate_row();

  if(bEncButtFlag && !bResetFlag)
  {
    lResetTimer = millis()+lResetStall;
    bResetFlag  = true;
    bEncButtFlag = false;
    display.drawPixel(SCREEN_HEIGHT-1,0,1);
    display.display();
  }

  if(bResetFlag)
  {
    if(lResetTimer<millis())
    {
      if(!digitalRead(encButtPin))
      {
        display.clearDisplay();
        display.display();
        while(!digitalRead(encButtPin))
        {
          //wait for the sweet release of encButt
        }
      //and then exit the program back to main menu somehow (without creating memory leaks yay!)
      //reset();
      }
      else
      { display.drawPixel(SCREEN_HEIGHT-1,0,0);
      }
      bResetFlag = false;
      bEncButtFlag = false;
    }
  }
}


void draw_slab(uint16_t x, uint16_t y, uint8_t num)
{
	//x gives x position duh
	//y gives y stack duh
	//num is how many blocks across remain

	for(uint8_t blocks = 0; blocks < num; blocks++)
	{
		display.fillRect( (x+blocks)*nBlockWidth,  y*nBlockHeight,  nBlockWidth,  nBlockHeight,  GRAY_1);
		display.drawRect( (x+blocks)*nBlockWidth,  y*nBlockHeight,  nBlockWidth,  nBlockHeight,  GRAY_WHITE);
	}
}


void lose_state()
{
  lSpeedDelay = fSpeedDelayInit;
  nCurrentRow = SCREEN_WIDTH/nBlockHeight - 1;
  xPos = -1;
  nBlockCount = nBlockCountInit;

  for(uint8_t wait = 0; wait < nFailBlink; wait++)
  {
    display.invertDisplay(true);
    display.display();
    delay(400);
    display.invertDisplay(false);
    display.display();
    delay(400);
  }
  display.clearDisplay();
  bResetFlag = false;
  bEncButtFlag = false;
}

void win_state()
{
  lSpeedDelay = fSpeedDelayInit;
  nCurrentRow = SCREEN_WIDTH/nBlockHeight - 1;
  xPos = -1;
  nBlockCount = nBlockCountInit;

  display.display();
  delay(nWinPause);
  
  for(uint8_t rows = SCREEN_WIDTH/nBlockHeight ; rows > 0; rows--)
  {
    display.fillRect(0,  (rows-1)*nBlockHeight,  SCREEN_HEIGHT,  nBlockHeight,  GRAY_BLACK);
    display.display();  //deletes one row at a time from the bottom up
    delay(nWinEraseDelay);   
  }

  //and then animate some fireworks or some shit
  //
  //
  //

  display.setCursor(0,SCREEN_WIDTH/2 - 1);
  display.print(" congrats!");
  display.display();
  delay(3000);
  display.clearDisplay();
  bResetFlag = false;
  bEncButtFlag = false;
}

void animate_row()
{
  display.fillRect( xPos,  nCurrentRow*nBlockHeight,  SCREEN_HEIGHT,  nBlockHeight,  GRAY_BLACK);
  if(bAnimateDir)
  {
    if(xPos+nBlockCount<SCREEN_HEIGHT/nBlockWidth)
    {  xPos++;
    }
    else
    {  
      bAnimateDir = false;
      xPos--;
    }
  }
  else
  {
    if(xPos<1)
    {  
      bAnimateDir = true;
      xPos++;
    }
    else
    {  xPos--;
    }
  }
  draw_slab(xPos,nCurrentRow,nBlockCount);
  display.display();
  unsigned long lWaitTime = millis()+(unsigned long)lSpeedDelay;
  while(millis()<lWaitTime)
  {
    //kill time
  }
}
