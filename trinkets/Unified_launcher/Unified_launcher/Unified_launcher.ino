/////////////////
// let's have the gamebox run multiple things at once yeah
// with like a menu or some shit
// 28 feb 2026
/////////////////


#include <gamebox.h>
#include <vector>
#include <stack>
#include "C:\Users\bagil\Documents\Projects_\gamebox\trinkets\raycaster\3d_frankenstein\imagedata.h"


#define PROJECTILE_SPEED 0.2
#define RANDOM_MAX 6
#define SPRITE_LIMIT 16

struct sObject      //one lone coder sprite implementation
{
  float x;
  float y;
  float vx; //item velocity for movement
  float vy; 
  bool removed;
  byte objectType;
  byte *image;
};

std::vector<sObject> objectVector =
{
  {6.5, 8.5, 0, 0, false, 0, sprite[0]},
  {16.5, 16.5, 0, 0, false, 0, sprite[0]},
  {10.5, 3.5, 0, 0, false, 0, sprite[0]}
};



const float FOVhalf = radians(35);  //half of the player's total field of view in degrees, to save a single devision lol will be heading+this and heading-this (edge wrapped)
const float maxVel = .025;  //full run speed. we're just guessing to start
const float turnSpeed = PI/32;  //degrees. how far we tick in a direction on encoder blip. might have to change if converting everything to floats
const byte numWalls = 5;
const float maxSpeed = .1;
const byte deceleration = 2*maxSpeed;
const byte blockSize = 1;
//const int rightEdge = (SCREEN_WIDTH-1)/ blockSize;
const int rightEdge = (SCREEN_WIDTH-1)/ 2;
const int leftEdge = 0;
const int bottomEdge = (SCREEN_HEIGHT-1)/ blockSize;
const int topEdge = 0;
const int viewWidth = SCREEN_WIDTH-1 - rightEdge;   //these will have to change if we swap window orientations
const int viewHeight = SCREEN_HEIGHT-1;// - bottomEdge;
int drawDistance = (SCREEN_HEIGHT-1);
const int mazeWidth = 24;
const int mazeHeight = 24;
const float clippingDist = 1;
const int spriteHeight = 64;
const int spriteWidth = 64;


const byte mapArray[mazeWidth][mazeHeight]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1} 
};



int frameRate;
byte frameRefreshCount = 0;

float pHeadingX = -1.5;       //center of player's vield of view
float pHeadingY = 0;      //making this a vector instead of an angle to lean on https://lodev.org/cgtutor/raycasting.html
float pHeading = pHeadingY/pHeadingX; //is this right?? man i'm bad at geometry
float pVelX = 0;
float pVelY = 0;
float pSpeed = 0;   //player's movement i guess?
float pVel = 0;
float pX = leftEdge+3;   //player position in x
float pY = topEdge+3;  //player position in y
float cPlaneX = 0;        //camera plane vector in x
float cPlaneY = .66;     //ditto y
unsigned long tPrevFrame, tCurrentFrame, frameTime;
float zbuffer[viewWidth];


uint8_t SCREEN_ROTATION = 0;
typedef struct  
{
  int x, y;
  unsigned
    north:1,
    east:1,
    south:1,
    west:1,
    visited:1;
} positionboy;

std::vector<positionboy> blockList; 
std::stack<int> mazeStack;

const int mazeBlockSize = 8;   //how many pixels square each maze block will be
const int rows = (SCREEN_HEIGHT) / mazeBlockSize;
const int cols = (SCREEN_WIDTH) / mazeBlockSize;
const int drawDelay = 25;
int totalVisits = 0;
int currentPos = 0;  //index in the maze blocklist
uint8_t nUpCount  = 0;  //used for main menu and pong
uint8_t nDownCount  = 0;
#define nUpThreshold 2
#define nDownThreshold 2//used for main menu and pong and translator

uint8_t currentCharacter = 0;
uint16_t characters[8] = {0x5a00,0x5c1e,0xa200,0xd600,0x5404,0xea06,0xa200,0};  //"translation" for splashscreen
enum alphabet {BBB,DDD,FFF,GGG,HHH,JJJ,KKK,LLL,MMM,NNN,PPP,RRR,SSS,TTT,VVV,WWW,YYY,ZZZ,ch,sh,these,think,zh,ing,blank1,
                ay,uh,aw,hat,hard,ee,eh,look,eye,ih,shoo,oh,oy,ow,blank2,normal,invert};
alphabet alphaVar;
String characterList[42] = {"  B","  D","  F","  G","  H","  J","  K","  L","  M","  N","  P","  R","  S","  T","  V","  W","  Y","  Z",
                            " CH"," SH","(TH)ESE","(TH)INK"," ZH"," ING","[none]",
                            " AY"," UH"," AW","H(A)T","H(A)RD"," EE"," EH","L(OO)K"," EYE"," IH","SH(OO)"," OH"," OY"," OW","[none]","normal","invert"};
#define CONS_START BBB
#define CONS_END blank1
#define VOWEL_START ay
#define VOWEL_END blank2


enum gameOption {lifeGame, stackerGame, pongGame, mazeGen, etchGame, raycastGame, translateGame};
gameOption c;

const int NUMTRINKETS = 7;
String displayLine[NUMTRINKETS] = { "game of life",
                                    "stacker",
                                    "pong",
                                    "maze",
                                    "etch a sketch",
                                    "raycaster",
                                    "translate" };


void setup()
{
  oled_setup(0);
  display.setTextColor(GRAY_WHITE, GRAY_1);
  display.setCursor(0,0);
  display.println("select a trinket:");
  display.setTextColor(GRAY_BLACK,GRAY_WHITE);
  display.println(displayLine[0]);
  display.setTextColor(GRAY_WHITE, GRAY_BLACK);
  for (byte i = 1; i<NUMTRINKETS; i++)   
  {  display.println(displayLine[i]);
  }
  display.display();
  flag.buttFlag = 0;
  flag.resetFlag = 0;
  flag.encButtFlag = 0;
  flag.CCflag = 0;
  flag.CWflag = 0;
}

void loop() 
{

  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(flag.CCflag)  //stolen from pong encoder handling but backwards
  {
    nUpCount++;
    flag.CCflag = 0;
  }
  if(flag.CWflag)  //stolen from pong encoder handling but backwards
  {
    nDownCount++;
    flag.CWflag = 0;
  }

  if(nUpCount > nUpThreshold+3)  //CC move! stolen from pong encoder handling but extra
  {
    nUpCount = 0;
    nDownCount = 0;
    if((c > 0))
    {
      display.fillRect(0, 8*(c+1), SCREEN_WIDTH, 8, GRAY_BLACK);
      display.setTextColor(SSD1322_WHITE);
      display.setCursor(0,8*(c+1));
      display.print(displayLine[c]);
      display.setCursor(0,8*(c));
      display.setTextColor(GRAY_BLACK, GRAY_WHITE);
      display.print(displayLine[c-1]);
      display.display();
      c=c-1;
    }
    flag.CCflag = 0;
  }

  if(nDownCount > nDownThreshold+3)  //CW move! stolen from pong encoder handling but extra
  {
    nUpCount = 0;
    nDownCount = 0;
    if(c < NUMTRINKETS-1)
    {
      display.fillRect(0, 8*(c+1), SCREEN_WIDTH, 8, GRAY_BLACK);
      display.setTextColor(SSD1322_WHITE);
      display.setCursor(0,8*(c+1));
      display.println(displayLine[c]);
      display.setTextColor(GRAY_BLACK, GRAY_WHITE);
      display.println(displayLine[c+1]);
      display.display();
      c=c+1;
    }
    flag.CWflag = 0;
  }

  if(flag.buttFlag)
  {
    delay(500);
    display.setTextColor(GRAY_WHITE);
    display.setCursor(0,0);
    flag.buttFlag = 0;
    flag.CWflag = 0;
    flag.CCflag = 0;
    display.clearDisplay();

    switch (c)
    {
      case lifeGame:

        life();
      break;

      case stackerGame:

        oled_setup(3);
        stacker();
      break;

      case pongGame:

        pong();
      break;

      case mazeGen:

        maze();
      break;

      case etchGame:

        etch();
      break;

      case raycastGame:

        raycaster();
      break;

      case translateGame:

        translate();
      break;

    }
  }
}


