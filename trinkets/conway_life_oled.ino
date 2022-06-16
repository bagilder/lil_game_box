///////////////////////////
// conway's life on my oled
// bgilder
// 16 may 2022
///////////////////////////


#include <SPI.h>
#include <SSD1322_for_Adafruit_GFX.h>

#define SCREEN_WIDTH 256 // in pixels
#define SCREEN_HEIGHT 64 // in pixels
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


#define buttPin 2
#define encButtPin 3   
#define encApin 4
#define encBpin 5
#define soundPin 19   

volatile bool bButtFlag = false;
volatile bool bEncButtFlag = false;
volatile bool bCWflag = false;
volatile bool bCCflag = false;
volatile bool bResetFlag = false;
#define lResetStall 10000;  //ms. how long to wait before the reset button does something
unsigned long lResetTimer = 0;


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
  //Serial.begin(9600);

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
     //Serial.print("Unable to initialize OLED");
     while (1) yield();
  }
  display.cp437(true);    //proper extended character set
  display.setRotation(0);   //landscape
  display.clearDisplay();
  //display.display();
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(SSD1322_WHITE);
  display.setCursor(0,0);

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
    // program specific logic for CC rotation
    nDelayTime += delayIncrement;
    if(nDelayTime > delayMax)
    {  nDelayTime = delayMax;
    }
    /////////
  }
  else
  {
    bCWflag = true;
    bCCflag = false;
    
    /////////
    // program specific logic for CW rotation
    nDelayTime -= delayIncrement;
    if(nDelayTime < 0)
    { nDelayTime = 0;
    }
    /////////
  }
}

/*
butt_logic()
{
  randomize_cells();
}
*/


