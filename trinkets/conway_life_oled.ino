///////////////////////////
// conway's life on my oled
// bgilder
// 15 june 2022
///////////////////////////


#include <gamebox.h>

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait



const unsigned long randThreshold = 30; //rough percentage of screen starting with live cells
volatile int nDelayTime = 45;
#define resetDelay 500
#define delayIncrement 10
#define delayMax 800
int nHistoryCounter = 0;
const int nHistoryThreshold = 30;  //(30*n) (common multiple of oscillators)
unsigned long populationCounter = 0;
bool currentGen[SCREEN_HEIGHT][SCREEN_WIDTH];
bool nextGen[SCREEN_HEIGHT][SCREEN_WIDTH];
bool historyGen[SCREEN_HEIGHT][SCREEN_WIDTH];


void setup()   
{
  oled_setup(SCREEN_ROTATION);

  display.print("conway's game of life    bgilder 16may2022\n  a bespoke oled toroidal implementation\n\n\npress button to begin or restart\n\nuse knob to change simulation speed");
  display.display();

  while(digitalRead(buttPin))  
  {
      //kill time until button press
  }
  //display.clearDisplay();
  randomSeed(millis());
  delay(100);
  randomize_cells();
}


/*
butt_logic()
{
  randomize_cells();
}
*/

void CC_logic()
{
    nDelayTime += delayIncrement;
    if(nDelayTime > delayMax)
    {  nDelayTime = delayMax;
    }
    flag.CCflag = false;
}

void CW_logic()
{
    nDelayTime -= delayIncrement;
    if(nDelayTime < 0)
    { nDelayTime = 0;
    }
    flag.CWflag = false;
}


void loop()
{
  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(flag.buttFlag) 
  {
    randomize_cells(); //butt_logic();
    flag.buttFlag = false;
    unsigned long holding = millis() + resetDelay;
    while(millis() < holding)
    {
      //kill time
    }  
  }

  
  if(flag.CCflag)
  { CC_logic();
  }

  if(flag.CWflag)
  { CW_logic();
  }
  

  if(nDelayTime < delayMax)
  { 
    unsigned long holding = millis() + nDelayTime;
    while(millis() < holding)
    {
        #ifdef ENCODERLIBRARY
        check_encoder();  //update rotational encoder flags
        #endif
        if(flag.CCflag)
        { CC_logic();
        }
        if(flag.CWflag)
        { CW_logic();
        }
    }
  }
  else if(nDelayTime == delayMax)
  {
    display.setCursor(0,0);
    display.print("PAUSED");
    display.display();
    unsigned long holding = millis() + nDelayTime;
    while(millis() < holding)
    {
      //kill time
    }
    for(int row = 0; row < SCREEN_HEIGHT/8; row++)
    {
      for(int col = 0; col < 8*8; col++)
      {  display.drawPixel(col,row,nextGen[row][col]);
      }
    }
    display.display();
    while(nDelayTime == delayMax)
    {
        #ifdef ENCODERLIBRARY
        check_encoder();  //update rotational encoder flags
        #endif
        if(flag.CCflag)
        { CC_logic();
        }
        if(flag.CWflag)
        { CW_logic();
        }
    }
  }

  cell_efficient();   //cell_old();

  bool bChangeDetected = false;
  for(int row = 0; row < SCREEN_HEIGHT; row++)
  {
    for(int col = 0; col < SCREEN_WIDTH; col++)
    {
      display.drawPixel(col,row,nextGen[row][col]);

      if(nHistoryCounter >= nHistoryThreshold)
      {
        if (currentGen[row][col] != historyGen[row][col])
        { bChangeDetected = true;
        }
        historyGen[row][col] = currentGen[row][col];
      }
      currentGen[row][col] = nextGen[row][col];
    }
  }

  if(nHistoryCounter >= nHistoryThreshold)
  {
    nHistoryCounter = 0;
    if(!bChangeDetected)
    { randomize_cells(); //reset if only oscillators remain
    }
  }

  display.display();
  nHistoryCounter++;


  ///// exit handling /////
  if(flag.encButtFlag && !flag.resetFlag)
  {
    lResetTimer = millis()+lResetStall;
    flag.resetFlag  = true;
    flag.encButtFlag = false;
    //display.drawPixel(0,SCREEN_WIDTH-1,GRAY_WHITE);
    //display.display();
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
      { //display.drawPixel(0,SCREEN_WIDTH-1,0);
      }
      flag.resetFlag = false;
      flag.encButtFlag = false;
    }
  }
}

void cell_efficient()   //why yes, this does automatically scale to fit any screen size
{

  bool topleft=false, above=false, topright=false, left=false, right=false, botleft=false, below=false, botright=false;

  for(int row = 0; row < SCREEN_HEIGHT; row++)
  {
    for(int col = 0; col < SCREEN_WIDTH; col++)
    {
      topleft = currentGen[(row-1+SCREEN_HEIGHT) % SCREEN_HEIGHT][(col-1+SCREEN_WIDTH) % SCREEN_WIDTH];
      above = currentGen[(row-1+SCREEN_HEIGHT) % SCREEN_HEIGHT][col];
      topright = currentGen[(row-1+SCREEN_HEIGHT) % SCREEN_HEIGHT][(col+1) % SCREEN_WIDTH];
      left = currentGen[row][(col-1+SCREEN_WIDTH) % SCREEN_WIDTH];
      right = currentGen[row][(col+1) % SCREEN_WIDTH];
      botleft = currentGen[(row+1) % SCREEN_HEIGHT][(col-1+SCREEN_WIDTH) % SCREEN_WIDTH];
      below = currentGen[(row+1) % SCREEN_HEIGHT][col];
      botright = currentGen[(row+1) % SCREEN_HEIGHT][(col+1) % SCREEN_WIDTH];
      
      bool neighbors[] = {topleft,above,topright,left,right,botleft,below,botright};
      uint8_t neighborCount = 0;
      
      for(uint8_t summary = 0; summary < 8; summary++)
      {
        if(neighbors[summary])
        { neighborCount++;
        }
      }
      if(!currentGen[row][col] && neighborCount == 3) //dead cell with 3 live neighbors is born
      { nextGen[row][col] = true;
      }
      else if(currentGen[row][col])
      {
        if(neighborCount < 2 || neighborCount > 3)  //live cell with 2 or 3 live neighbors lives
        { nextGen[row][col] = false;        //else it dies
        }
      }
    }
  }
}

void randomize_cells()
{
  //display.clearDisplay();
  for(int row = 0; row < SCREEN_HEIGHT; row++)
  {
    for(int col = 0; col < SCREEN_WIDTH; col++)
    {
      random(100) < randThreshold ? currentGen[row][col] = true : currentGen[row][col] = false;
      //display.drawPixel(col,row,currentGen[row][col]);
      nextGen[row][col] = currentGen[row][col];
    }
  }

  if(populationCounter)
  {  
    display.setCursor(0,0);
    display.print(populationCounter);
  }
  display.display();
  populationCounter++;
  unsigned long holding = millis() + 250;
  while(millis() < holding)
  {
    //kill time
  }
}