void life() 
{

  SCREEN_ROTATION= 0;
  int resetDelay = 500;
  const unsigned long randThreshold = 30; //rough percentage of screen starting with live cells
  volatile int nDelayTime = 45;
  int delayIncrement= 10;
  int delayMax =800;
  int nHistoryCounter = 0;
  const int nHistoryThreshold = 30;  //(30*n) (common multiple of oscillators)
  unsigned long populationCounter = 0;
  bool currentGen[SCREEN_HEIGHT][SCREEN_WIDTH];
  bool nextGen[SCREEN_HEIGHT][SCREEN_WIDTH];
  bool historyGen[SCREEN_HEIGHT][SCREEN_WIDTH];
  unsigned long holding = millis() + 250;

  //oled_setup(SCREEN_ROTATION);

  display.print("conway's game of life    bgilder 16may2022\n  a bespoke oled toroidal implementation\n\n\npress button to begin or restart\n\nuse knob to change simulation speed");
  display.display();

  while(digitalRead(buttPin))  
  {
      //kill time until button press
  }
  //display.clearDisplay();
  randomSeed(millis());
  delay(100);
  ///////////life_randomize_cells());
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
  holding = millis() + 250;
  while(millis() < holding)
  {
    //kill time
  }
  //////////////////////////

  while(1)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif

    if(flag.buttFlag) 
    {
      ////////////////life_randomize_cells(currentGen, nextGen); //butt_logic();
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
      holding = millis() + 250;
      while(millis() < holding)
      {
        //kill time
      }
      ///////////////////////

      flag.buttFlag = 0;
      holding = millis() + resetDelay;
      while(millis() < holding)
      {
        //kill time
      }  
    }


    if(flag.CCflag)
    { 
      nDelayTime += delayIncrement;
      if(nDelayTime > delayMax)
      {  nDelayTime = delayMax;
      }
      flag.CCflag = 0;
    }

    if(flag.CWflag)
    { 
      nDelayTime -= delayIncrement;
      if(nDelayTime < 0)
      { nDelayTime = 0;
      }
      flag.CWflag = 0;
    }


    if(nDelayTime < delayMax)
    { 
      holding = millis() + nDelayTime;
      while(millis() < holding)
      {
          #ifdef ENCODERLIBRARY
          check_encoder();  //update rotational encoder flags
          #endif
          if(flag.CCflag)
          { 
            nDelayTime += delayIncrement;
            if(nDelayTime > delayMax)
            {  nDelayTime = delayMax;
            }
            flag.CCflag = 0;
          }
          if(flag.CWflag)
          { 
            nDelayTime -= delayIncrement;
            if(nDelayTime < 0)
            { nDelayTime = 0;
            }
            flag.CWflag = 0;
          }
      }
    }
    else if(nDelayTime == delayMax)
    {
      display.setCursor(0,0);
      display.print("PAUSED");
      display.display();
      holding = millis() + nDelayTime;
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
          { 
            nDelayTime += delayIncrement;
            if(nDelayTime > delayMax)
            {  nDelayTime = delayMax;
            }
            flag.CCflag = 0;
          }
          if(flag.CWflag)
          { 
            nDelayTime -= delayIncrement;
            if(nDelayTime < 0)
            { nDelayTime = 0;
            }
            flag.CWflag = 0;
          }
      }
    }


    //////// life_cell_efficient();   //cell_old();
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
      { 
        //////////////life_randomize_cells(currentGen, nextGen); //reset if only oscillators remain
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
          holding = millis() + 250;
          while(millis() < holding)
          {
            //kill time
          }
      }
    }

    display.display();
    nHistoryCounter++;

/*
    ///// exit handling /////
    if(flag.encButtFlag && !flag.resetFlag)
    {
      lResetTimer = millis()+lResetStall;
      flag.resetFlag  = 1;
      flag.encButtFlag = 0;
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
        return;
        }
        else
        { //display.drawPixel(0,SCREEN_WIDTH-1,0);
        }
        flag.resetFlag = 0;
        flag.encButtFlag = 0;
      }
    }
*/
  }
}//end life


//////////stacker section /////////////

void stacker()
{

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
      display.println("press");
      display.println("    to");
      display.print("      start");
      display.display();
      while(digitalRead(buttPin))
      {
      }
      flag.buttFlag = 0;
      delay(500);
      display.clearDisplay();
      stacker_draw_slab(0, nCurrentRow, nBlockCountInit);
      display.display();
      delay(lSpeedDelay);
      display.fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH - nBlockHeight, GRAY_BLACK);   //clear everything but the bottom row

      




  while(1)
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
        stacker_draw_slab(xPos,nCurrentRow,nBlockCount);
      }

      xPosPrev = xPos;
      lSpeedDelay *= fDelayShift;   //speed up that timer baybee. trucates for millis()

      if(bLose)
      {  //lose state
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
        flag.resetFlag = 0;
        flag.encButtFlag = 0;
      }
      else
      {
        if(nCurrentRow)
        {  nCurrentRow--;
        }
        else
        {  //win state
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
          flag.resetFlag = 0;
          flag.encButtFlag = 0;
        }
      }
      flag.buttFlag = 0;
    }


////animate row
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
  stacker_draw_slab(xPos,nCurrentRow,nBlockCount);
  display.display();
  unsigned long lWaitTime = millis()+(unsigned long)lSpeedDelay;
  while(millis()<lWaitTime)
  {
    //kill time
  }
//////animate row////
/*
    ///// exit handling /////
    if(flag.encButtFlag && !flag.resetFlag)
    {
      lResetTimer = millis()+lResetStall;
      flag.resetFlag  = 1;
      flag.encButtFlag = 0;
      display.drawPixel(SCREEN_HEIGHT-1,0,1);
      display.display();
    }
    ///// exit handling /////
    if(flag.encButtFlag && !flag.resetFlag)
    {
      lResetTimer = millis()+lResetStall;
      flag.resetFlag  = 1;
      flag.encButtFlag = 0;
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
        return;
        }
        else
        { //display.drawPixel(0,SCREEN_WIDTH-1,0);
        }
        flag.resetFlag = 0;
        flag.encButtFlag = 0;
      }
    }/////////////////////WHY DOESN'T THIS WORK THE SAME WAY IT DOES IN LIFE??????????
*/
  }
}//end stacker


void stacker_draw_slab(uint16_t x, uint16_t y, uint8_t num)
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



//////////////////end stacker section//////////////////




///////////////////pong////////////////////////