void loop()
{

  if(bButtFlag) 
  {
    randomize_cells(); //butt_logic();
    bButtFlag = false;
    unsigned long holding = millis() + resetDelay;
    while(millis() < holding)
    {
      //kill time
    }  
  }

  /*
  if(bCCflag)
  {

  }

  if(bCWflag)
  {

  }
  */

  if(nDelayTime<delayMax)
  { 
    unsigned long holding = millis() + nDelayTime;
    while(millis() < holding)
    {
      //kill time
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
      //kill time until no longer paused
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
  if(bEncButtFlag && !bResetFlag)
  {
    lResetTimer = millis()+lResetStall;
    bResetFlag  = true;
    bEncButtFlag = false;
    //display.drawPixel(0,SCREEN_WIDTH-1,GRAY_WHITE);
    //display.display();
  }
  if(bResetFlag)
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
      bResetFlag = false;
      bEncButtFlag = false;
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

/*
void draw_history_gen()
{
  for(int row = 0; row < SCREEN_HEIGHT; row++)
  {
    for(int col = 0; col < SCREEN_WIDTH; col++)
    {
      display.drawPixel(col,row,historyGen[row][col]);
      //currentGen[row][col] = nextGen[row][col];
    }
  }
  display.display();
}

void draw_current_gen()
{
  for(int row = 0; row < SCREEN_HEIGHT; row++)
  {
    for(int col = 0; col < SCREEN_WIDTH; col++)
    {
      display.drawPixel(col,row,currentGen[row][col]);
      //currentGen[row][col] = nextGen[row][col];
    }
  }
  display.display();
}

void draw_next_gen()
{
  for(int row = 0; row < SCREEN_HEIGHT; row++)
  {
    for(int col = 0; col < SCREEN_WIDTH; col++)
    {
      display.drawPixel(col,row,nextGen[row][col]);
      //currentGen[row][col] = nextGen[row][col];
    }
  }
  display.display();
}
*/

/*
void cell_old()
{
  bool topleft = false;
  bool above = false;
  bool topright = false;
  bool left = false;
  bool right = false;
  bool botleft = false;
  bool below = false;
  bool botright = false;
  int col = 0;
  int row = 0;

  //top_row_check
  ///first, the leftmost column (col 0) of the top row
  topleft = currentGen[SCREEN_HEIGHT-1][SCREEN_WIDTH-1];
  above = currentGen[SCREEN_HEIGHT-1][0];
  topright = currentGen[SCREEN_HEIGHT-1][1];
  left = currentGen[0][SCREEN_WIDTH-1];
  right = currentGen[0][1];
  botleft = currentGen[1][SCREEN_WIDTH-1];
  below = currentGen[1][0];
  botright = currentGen[1][1];
  do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);
  
  ///then, the middle parts of the top row
  for(col = 1; col < SCREEN_WIDTH-1; col++)
  {
    topleft = currentGen[SCREEN_HEIGHT-1][col-1];
    above = currentGen[SCREEN_HEIGHT-1][col];
    topright = currentGen[SCREEN_HEIGHT-1][col+1];
    left = currentGen[0][col-1];
    right = currentGen[0][col+1];
    botleft = currentGen[1][col-1];
    below = currentGen[1][col];
    botright = currentGen[1][col+1];
    do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);
  }

  ///then, the rightmost column (col SCREEN_WIDTH-1) of the top row
  topleft = currentGen[SCREEN_HEIGHT-1][SCREEN_WIDTH-1-1];
  above = currentGen[SCREEN_HEIGHT-1][SCREEN_WIDTH-1];
  topright = currentGen[SCREEN_HEIGHT-1][0];
  left = currentGen[0][SCREEN_WIDTH-1-1];
  right = currentGen[0][0];
  botleft = currentGen[1][SCREEN_WIDTH-1-1];
  below = currentGen[1][SCREEN_WIDTH-1];
  botright = currentGen[1][0];
  do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);

  //now, the middle parts of the screen
  for(row = 1; row < SCREEN_HEIGHT-1; row++)
  {
    ///left_edge_check
    col = 0;
    topleft = currentGen[row-1][SCREEN_WIDTH-1];
    above = currentGen[row-1][0];
    topright = currentGen[row-1][1];
    left = currentGen[row][SCREEN_WIDTH-1];
    right = currentGen[row][1];
    botleft = currentGen[row+1][SCREEN_WIDTH-1];
    below = currentGen[row+1][0];
    botright = currentGen[row+1][1];
    do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);
    
    for(col = 1; col < SCREEN_WIDTH-1; col++)
    {
      topleft = currentGen[row-1][col-1];
      above = currentGen[row-1][col];
      topright = currentGen[row-1][col+1];
      left = currentGen[row][col-1];
      right = currentGen[row][col+1];
      botleft = currentGen[row+1][col-1];
      below = currentGen[row+1][col];
      botright = currentGen[row+1][col+1];
      do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);
    }

    ///right_edge_check
    col = SCREEN_WIDTH-1;
    topleft = currentGen[row-1][SCREEN_WIDTH-1-1];
    above = currentGen[row-1][SCREEN_WIDTH-1];
    topright = currentGen[row-1][0];
    left = currentGen[row][SCREEN_WIDTH-1-1];
    right = currentGen[row][0];
    botleft = currentGen[row+1][SCREEN_WIDTH-1-1];
    below = currentGen[row+1][SCREEN_WIDTH-1];
    botright = currentGen[row+1][0];
    do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);
  }

  //bottow_row_check
  ///first, the leftmost column (col 0) of the bottom row
  topleft = currentGen[SCREEN_HEIGHT-1-1][SCREEN_WIDTH-1];
  above = currentGen[SCREEN_HEIGHT-1-1][0];
  topright = currentGen[SCREEN_HEIGHT-1-1][1];
  left = currentGen[SCREEN_HEIGHT-1][SCREEN_WIDTH-1];
  right = currentGen[SCREEN_HEIGHT-1][1];
  botleft = currentGen[0][SCREEN_WIDTH-1];
  below = currentGen[0][0];
  botright = currentGen[0][1];
  do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);

  ///then, the middle parts of the bottom row
  for(col = 1; col < SCREEN_WIDTH-1; col++)
  {
    topleft = currentGen[SCREEN_HEIGHT-1-1][col-1];
    above = currentGen[SCREEN_HEIGHT-1-1][col];
    topright = currentGen[SCREEN_HEIGHT-1-1][col+1];
    left = currentGen[SCREEN_HEIGHT-1][col-1];
    right = currentGen[SCREEN_HEIGHT-1][col+1];
    botleft = currentGen[0][col-1];
    below = currentGen[0][col];
    botright = currentGen[0][col+1];
    do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);
  }

  ///then, the rightmost column (col SCREEN_WIDTH-1) of the bottom row
  topleft = currentGen[SCREEN_HEIGHT-1-1][SCREEN_WIDTH-1-1];
  above = currentGen[SCREEN_HEIGHT-1-1][SCREEN_WIDTH-1];
  topright = currentGen[SCREEN_HEIGHT-1-1][0];
  left = currentGen[SCREEN_HEIGHT-1][SCREEN_WIDTH-1-1];
  right = currentGen[SCREEN_HEIGHT-1][0];
  botleft = currentGen[0][SCREEN_WIDTH-1-1];
  below = currentGen[0][SCREEN_WIDTH-1];
  botright = currentGen[0][0];
  do_da_counting(row,col,topleft,above,topright,left,right,botleft,below,botright);



}

void do_da_counting(int row_,int col_,bool tl,bool a,bool tr,bool l,bool r,bool bl,bool b,bool br)
{
  uint8_t neighborCount = 0;
  bool neighbors[] = {tl,a,tr,l,r,bl,b,br};

  for(uint8_t summary = 0; summary < 8; summary++)
  {
    if(neighbors[summary])
    { neighborCount++;
    }
  }

  if(!currentGen[row_][col_] && neighborCount == 3) //dead cell with 3 live neighbors is born
  { nextGen[row_][col_] = true;
  }
  else if(currentGen[row_][col_])
  {
    if(neighborCount < 2 || neighborCount > 3)  //live cell with 2 or 3 live neighbors lives
    { nextGen[row_][col_] = false;        //else it dies
    }
  }
}
*/
