///////////////////////////
// 1-player pong for er-oledm032
// bgilder 29 may 2022
///////////////////////////

/// 15 july 22 update:
//penalizes relaunch button cheese
//removed detect_collisions()
//paddle-area-specific dy adjustments (new paddle graphic to suggest area differences)
//fiddled with title screen
//decreased paddle move distance (more rotations for full coverage)


/// to do: add left and right graphics.


#include <gamebox.h>

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait


uint8_t nScore = 0;
uint8_t nUpCount  = 0;
uint8_t nDownCount  = 0;
#define nUpThreshold 2
#define nDownThreshold 2
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

void setup()   
{
  oled_setup(SCREEN_ROTATION);
  title_screen_pong();

  while(digitalRead(buttPin))
  {
    // kill time
  }
  flag.buttFlag = 0;  //doesn't give cpu free point at start of game
  display.clearDisplay();
  draw_graphics();
  randomSeed(millis());
  delay(100);
  display.setTextSize(6);
  draw_score();
  draw_paddle();
  draw_ball();
  new_ball();
}

void title_screen_pong()
{   
    display.setCursor(40,2);
    display.println("hey it's a one player pong!");
    display.println("bespoke version by bgilder 30may2022");
    display.println("\n\ntwist to move paddle\n\nbutton relaunches ball");
    display.display();
}

void draw_graphics()
{
  display.drawRect(nLeftBound,0,nRightBound - nLeftBound ,SCREEN_HEIGHT, GRAY_WHITE);
  //and then some pixel art of 1- P L  []  P O N G or something
  display.display(); 
}


void loop()
{
  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

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
    new_ball();
    flag.buttFlag = 0;
  }

  calculate_ball();
  draw_score();
  draw_paddle();
  draw_ball();
}

void new_ball()
{
  x = SCREEN_WIDTH/2;
  y = SCREEN_HEIGHT/2;
  dx = xVelDefault;
  
  dy = random(velBound)*2-velBound; //give random up/down direction for ball to first fire
  paddlePos = paddlePosDefault;
  draw_score();
  draw_paddle();
  draw_ball();
  delay(newBallDelay);
  elapsedTime = millis();
}

void calculate_ball()
{

  unsigned long now = millis();
  elapsedTime = now - elapsedTime;  //this looks fucky but i think it might work
  if(elapsedTime <1) elapsedTime = 1;
 /*   display.setTextSize(1);
  display.setTextColor(GRAY_WHITE);
  display.setCursor(2,2);
  display.fillRect(0,0,SCREEN_WIDTH/4-1,SCREEN_HEIGHT,GRAY_BLACK);
  display.print("el ");
  display.println(elapsedTime);
  display.print("dx ");
  display.println(dx);
  display.print("dy ");
  display.println(dy);
  display.print("x ");
  display.println(x);
  display.print("y ");
  display.print(y);
  */
  
  if(x + dx/elapsedTime <= nLeftBound)  //left side hit. score point against you
  {
    nScore++;
    draw_score();
    new_ball();
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
}


void draw_score()
{
  display.fillRect(nLeftBound,0,nRightBound - nLeftBound ,SCREEN_HEIGHT, GRAY_BLACK);
  display.drawRect(nLeftBound,0,nRightBound - nLeftBound ,SCREEN_HEIGHT, GRAY_3);
  display.setCursor(114,10);
  display.setTextColor(GRAY_1);
  display.print(nScore%=10);
}

void draw_paddle()
{
    if(paddlePos+paddleLength  >= SCREEN_HEIGHT)
      paddlePos = SCREEN_HEIGHT-paddleLength-1;
    if(paddlePos<1)
      paddlePos = 1;
    display.drawRect(nLeftBound+paddleOffset, 1,2,SCREEN_HEIGHT-2, GRAY_BLACK);   //erase entire paddle space (excluding boundary)
    display.drawRect(nLeftBound+paddleOffset+1, paddlePos, 1, paddleLength, GRAY_6);
    display.drawRect(nLeftBound+paddleOffset, paddlePos+paddleLength/4, 2, paddleLength/2, GRAY_6);
}

void draw_ball()
{
  //ballNext = display.getPixel(x,y);
  //display.drawPixel(xPrev,yPrev,ballPrev);
  display.drawRect(x,y,2,2,GRAY_WHITE);
  //ballPrev = ballNext;
  display.display();
}