void pong()
{

  uint8_t nScore = 0;
  float x = SCREEN_WIDTH/2;
  float xPrev = x;
  float y = SCREEN_HEIGHT/2;
  float yPrev = y;
  #define xVelDefault -5
  #define velBound 5
  float dx = xVelDefault;;   //velocity x
  float dy = 0;   //velocity y
  #define nLeftBound SCREEN_WIDTH/3
  #define nRightBound 2*SCREEN_WIDTH/3
  uint8_t ballPrev = 0;   //storing colors for ball wake redraw
  uint8_t ballNext = 0;
  #define newBallDelay 1000
  #define paddleOffset  1
  #define paddleLength SCREEN_HEIGHT/4
  #define paddlePosDefault SCREEN_HEIGHT/2 - paddleLength/2
  int8_t paddlePos = paddlePosDefault;
  #define paddleMove 4
  unsigned long elapsedTime = 0;
  unsigned long now = millis();
  bool newBallSignal = true;
  display.setCursor(40,2);
  display.println("hey it's a one player pong!");
  display.println("bespoke version by bgilder 30may2022");
  display.println("\n\ntwist to move paddle\n\nbutton relaunches ball");
  display.display();
  while(digitalRead(buttPin))
  {  // kill time
  }
  flag.buttFlag = 0;  //doesn't give cpu free point at start of game
  display.clearDisplay();
  display.drawRect(nLeftBound,0,nRightBound - nLeftBound ,SCREEN_HEIGHT, GRAY_WHITE);
  //and then some pixel art of 1- P L  []  P O N G or something
  display.display(); 
  randomSeed(millis());
  delay(100);
  display.setTextSize(6);


  while(1)
  {
  
#ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(newBallSignal)
  {
    x = SCREEN_WIDTH/2;
    y = SCREEN_HEIGHT/2;
    dx = xVelDefault;

    randomSeed(millis()%255);
    dy = random(velBound)*3-velBound; //give random up/down direction for ball to first fire
    paddlePos = paddlePosDefault;
  /*  draw_score();
    draw_paddle();
    draw_ball();*/
  }

    //////////////draw_score();
  display.fillRect(nLeftBound,0,nRightBound - nLeftBound ,SCREEN_HEIGHT, GRAY_BLACK);
  display.drawRect(nLeftBound,0,nRightBound - nLeftBound ,SCREEN_HEIGHT, GRAY_3);
  display.setCursor(114,10);
  display.setTextColor(GRAY_1);
  display.print(nScore%=10);
//////////// end score draw
  
  //////////////draw_paddle();
  if(paddlePos+paddleLength  >= SCREEN_HEIGHT)
    paddlePos = SCREEN_HEIGHT-paddleLength-1;
  if(paddlePos<1)
    paddlePos = 1;
  display.drawRect(nLeftBound+paddleOffset, 1,2,SCREEN_HEIGHT-2, GRAY_BLACK);   //erase entire paddle space (excluding boundary)
  display.drawRect(nLeftBound+paddleOffset+1, paddlePos, 1, paddleLength, GRAY_6);
  display.drawRect(nLeftBound+paddleOffset, paddlePos+paddleLength/4, 2, paddleLength/2, GRAY_6);
  ////////////////////// end paddle draw
    
  //////////draw_ball();
  display.drawRect(x,y,2,2,GRAY_WHITE);
  display.display();
  ///////// end ball draw

  if(newBallSignal)
  {
    delay(newBallDelay);
    elapsedTime = millis();
    newBallSignal = false;
  }

  if(flag.CWflag)
  {
    nUpCount++;
    flag.CWflag = 0;
  }
  if(flag.CCflag)
  {
    nDownCount++;
    flag.CCflag = 0;
  }
  if(nUpCount > nUpThreshold)
  {
    paddlePos + paddleLength >= SCREEN_HEIGHT-1 ? paddlePos = SCREEN_HEIGHT-1-paddleLength : paddlePos+=paddleMove; 
    nUpCount = 0;
    nDownCount = 0;
  }
  if(nDownCount > nDownThreshold)
  {
    paddlePos <= 1 ? paddlePos = 1 : paddlePos-=paddleMove; 
    nUpCount = 0;
    nDownCount = 0;
  }
  if(flag.buttFlag)
  {
    nScore++;   //penalizes relaunch-to-avoid-loss cheese
    //new_ball();
    newBallSignal = true;
    flag.buttFlag = 0;
  }

  ////////////////////calculate_ball();
  unsigned long now = millis();
  elapsedTime = now - elapsedTime;  //this looks fucky but i think it might work
  if(elapsedTime <1) elapsedTime = 1;
  if(x + dx/elapsedTime <= nLeftBound)  //left side hit. score point against you
  {
    nScore++;
   // draw_score();
    //new_ball();
    newBallSignal = true;
  }
  if(x+dx/elapsedTime>=nRightBound-1) //right side hit. bounce back
  {  dx *= -1;
  }
  if(y+dy/elapsedTime <= 1 || y+dy/elapsedTime >= SCREEN_HEIGHT-1 ) //ceiling or floor hit. bounce back
  { dy *= -1;
  }
  if(x+dx/elapsedTime < paddleOffset+nLeftBound+1 && (y>= paddlePos && y<=(paddlePos+paddleLength))) //paddle hit (i hope). bounce back 
  { 
    dx *= -1;
    dx+=.5;
    if(y>=paddlePos && y<(paddlePos+paddleLength/4))  //if we hit on the bottom quarter of the paddle
    {
      if(dy<0)
      { dy += 0.25*dy;
      }
      else
      { dy -= .3*dy;
      }
    }
    else if(y>(paddlePos+3*paddleLength/4) && y<=(paddlePos+paddleLength))  //if we hit on the top quarter of the paddle
    {
      if(dy<0)
      { dy -= .3*dy;
      }
      else
      { dy += .25*dy;
      }
    }
    if(!dy)     //so we don't get stuck back and forth forever
    { dy = 0.1;
    }
  }
  xPrev = x;
  yPrev = y;
  if(dy>2*velBound) 
    dy> 5*velBound ? dy = velBound : dy = 2*velBound;
  if(dy<-2*velBound) 
    dy<-5*velBound ? dy = -velBound : dy = -2*velBound;
  x = x+dx/elapsedTime;
  y = y+dy/elapsedTime;
  elapsedTime = now;
////////// end ball calculate
  
  }
}






void maze()
{

  display.print("recursive backtracking maze generator\n\n   bgilder 16july2022, 9sept2022\n\n randomly generates fully connected maze\n\n\n press button for new maze");
  display.display();

  while(digitalRead(buttPin))  
  {  delay(1);   //take it easy, bud
  }
  flag.buttFlag = 0;
  display.clearDisplay();
  display.display();
  randomSeed(millis());
  delay(500);

  while(1)
  {
  
    display.clearDisplay();
    display.display();
    delay(500);
    ////maze_populate_vector();
    while(!blockList.empty())
    {  blockList.pop_back();     //this is hacky but shut up. easy way to let me reuse it
    }
    while(!mazeStack.empty())
    {  mazeStack.pop();          //avoid those memory leaks boo
    }
    totalVisits = 1;
    //currentPos = 0;   //removing this reset starts the new maze where the old one left off. how fun
    for(int j = 0; j < rows; j++)
    {
      for(int i = 0; i < cols; i++)
      {
        positionboy newBoy;
        newBoy.x = i;
        newBoy.y = j;
        newBoy.north = 1;
        newBoy.east = 1;
        newBoy.south = 1;
        newBoy.west = 1;
        newBoy.visited = 0;
        blockList.push_back(newBoy);
        maze_draw_position(i,j);
      }
    }
    display.display();
    ////end maze populate vetor
    
    blockList[currentPos].visited = 1;
    while(totalVisits < cols*rows)
    {
      int uno = blockList[currentPos].x;
      int dos = blockList[currentPos].y;
      ////maze_neighbors(uno,dos);
      std::vector<int> neighborChoices;
        
      int upindex = uno + (dos-1)*cols;
      int rightindex = (uno+1) + dos*cols;  //or, currentPos+1
      int downindex = uno + (dos+1)*cols;
      int leftindex = (uno-1) + dos*cols;   //or, currentPos-1
    
      if(dos > 0  &&  !blockList[upindex].visited)  //we're not on the top row & can move up
      { neighborChoices.push_back(0);
      }
      if(uno < cols-1  &&  !blockList[rightindex].visited) //we're not on the right side & can move right
      { neighborChoices.push_back(1);
      }
      if(dos < rows-1  &&  !blockList[downindex].visited) //we're not on the bottom row & can move down
      {  neighborChoices.push_back(2);
      }
      if(uno > 0  &&  !blockList[leftindex].visited)  //we're not on the left side & can move left
      { neighborChoices.push_back(3);
      }
    
      if(!neighborChoices.empty())
      {
        switch(neighborChoices[random(neighborChoices.size())])   //choose among the viable neighbor movement options
        {
          case 0:   //up
            blockList[currentPos].north = 0;  //if there is an available path to the north, kill the north wall
            blockList[upindex].south = 0;     //and also mark the cell above as not having a south wall anymore
            currentPos = upindex;             //then push to the top of the stack the northern neighbor's coordinates
            break;
          case 1:   //right
            blockList[currentPos].east = 0;   //if there is an available path to the east, kill the east wall
            blockList[rightindex].west = 0;   //and also mark the cell to the right as not having a west wall anymore
            currentPos = rightindex;          //then push to the top of the stack the eastern neighbor's coordinates
            break;
          case 2:   //down
            blockList[currentPos].south = 0;  //if there is an available path to the south, kill the south wall
            blockList[downindex].north = 0;   //and also mark the cell below as not having a north wall anymore
            currentPos = downindex;           //then push to the top of the stack the southern neighbor's coordinates
            break;  
          case 3:   //left 
            blockList[currentPos].west = 0;   //if there is an available path to the west, kill the west wall
            blockList[leftindex].east = 0;    //and also mark the cell to the left as not having an east wall anymore            
            currentPos = leftindex;           //then push to the top of the stack the western neighbor's coordinates
            break;  
        }
        totalVisits++;
        mazeStack.push(currentPos);
        blockList[currentPos].visited = 1;
      }
      else  
      {     //pop off the stack to backtrack
        currentPos = mazeStack.top();
        mazeStack.pop();
      }
      display.fillRect(blockList[currentPos].x*mazeBlockSize+1,blockList[currentPos].y*mazeBlockSize+1,mazeBlockSize-1,mazeBlockSize-1,GRAY_5);   
      display.display();    //show where the current position is, as a treat
      delay(drawDelay);
      //// end maze neighbors
  
      
      maze_draw_position(uno,dos);
    }
    display.display();
    delay(1000);
    maze_draw_position(blockList[currentPos].x,blockList[currentPos].y);
    display.display();  //hide final position square
    while(digitalRead(buttPin))
    {  delay(1);   //no need to burn yourself out while waiting
    }
  }
}


