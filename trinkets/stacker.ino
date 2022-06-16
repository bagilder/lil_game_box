///////////////////////////
// stacker for er-oledm032
// bgilder 15 june 2022
///////////////////////////



#include <gamebox.h>

#define SCREEN_ROTATION 3 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait



#define nBlockWidth 8   //must be even factor of screen height (for portrait)
#define nBlockHeight 8  //must be even factor of screen width (for portrait)

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


void setup()   
{
  oled_setup(SCREEN_ROTATION);

  title_screen_stacker();
}


void title_screen_stacker()
{
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
  display.println("stack\n\n\n");
  //display.setCursor(0,25*8);
  display.println("hold");
  display.println("    to");
  display.print("      start");

  draw_slab(0, nCurrentRow, nBlockCount);
  display.display();
  delay(lSpeedDelay);
  while(digitalRead(buttPin))
  {  animate_row();           //for some reason this is giving bad button response. fix it ///or don't, just say to hold on start
  }
  display.fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH - nBlockHeight, GRAY_BLACK);   //clear everything but the bottom row
}


void loop()
{
  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(flag.buttFlag) //we check where we're situated, lop off any dead blocks, and determine if we've lost
  {
    bool bLose = false;
    if(nCurrentRow>=SCREEN_WIDTH/nBlockHeight - 1) //only the bottom row
    { xPosPrev = xPos;
    }
    if(xPos != xPosPrev)  // oops our edges aren't the same
    {
      nBlockCount > abs(xPos - xPosPrev) ? nBlockCount -= abs(xPos - xPosPrev) : nBlockCount = 0;
      if(nBlockCount)
      {
        if(xPos < xPosPrev)  //we're too far to the left
        { xPos = xPosPrev;  //shift to align with stack
        }
        display.fillRect(0, nCurrentRow*nBlockHeight, SCREEN_HEIGHT, nBlockHeight, GRAY_BLACK); //cover up our mistakes
      }
      else
      { bLose = true;
      }
      draw_slab(xPos,nCurrentRow,nBlockCount);
    }
    
    xPosPrev = xPos;
    lSpeedDelay *= fDelayShift;   //speed up that timer baybee. trucates for millis()

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
    flag.buttFlag = false;
  }
  
  animate_row();


  ///// exit handling /////
  if(flag.encButtFlag && !flag.resetFlag)
  {
    lResetTimer = millis()+lResetStall;
    flag.resetFlag  = true;
    flag.encButtFlag = false;
    display.drawPixel(SCREEN_HEIGHT-1,0,1);
    display.display();
  }
  if(flag.resetFlag)
  {
    if(lResetTimer<millis())
    {
      if(!digitalRead(encButtPin))    //yes i know these are nested but i'm doing that intentionally
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
      flag.resetFlag = false;
      flag.encButtFlag = false;
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
		display.fillRect((x+blocks)*nBlockWidth, y*nBlockHeight, nBlockWidth, nBlockHeight, GRAY_1);
		display.drawRect((x+blocks)*nBlockWidth, y*nBlockHeight, nBlockWidth, nBlockHeight, GRAY_WHITE);
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
  flag.resetFlag = false;
  flag.encButtFlag = false;
}

void win_state()
{
  lSpeedDelay = fSpeedDelayInit;
  nCurrentRow = SCREEN_WIDTH/nBlockHeight - 1;
  xPos = -1;
  nBlockCount = nBlockCountInit;

  display.display();
  delay(nWinPause);
  
  for(uint8_t rows = SCREEN_WIDTH/nBlockHeight ; rows > 0; rows--) //deletes one row at a time from the bottom up
  {     
    display.fillRect(0, (rows-1)*nBlockHeight, SCREEN_HEIGHT, nBlockHeight, GRAY_BLACK);
    display.display();  
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
  flag.resetFlag = false;
  flag.encButtFlag = false;
}

void animate_row()
{
  display.fillRect( xPos, nCurrentRow*nBlockHeight, SCREEN_HEIGHT, nBlockHeight, GRAY_BLACK); //erase current row
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