void maze_draw_position(int _x, int _y)
{
  int index = _x + _y*cols;
  int xApparent = _x*mazeBlockSize;
  int yApparent = _y*mazeBlockSize;
  display.drawRect(xApparent,yApparent,mazeBlockSize+1,mazeBlockSize+1,GRAY_1);   //remove old drawn walls
  if(blockList[index].north)
  { display.drawLine(xApparent,yApparent,xApparent+mazeBlockSize,yApparent,GRAY_4);                     //never
  }
  if(blockList[index].east)
  { display.drawLine(xApparent+mazeBlockSize,yApparent,xApparent+mazeBlockSize,yApparent+mazeBlockSize,GRAY_3); //eat
  }
  if(blockList[index].south)
  { display.drawLine(xApparent+mazeBlockSize,yApparent+mazeBlockSize,xApparent,yApparent+mazeBlockSize,GRAY_4); //shredded
  }
  if(blockList[index].west)
  { display.drawLine(xApparent,yApparent+mazeBlockSize,xApparent,yApparent,GRAY_3);                     //wheat
  }
  if(blockList[index].visited)
  { display.fillRect(xApparent+1,yApparent+1,mazeBlockSize-1,mazeBlockSize-1,GRAY_1); //after the walls are drawn, show where we've been
  }
}







void etch() 
{

  display.print("one-knob etch-a-sketch\n\n    hold button to change vert/horiz");
  display.display();

  while(digitalRead(buttPin))  
  {  delay(1);   //take it easy, bud
  }
  flag.buttFlag = 0;
  display.clearDisplay();
  display.display();

  const char holdDelay  = 75; //ms
  const char randThreshold = 30;
  const char eraseIteration = 10;   //number of times the randomized clearing function will run before blacking the screen
  const long centerHoriz = SCREEN_WIDTH/2-1;
  const long centerVert = SCREEN_HEIGHT/2-1;
  const int vertOffset = SCREEN_HEIGHT-1;   //for vertical encoder behavior switching. change to 0 or SCREEN_HEIGHT-1 to reverse vertical movement behavior
  
  
  // THESE ARE THE VARIABLES
  bool dissolveState = true;
  bool dissolveStatePrev = dissolveState;
  bool holdState = false;
  bool holdStatePrev = false;
  long newVert = centerVert;
  long newHoriz = centerHoriz;
  long positionHoriz = centerHoriz;
  long positionVert = centerVert;
  unsigned long holdTime = 0;
  long startLastVert = centerVert;   //for draw_line. trying to kill the line stutter effect.
  long startLastHoriz = centerHoriz;


  while(1)
  {

    if(flag.buttFlag)   //the button done gone been pressed
    {
      bool quickCheck = digitalRead(buttPin);
      if(quickCheck)
      {
        holdState = 0;      //oops not anymore
        flag.buttFlag = 0;
        knob.write(-1*positionHoriz);
      }
      else
      {
        if(holdStatePrev)     //we were holding the button and still are, so vertical mode
        {
          newVert = knob.read();  //up and down       //absolute
          
          if(newVert < 0)   //bounds check
          { 
            newVert = 0;
            knob.write(0);  //directly overwrites encoder position
          }
          else if(newVert > SCREEN_HEIGHT-1)
          { 
            newVert = SCREEN_HEIGHT-1;
            knob.write(SCREEN_HEIGHT-1);
          }
          if(positionVert != newVert)
          { 
            ////etch_draw_point(GRAY_2);   //kills the white dot
            display.drawPixel(positionHoriz,vertOffset-positionVert, GRAY_2);   //(height-1 - vert) reverses direction of movement on vertical knob spin. do what feels good
            display.display();
            //// end draw point
            ////etch_draw_line(newVert,GRAY_2,true);
            display.drawLine(positionHoriz, vertOffset-startLastVert, positionHoriz, vertOffset-newVert, GRAY_2);   //start x, start y, end x, end y, color 
            startLastVert = newVert;
            display.display();
            ////end draw line
            positionVert = newVert;
          } 
        }
        else
        {
          unsigned long currentNow = millis();
          if(currentNow >= holdTime + holdDelay)
          {
            holdTime = millis();
            holdState = true;           //pretty sure the first entrance triggers a hold but whatever
            knob.write(positionVert);
          }
        }
      }
      holdStatePrev = holdState;   
    }
    else
    {
      newHoriz = knob.read();  //left and right  //absolute
      newHoriz *= -1;
  
      if(newHoriz < 0)    //bounds check
      { 
        newHoriz = 0;
        knob.write(0); //directly overwrites encoder position
      }
      else if(newHoriz > SCREEN_WIDTH-1)
      { 
        newHoriz = SCREEN_WIDTH-1;
        knob.write(-1*(SCREEN_WIDTH-1));
      }
      if(positionHoriz != newHoriz)
      { 
        ////etch_draw_point(GRAY_3);   //kills the white dot  
        display.drawPixel(positionHoriz,vertOffset-positionVert, GRAY_3);   //(height-1 - vert) reverses direction of movement on vertical knob spin. do what feels good
        display.display();
        //// end draw point
        ////etch_draw_line(newHoriz,GRAY_3,false);   //gray3 because of pwm latency on oled itself. horiz lines get darker the more the row is illuminated. "feels" better
        display.drawLine(startLastHoriz, vertOffset-positionVert, newHoriz, vertOffset-positionVert, GRAY_3);   //start x, start y, end x, end y, color  
        startLastHoriz = newHoriz;
        display.display();
        //// end draw line
        positionHoriz = newHoriz;
      }
    }
  
    ////etch_draw_point(GRAY_WHITE);   //draw current cursor location
    display.drawPixel(positionHoriz,vertOffset-positionVert, GRAY_WHITE);   //(height-1 - vert) reverses direction of movement on vertical knob spin. do what feels good
    display.display();
    //// end draw point

    dissolveState = digitalRead(encButtPin);
    if(!dissolveState && dissolveStatePrev)
    {
      delay(holdDelay);
      if(!digitalRead(encButtPin))
      { 
        ////pixel_dissolve(); // this is the only call. let's move it inline
        for(int iteration = 0; iteration < eraseIteration; iteration++)
        {
          for(int row = 0; row < SCREEN_HEIGHT; row++)
          {
            for(int col = 0; col < SCREEN_WIDTH; col++)
            {
              if(random(100) < randThreshold) 
              { display.drawPixel(col, row, GRAY_BLACK);
              }
            }
          }
          display.display();
        }
        display.clearDisplay();
        display.display();
        ////end pixel dissolve
  
      }  
    }
    dissolveStatePrev = dissolveState;
  }
}



void raycaster()
{

display.print("raycasting demo on the game box\n\nbgilder 23 sept 2022, 19 oct 2022\n\n\nwhy on gods green earth did i decide to\nbuild this from scratch");
  display.display();

  while(digitalRead(buttPin))  
  {  delay(1);   //take it easy, bud
  }
  flag.buttFlag = 0;
  display.clearDisplay();
  display.display();
  
  randomSeed(millis());
    tCurrentFrame = millis();

    while(1)
    {
    pHeadingX = cosf(pHeading);
    pHeadingY = sinf(pHeading);
    player_movement();
    render_frame();
    }

}


void fire_bullet() {
  //a test of projectiles and object list culling
  sObject o;
  o.x = pX;
  o.y = pY;
  float noise = (random(RANDOM_MAX)-0.5*RANDOM_MAX)/RANDOM_MAX;
  o.vx = cosf(pHeading + 0.1*noise)*PROJECTILE_SPEED;
  o.vy = sinf(pHeading + 0.1*noise)*PROJECTILE_SPEED;
  o.image = sprite[1];
  o.removed = false;
  o.objectType = 1;
  objectVector.push_back(o);
}

void draw_sprites()
{
  
  //is the object within the user's FOV?
  for(int each = 0; each < objectVector.size(); each++)  //for(auto &object : objectList)
  {
    //sObject object = objectList.data[each];  //this is so dumb. why doesn't atmel support lists
    sObject object2 = objectVector[each];

    //update object physics
    object2.x += object2.vx;
    object2.y += object2.vy;
    if(object2.objectType == 1 && mapArray[(int)object2.x][(int)object2.y] > 0)  //if a projectile and it hits a wall
      object2.removed = true;
    
    float sVecX = object2.x - pX;
    float sVecY = object2.y - pY;
    float sDist = sqrtf(sVecX*sVecX + sVecY*sVecY);  //pythagoras!
    
    //calculate angle between sprite and player
    float sAngle = -(atan2f(pHeadingY,pHeadingX) - atan2f(sVecY, sVecX));    //do we need to include the camera plane vectors here????   
    if(sAngle < -PI)        //this is currently 0->2pi, but his was -pi -> +pi. 
    {  sAngle += PI*2;
    }
    if(sAngle > PI)
    {  sAngle -= PI*2;
    }

    display.drawPixel(object2.x,object2.y,GRAY_3);  //show object on screen 
    
    bool canSee = fabs(sAngle) < FOVhalf;

/*
    //diagnostic for barrel sprite visibility
      display.setCursor(4*8,(4+each)*8);
      display.print(degrees(sAngle));
      display.print(" ");
      display.print(canSee);
      display.setCursor(4*8,7*8);
      if(each==2)
      display.print(sDist);
*/
      //diagnostic for objectVector length
      display.setCursor(4*8,4*8);
      display.print(objectVector.size());
          
    if(canSee && sDist>clippingDist && sDist < drawDistance)
    {

      float sHead = (viewHeight / 2) - viewHeight/sDist; //idr why the +1 on the walls but i'm putting it here just in cases (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistanceFromPlayer)
      float sToe = viewHeight-sHead;
      float sHeight = sToe - sHead;   //olc method
      //float sHeight =2*viewHeight/sDist;  //yonny zohar lecture method (approximately i guess)
      /*if(sHeight >= spriteHeight)
        sHeight = spriteHeight-1;*/
      float getRatioed = spriteHeight/spriteWidth;  //for square sprites this will be 1
      float sWidth = sHeight / getRatioed;    //for square sprites this will also be 1
     /* if(sWidth >= spriteWidth)
        sWidth = spriteWidth-1;*/
      float sPos = ((sAngle/FOVhalf)*0.5 + 0.5)*(float)viewWidth;   //center location of sprite //view width because we aren't ray tracing columns anymore?

    /*
    //diagnostic for different sprite distance calculation types

    if(each==2)
    {
      display.setCursor(0*8,4*8);
      display.println(sHeight);
      display.print(2*viewHeight/sDist);
     }
    */
      
      for(int sX = sWidth-1; sX >=0; sX--)   //because of the mirroring problem
      {
        for(int sY = sHeight-1; sY >= 0; sY--)   //draw sprite pixels    ///because of the mirroring problem
        {
          float sSampleX = sX/sWidth;   //normalized for screen space. a proportion i guess? but ours is based on an array so i'll have to change this. handy for png tho
          float sSampleY = sY/sHeight;
          int sColumn = (int)(sPos + sX - (sWidth/2));
          if(sColumn>=0 && sColumn<viewWidth)
          {
            byte texel = object2.image[spriteWidth * (int)(sSampleY*spriteHeight) + (int)(sSampleX*spriteWidth)]; //sampleX is percentage, need to scale it back up to full texture size
            if(texel && zbuffer[sColumn] >= sDist)
            { 
              display.drawPixel(sColumn+rightEdge,sToe-sY,texel);
              zbuffer[sColumn] = sDist;   //breaks transparency at sprite overlap
            }
          }
        }
      }
    }
    objectVector[each].x = object2.x;
    objectVector[each].y = object2.y;
    objectVector[each].removed = object2.removed;
  }
}



void frame_rate()
{
  if(++frameRefreshCount > 3)
  {
    frameRate = 1000/frameTime;
    frameRefreshCount = 0;
  }
  display.setTextSize(0);
  display.setCursor(0,7*8);
  display.print((int)frameRate);
}

void player_movement()
{
  ///first, let's figure out one-knob movement

  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(flag.buttFlag)
  {
      fire_bullet();
      flag.buttFlag = 0;
  }

  if(!digitalRead(encButtPin))
  {
    if(++pSpeed > maxSpeed)    //will this sneaky auto increment my variable? i think maybe
    { pSpeed = maxSpeed;
    }
    pVel += frameTime * pSpeed / blockSize; //will this give me framerate compensation? framerate = 1/frame time, so.....
    if(pVel > maxVel)
    {  pVel = maxVel;
    }
    
    pVelX = pHeadingX*pSpeed;
    pVelY = pHeadingY*pSpeed;

    if(!mapArray[int(pX+pVelX*3)][int(pY)]) //if the space we're going to isn't occupied
    {  pX += pVelX/frameRate*60;    //these two can be +='d above to save ops but this is for debugging help 
    }
    if(!mapArray[int(pX)][int(pY+pVelY*3)]) 
    {  pY += pVelY/frameRate*60;
    }
    flag.encButtFlag = 0;
  }
  else
  {  pSpeed -= deceleration / frameTime; // tune decel for comfort
    pVel -=pSpeed;
  }

  if(flag.CWflag)   //turn right
  {
    pHeading += turnSpeed;  
    if(pHeading >= PI*2)
    {  pHeading -= PI*2;
    }
    float oldPlaneX = cPlaneX;  //realign camera plane
    cPlaneX = cPlaneX * cosf(turnSpeed) - cPlaneY * sinf(turnSpeed);
    cPlaneY = oldPlaneX * sinf(turnSpeed) + cPlaneY * cosf(turnSpeed);
    flag.CWflag = 0;
  }

  if(flag.CCflag)   //turn left
  {
    pHeading -= turnSpeed;  
    if(pHeading < 0)
    {  pHeading += PI*2;
    }
    float oldPlaneX = cPlaneX;  //realign camera plane
    cPlaneX = cPlaneX * cosf(-turnSpeed) - cPlaneY * sinf(-turnSpeed);
    cPlaneY = oldPlaneX * sinf(-turnSpeed) + cPlaneY * cosf(-turnSpeed);
    flag.CCflag = 0;
  }

  if(pX > rightEdge -1)
  {  pX = rightEdge -1;
  }
  else if (pX <= leftEdge)
  {  pX = leftEdge+1;
  }
  if(pY > bottomEdge -1)
  {  pY = bottomEdge -1;
  }
  else if(pY <= topEdge)
  {  pY = topEdge+1;
  }
}

bool shouldDelete(sObject & o)
{
  return o.removed;
}

void render_frame()
{
  tCurrentFrame = millis();
  frameTime = tCurrentFrame - tPrevFrame;
  ///display minimap and rays  
  for(int row = 0; row < mazeHeight; row++)
  {
    for(int col = 0; col < mazeWidth; col++)
    {  display.drawPixel(row,col,mapArray[row][col]);
    }
  }
  display.drawLine(pX,pY,pX+3*cosf(pHeading),pY+3*sinf(pHeading), GRAY_3);  //shows player heading, relies on heading being an angle
  
  //garbage collect sprites
  objectVector.erase(
    remove_if(objectVector.begin(), objectVector.end(), shouldDelete),
    objectVector.end());
/*     
  for(int each = objectList.length -1; each >= 0 ; each--)  //for(auto &object : objectList)
  {
    if(objectList.data[each].removed)   //this is so stupidddd why can't atmel just support lists
      objectList.remove(each);    //currently there is a bug that erases all sprites later than a sprite that gets removed. changing to each-- didn't fix it
  }
*/  
  ///cast_rays_olc(); //known functional
  cast_rays();
  draw_sprites();
  display.drawPixel(pX,pY, GRAY_WHITE); //player pixel
  //display.drawFastVLine(rightEdge, topEdge, bottomEdge, GRAY_WHITE);  //view area boundary
  frame_rate();     
  display.display();
  //delay(100);
  display.clearDisplay();
  tPrevFrame = tCurrentFrame;
}




void cast_rays()
{
  //this is using a DDA implementation from https://lodev.org/cgtutor/raycasting.html
  //Digital Differential Analysis

  for(int xCols = 0; xCols < viewWidth; xCols++)
  {
    //calculate ray position and direction
    /*right side of the screen will get coordinate 1, the center of the screen gets coordinate 0, and the left side of the screen gets coordinate -1*/

    float cameraX = 2 * xCols / float(viewWidth) - 1; //x-coordinate in camera space
    float rayDirX = pHeadingX + cPlaneX * cameraX;
    float rayDirY = pHeadingY + cPlaneY * cameraX;

    int pMapX = (int)pX;  //quantized map space
    int pMapY = (int)pY;

    float rayStepDistX, rayStepDistY; //distances dda ray steps go in x grid and y grid block sides

    //length of ray from one x or y-side to next x grid or y grid block side
    //these are derived as:
    //deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
    //deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
    //which can be simplified to abs(|rayDir| / rayDirX) and abs(|rayDir| / rayDirY)
    //where |rayDir| is the length of the vector (rayDirX, rayDirY). Its length,
    //unlike (dirX, dirY) is not 1, however this does not matter, only the
    //ratio between deltaDistX and deltaDistY matters, due to the way the DDA
    //stepping further below works. So the values can be computed as below.
    // Division through zero is prevented, even though technically that's not
    // needed in C++ with IEEE 754 floating point values.

    float deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);

    //what direction to step in x or y-direction (either +1 or -1)
    int stepX;
    int stepY;

    /**determine the initial length and directions**/
    //if the x component of the ray direction is negative   //meaning looking left
    if(rayDirX < 0) 
    {
      stepX = -1;   //we'll be checking negatively
      rayStepDistX = (pX - pMapX) * deltaDistX;
    }
    else    //looking right
    {
      stepX = 1;
      rayStepDistX = (pMapX + 1.0 - pX) * deltaDistX;
    }
    //if the y component of the ray direction is negative //meaning looking up
    if(rayDirY < 0) 
    {
      stepY = -1;   //we'll be checking negatively
      rayStepDistY = (pY - pMapY) * deltaDistY;
    }
    else    //looking down
    {
      stepY = 1;
      rayStepDistY = (pMapY + 1.0 - pY) * deltaDistY;
    }

    /***and now we do the actual DDA***/

    float perpWallDist; //the length of the ray perpendicular to the camera plane
    bool hit = 0; //was there a wall hit?
    bool side;    //was a NS or a EW wall hit? ;;x grid side = 0, y grid side = 1

    while(!hit)
    {
      //jump to next map square, either in x-direction, or in y-direction
      if(rayStepDistX < rayStepDistY)
      {
        rayStepDistX += deltaDistX;  //if x traversal is shorter, move in x direction
        pMapX += stepX;
        side = 0;
      }
      else
      {
        rayStepDistY += deltaDistY; //if y traversal is shorter, move in y direction
        pMapY += stepY;
        side = 1;
      }
      //Check if ray has hit a wall
      if(mapArray[pMapX][pMapY] != 0) 
      {  hit = 1;
      }
    }

    float destinationX, destinationY;  

    //Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
    if(side) 
    {  perpWallDist = (rayStepDistY - deltaDistY);
    }
    else          
    {  perpWallDist = (rayStepDistX - deltaDistX);
    }
    /*if(perpWallDist > drawDistance)
    {  perpWallDist = drawDistance;
    } */

    destinationX = pX + pHeadingX+rayDirX*perpWallDist;
    destinationY = pY + pHeadingY+rayDirY*perpWallDist;

    if(perpWallDist > 0 && perpWallDist <= drawDistance) //if we can see walls
    {  
      display.drawLine(pX,pY,destinationX,destinationY, GRAY_1);  //actually draw the ray
      
      //and now draw some "3d" walls?????? oh boy!
       int colHeight = 2*viewHeight/perpWallDist;  //unfortunately, this version of raycasting makes our distant walls itty bitty. oh well

    /*  if(colHeight > viewHeight)
      {  colHeight = viewHeight;  //seriously trippy glitches when we clip thru walls otherwise
      }*/ //moved to textured_walls to fix texture clipping
      if(colHeight < 0)
      {  colHeight = 0;
      }

       /// update depth buffer

       zbuffer[xCols] = perpWallDist;
       
     // textured_walls(xCols,perpWallDist,rayDirX,rayDirY, side, pMapX, pMapY, colHeight, destinationX, destinationY);  //for portability's sake

float fSampleX = 0.0;
 // Determine where ray has hit wall. Break Block boundary
            // int 4 line segments
            float fBlockMidX = (float)pMapX + 0.5f;
            float fBlockMidY = (float)pMapY + 0.5f;

            float fTestPointX = pX + rayDirX * perpWallDist;
            float fTestPointY = pY + rayDirY * perpWallDist;

//display.setCursor(5*8,5*8);
//display.println(fTestPointX);

            float fTestAngle = atan2f((fTestPointY - fBlockMidY), (fTestPointX - fBlockMidX));

            if (fTestAngle >= -PI * 0.25f && fTestAngle < PI * 0.25f)
              fSampleX = fTestPointY - (float)pMapY;
            if (fTestAngle >= PI * 0.25f && fTestAngle < PI * 0.75f)
              fSampleX = fTestPointX - (float)pMapX;
            if (fTestAngle < -PI * 0.25f && fTestAngle >= -PI * 0.75f)
              fSampleX = fTestPointX - (float)pMapX;
            if (fTestAngle >= PI * 0.75f || fTestAngle < -PI * 0.75f)
              fSampleX = fTestPointY - (float)pMapY;
//display.print(fSampleX);
//display.display();
////delay(100);
//display.clearDisplay();





   // perpWallDist = cos(fRayAngle-pHeading)*pMapX + sin(fRayAngle-pHeading)*pMapY;   //fisheye correction?
      // Calculate distance to ceiling and floor
      int nCeiling = (float)(viewHeight / 2.0) - viewHeight / ((float)perpWallDist);
      int nFloor = viewHeight-nCeiling;
      int nHeight = nCeiling - nFloor;

       
//int drawEnd = viewHeight/2 + viewHeight/perpWallDist/2;
//int drawStart = viewHeight/2 - viewHeight/perpWallDist/2;



    for (int y = viewHeight-1; y >=0; y--)
      {
        // Each Row
        if (y > nCeiling && y <= nFloor)
        {
          // Draw Wall
          if (perpWallDist < drawDistance)
          {
            byte textureNum = mapArray[(int)pMapX][(int)pMapY]-1;   //-1 to 0-align with texture array
            float fSampleY = ((float)y - (float)nCeiling) / ((float)nFloor - (float)nCeiling);
                  byte tempColor = texture[textureNum][textureHeight * (int)(fSampleY*textureHeight) + (int)(fSampleX*textureWidth)];
                  if(side)
                    tempColor -= 2;
              display.drawPixel(rightEdge+xCols, viewHeight-y, tempColor);    //draw the textured column to the screen buffer
          }
        }
      }



      // map the different distances onto different shades of gray
      //byte tempColor = map( colHeight, 0, viewHeight, 2,15);   //16 colors isn't as distinct, but it still looks a bit better than just my 8 //reserving ceiling & floor colors
      //byte tempColor = 3*mapArray[pMapX][pMapY];
            
      display.drawFastVLine(rightEdge+xCols, viewHeight/2 + colHeight/2 ,viewHeight/2 - colHeight/2 + 1, GRAY_1);  //floor different color than ceiling
      //display.drawFastVLine(rightEdge+xCols, viewHeight/2 - colHeight/2, colHeight, tempColor); //side? GRAY_3:GRAY_6); ///this will have to change if we add textures
    }
    else  //no walls in our view
    {
      display.drawFastVLine(rightEdge+xCols, viewHeight/2, viewHeight/2+1, GRAY_1); //fake it with blank space
      //display.drawPixel(rightEdge+xCols,viewHeight/2,GRAY_BLACK); //some vanishing point fakery
    }
  }
}




/////////////////////////// translate ////////////////////


void translate()
{
  oled_setup(0);
  flag.buttFlag = 0;
  flag.resetFlag = 0;
  display.setTextColor(GRAY_WHITE, GRAY_BLACK);
  display.clearDisplay();
  for(currentCharacter = 0; currentCharacter<7; currentCharacter++)
  {
    draw_chars();
    display.fillRect(0, 56, SCREEN_WIDTH-1, 8,  GRAY_BLACK); //x0,y0,w,h,color 
    characters[currentCharacter] = 0;
  }
  currentCharacter = 0;
  display.setCursor(165, 56);
  display.print("bgilder 26mar26");
  display.display();
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
  }
  display.clearDisplay();
  display.display();
  flag.buttFlag = 0;
  flag.encButtFlag = 0;
  flag.CCflag = 0;
  flag.CWflag = 0;

  while(1)
  {
    alphaVar = normal;
    select_inversion();
    if(currentCharacter>0 && currentCharacter<8)
    {  ask_continue();
    }
  }

}



void select_inversion()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
    if(flag.CWflag)
    {
      nDownCount++;
      flag.CWflag = 0;
    }
    if(flag.CCflag)
    {
      nUpCount++;
      flag.CCflag = 0;
    }
    if (nDownCount > nDownThreshold+3)
    { 
      nUpCount = 0;
      nDownCount = 0;
      characters[currentCharacter] = 0b0000000000000001; //yes inverted, vowel first
      alphaVar = invert;
    }
    else if(nUpCount > nUpThreshold+3)
    { 
      nUpCount = 0;
      nDownCount = 0;
      characters[currentCharacter] = 0;  //not inverted, cons first
      alphaVar = normal;
    }
    draw_chars();
  }
  flag.buttFlag = 0;

  if(characters[currentCharacter])  //inverted
  { select_vowel();
  }
  else
  { select_consonant();
  }
}


void select_consonant()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
    if(flag.CCflag)
    {
      nDownCount++;
      flag.CCflag = 0;
    }
    if(flag.CWflag)
    {
      nUpCount++;
      flag.CWflag = 0;
    }
    if (nUpCount > nUpThreshold+3)  //boundaries, folks
    { 
      nUpCount = 0;
      nDownCount = 0;
      alphaVar = alphaVar+1;
    }
    if(nDownCount > nDownThreshold+3)  //healthy boundaries
    { 
      nUpCount = 0;
      nDownCount = 0;
      if(alphaVar == CONS_START)
        alphaVar = CONS_END;
      else
        alphaVar = alphaVar-1;
    }
    if (alphaVar > CONS_END)  //boundaries, folks 
    { alphaVar = CONS_START;
    }
    if(alphaVar < CONS_START)  //healthy boundaries    //this Should be caught in the previous Else but jic
    { alphaVar = CONS_END;
    }

    switch(alphaVar)
    {
      case BBB:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001010001111111;
        break;

      case DDD:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1011010001111111;
        break;

      case FFF:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0111100001111111;
        break;

      case GGG:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101100001111111;
        break;

      case HHH:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101010001111111;
        break;

      case JJJ:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0011010001111111;
        break;

      case KKK:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001110001111111;
        break;

      case LLL:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101010001111111;
        break;

      case MMM:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1010000001111111;
        break;

      case NNN:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1010001001111111;
        break;

      case PPP:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101100001111111;
        break;

      case RRR:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101110001111111;
        break;

      case SSS:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0111110001111111;
        break;

      case TTT:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101101000111111;
        break;

      case VVV:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001011001111111;
        break;

      case WWW:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0000101001111111;
        break;

      case YYY:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101011001111111;
        break;

      case ZZZ:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101011001111111;
        break;

      case ch:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101001001111111;
        break;
        
      case sh:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1110101001111111;
        break;
        
      case these:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1111010001111111;
        break;
        
      case think:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101111001111111;
        break;
        
      case zh:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1011111001111111;
        break;

      case ing:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1111111001111111;
        break;
      
      case blank1:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0000000001111111;
        break;
    }
    draw_chars();
  }
  flag.buttFlag = 0;
  if(characters[currentCharacter] & 0b0000000000000001) //if it's reversed, this will be second so let's move to the next position
  {
    display.fillRect((30*currentCharacter)+0, 56, SCREEN_WIDTH-1-(30*currentCharacter), 8,  GRAY_BLACK); //x0,y0,w,h,color
    if(characters[currentCharacter] < 2)  //if double null, erase the midline. multiple words~!
    {  display.fillRect((30*currentCharacter)+0, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
    }
    else if((characters[currentCharacter] > 255) && (characters[currentCharacter] & 0b0000000001111110)) //there's actually a consonant-vowel pair
    {  
      display.drawCircle((30*currentCharacter)+15, 51, 2, GRAY_WHITE); //x, y, r, color
      display.drawCircle((30*currentCharacter)+15, 51, 3, GRAY_WHITE); //x, y, r, color
    }
    display.display();
    currentCharacter++;
    if(currentCharacter > 7)
    {  end_character_row();
    }
  }
  else  //otherwise, this will be first
  { select_vowel();
  }
}


void select_vowel()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
    if(flag.CCflag)
    {
      nDownCount++;
      flag.CCflag = 0;
    }
    if(flag.CWflag)
    {
      nUpCount++;
      flag.CWflag = 0;
    }
    if (nUpCount > nUpThreshold+3)  //boundaries, folks
    { 
      nUpCount = 0;
      nDownCount = 0;
      alphaVar = alphaVar+1;
    }
    if(nDownCount > nDownThreshold+3)  //healthy boundaries
    { 
      nUpCount = 0;
      nDownCount = 0;
      alphaVar = alphaVar-1;
    }
    if (alphaVar > VOWEL_END)  //boundaries, folks     ////i'm sure we could integrate the switches and just keep track of which phoneme type we're on for choosing where our boundaries are
    { alphaVar = VOWEL_START;
    }
    if(alphaVar < VOWEL_START)  //healthy boundaries
    { alphaVar = VOWEL_END;
    }

    switch(alphaVar)
    {
      case eye:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000011;
        break;
        
      case ay:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000101;
        break;
        
      case uh:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000111;
        break;
        
      case aw:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011101;
        break;
        
      case hat:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011111;
        break;

      case hard:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011001;
        break;

      case shoo:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100111111;
        break;
        
      case oh:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101111111;
        break;
        
      case ee:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101111101;
        break;
        
      case eh:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101111001;
        break;
        
      case look:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100111001;
        break;
        
      case ih:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101100001;
        break;
        
      case oy:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100100001;
        break;
        
      case ow:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101000001;
        break;

      case blank2:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000001;
        break;  
    }
    draw_chars();
  }
  flag.buttFlag = 0;
  if(characters[currentCharacter] & 0b0000000000000001) //if it's reversed, this will be first
  {  
    alphaVar = CONS_START;
    select_consonant();
  }
  else  //otherwise, this will be second so let's delete the english letters and move to the next position
  { 
    display.fillRect((30*currentCharacter)+0, 56, SCREEN_WIDTH-1-(30*currentCharacter), 8,  GRAY_BLACK); //x0,y0,w,h,color 
    if(characters[currentCharacter] < 2)  //if double null, erase the midline. multiple words~!
    {  display.fillRect((30*currentCharacter)+0, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
    }
    display.display();
    currentCharacter++;
  }
  if(currentCharacter > 7)
  {  end_character_row();
  }
}


void end_character_row()
{
    while(!flag.buttFlag)
    {
    }
    flag.buttFlag = 0;
    for(byte a = 0; a < 8; a++)
    {  characters[a] = 0;
    }
    display.clearDisplay();
    display.display();
    currentCharacter = 0;
}


void draw_chars()  //this feels pretty inelegant and brute force but let's do it 
{
  int charDisplayOffset = (30*currentCharacter);
  //delete current character's index bounding square. fresh slate, babyyy
  display.fillRect(charDisplayOffset+0, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
  display.drawLine(charDisplayOffset+0, 23, charDisplayOffset+30, 23, GRAY_WHITE);  //draw midline
  display.drawLine(charDisplayOffset+0, 24, charDisplayOffset+30, 24, GRAY_WHITE);
  display.fillRect(charDisplayOffset+0, 56, SCREEN_WIDTH-1-charDisplayOffset, 8,  GRAY_BLACK); //x0,y0,w,h,color    //just in cases
  display.setCursor(charDisplayOffset+0, 56); //type which state we're choosing
  if(alphaVar == think)
  {
    display.print("TH");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("ink");
  }
  else if (alphaVar == these)
  {
    display.print("TH");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("ese");
  }
  else if (alphaVar == hat)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("h");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("A");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("t");
  }
  else if (alphaVar == hard)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("h");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("A");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("rd");
  }
  else if (alphaVar == look)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("L");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("OO");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("K");
  }
  else if (alphaVar == shoo)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("sh");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("OO");
  }
  else
  {  display.print(characterList[alphaVar]);
  }
  display.setTextColor(GRAY_WHITE, GRAY_BLACK);
  
  //actually draw the shapes dawg
  if(characters[currentCharacter] & 0b0000000000000010)
  {
    display.drawLine(charDisplayOffset+15, 0, charDisplayOffset+30, 9, GRAY_WHITE);  //draw segment 1
    display.drawLine(charDisplayOffset+15, 1, charDisplayOffset+30, 10, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000000100)
  { 
    display.drawLine(charDisplayOffset+0, 9, charDisplayOffset+15, 0, GRAY_WHITE);   //draw segment 2
    display.drawLine(charDisplayOffset+0, 10, charDisplayOffset+15, 1, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000001000)
  { 
    display.drawLine(charDisplayOffset+0, 9, charDisplayOffset+0, 23, GRAY_WHITE);   //draw segment 3
    display.drawLine(charDisplayOffset+1, 9, charDisplayOffset+1, 23, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000010000)
  { 
    display.drawLine(charDisplayOffset+0, 30, charDisplayOffset+0, 39, GRAY_WHITE);   //draw segment 4
    display.drawLine(charDisplayOffset+1, 30, charDisplayOffset+1, 39, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000100000)
  { 
    display.drawLine(charDisplayOffset+0, 39, charDisplayOffset+15, 48, GRAY_WHITE);  //draw segment 5
    display.drawLine(charDisplayOffset+0, 40, charDisplayOffset+15, 49, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000001000000)
  { 
    display.drawLine(charDisplayOffset+15, 48, charDisplayOffset+30, 39, GRAY_WHITE);  //draw segment 6
    display.drawLine(charDisplayOffset+15, 49, charDisplayOffset+30, 40, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000001000000000)
  { 
    display.drawLine(charDisplayOffset+0, 9, charDisplayOffset+15, 18, GRAY_WHITE);  //draw segment 9
    display.drawLine(charDisplayOffset+0, 10, charDisplayOffset+15, 19, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000010000000000)
  { 
    display.drawLine(charDisplayOffset+15, 0, charDisplayOffset+15, 18, GRAY_WHITE);  //draw segment 10
    display.drawLine(charDisplayOffset+16, 0, charDisplayOffset+16, 18, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000100000000000)
  { 
    display.drawLine(charDisplayOffset+15, 18, charDisplayOffset+30, 9, GRAY_WHITE); //draw segment 11
    display.drawLine(charDisplayOffset+15, 19, charDisplayOffset+30, 10, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0001000000000000)
  { 
    display.drawLine(charDisplayOffset+15, 18, charDisplayOffset+15, 23, GRAY_WHITE); //draw segment 12
    display.drawLine(charDisplayOffset+16, 18, charDisplayOffset+16, 23, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0010000000000000)
  { 
    display.drawLine(charDisplayOffset+0, 39, charDisplayOffset+15, 30, GRAY_WHITE);  //draw segment 13
    display.drawLine(charDisplayOffset+0, 40, charDisplayOffset+15, 31, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0100000000000000)
  { 
    display.drawLine(charDisplayOffset+15, 30, charDisplayOffset+15, 48, GRAY_WHITE); //draw segment 14
    display.drawLine(charDisplayOffset+16, 30, charDisplayOffset+16, 48, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b1000000000000000)
  { 
    display.drawLine(charDisplayOffset+15, 30, charDisplayOffset+30, 39, GRAY_WHITE); //draw segment 15
    display.drawLine(charDisplayOffset+15, 31, charDisplayOffset+30, 40, GRAY_WHITE);
  }
  display.display();
}


void ask_continue()
{
  bool discontinuationate = 0;
  display.setCursor((30*currentCharacter)+2, 28);
  display.print("MORE?");
  display.setCursor((30*currentCharacter)+5, 38);
  display.print("yes");
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif

    if(flag.CCflag)
    {
      nDownCount++;
      flag.CCflag = 0;
    }
    if(flag.CWflag)
    {
      nUpCount++;
      flag.CWflag = 0;
    }

    if (nDownCount > nDownThreshold+3)  //boundaries, folks
    { 
      nUpCount = 0;
      nDownCount = 0;  
      display.setCursor((30*currentCharacter)+5, 38);
      display.print("yes");
      discontinuationate = 0;
    }
    else if(nUpCount > nUpThreshold+3)  //healthy boundaries
    { 
      nUpCount = 0;
      nDownCount = 0;
      display.setCursor((30*currentCharacter)+5, 38);
      display.print("no  ");
      discontinuationate = 1;
    }
    display.display();
  }
  flag.buttFlag = 0;
  if (discontinuationate)
  {
    display.fillRect((30*currentCharacter)+1, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
    display.display();
    end_character_row();
  }
}

