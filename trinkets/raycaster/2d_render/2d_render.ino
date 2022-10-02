///////////////////////////
// fuck it let's try a raycasting engine
// bgilder
// 23 sept 2022  
///////////////////////////

////start with player movement and go from there
/// 2D renderer first, then 3d drawing


#include <gamebox.h>
#include <vector>
#include <stack>

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait

const byte FOVhalf = 35;  //half of the player's total field of view in degrees, to save a single devision lol will be heading+this and heading-this (edge wrapped)
const float maxVel = .25;  //full run speed. we're just guessing to start
const float turnSpeed = PI/32;  //degrees. how far we tick in a direction on encoder blip. might have to change if converting everything to floats
const byte numWalls = 5;
const float maxSpeed = 1;
const byte deceleration = 2*maxSpeed;
const byte blockSize = 1;
//const int rightEdge = (SCREEN_WIDTH-1)/ blockSize;
const int rightEdge = (SCREEN_WIDTH-1)/ 2;
const int leftEdge = 0;
const int bottomEdge = (SCREEN_HEIGHT-1)/ blockSize;
const int topEdge = 0;

float frameRate;
int frameRefreshCount = 0;

float pHeading = 0;   //an angle, center of player's vield of view
float pVelX = 0;
float pVelY = 0;
float pSpeed = 0;   //player's movement i guess?
float pVel = 0;
float pX = rightEdge/2;
float pY = bottomEdge/2;
unsigned long tPrevFrame, tCurrentFrame, frameTime;

std::vector<int> wallVectorX;
std::vector<int> wallVectorY;


void setup()   
{
  oled_setup(SCREEN_ROTATION);    //gamebox
/*  //this is in random_walls now
  while(!wallVectorX.empty())   //i know this is super extremely unnecessary but let my brain do its own paranoia stuff ok
  { wallVectorX.pop_back();
  }
  while(!wallVectorY.empty())
  { wallVectorY.pop_back();
  }
*/
  display.print("raycasting demo on the game box\n\nbgilder 23 sept 2022\n\n\nwhy on gods green earth did i decide to\nbuild this from scratch");
  display.display();
  

  while(digitalRead(buttPin))  
  {  delay(1);   //take it easy, bud
  }
  display.clearDisplay();
  display.display();

randomSeed(millis());




}

void loop()
{
  random_walls();
  delay(1000);
  tCurrentFrame = millis();
  while(digitalRead(buttPin))  
  {  delay(10);   //take it easy, bud
      player_movement();
        render_frame();
  }
  pX = rightEdge/2;
  pY = bottomEdge/2;
  

}


void random_walls()
{
  while(!wallVectorX.empty())    //i know this is super extremely unnecessary but let my brain do its own paranoia stuff ok
  { wallVectorX.pop_back();
  }
  while(!wallVectorY.empty())
  { wallVectorY.pop_back();
  }
  
  for(byte i = 0; i < 2*numWalls; i++)    //build some random walls
  {
    wallVectorX.push_back(random(rightEdge));
    wallVectorY.push_back(random(bottomEdge));
  }
}

void cast_rays()
{

}

void render_frame()
{

  frame_rate();

  display.drawFastVLine(rightEdge, topEdge, bottomEdge, GRAY_WHITE);
  
	tCurrentFrame = millis();
	frameTime = tCurrentFrame - tPrevFrame;
  for(int i = 0; i < 2*numWalls; i+=2)    //start point, end point, start point, end point, color (i think??)
  { display.drawLine(wallVectorX[i],wallVectorY[i],wallVectorX[i+1],wallVectorY[i+1], GRAY_WHITE);
  }
  display.drawLine(pX,pY,pX+5*cos(pHeading),pY+5*sin(pHeading), GRAY_3);
  display.drawPixel(pX,pY, GRAY_WHITE);
  display.display();
  display.clearDisplay();
  tPrevFrame = tCurrentFrame;
}

void frame_rate()
{
 
  if(++frameRefreshCount > 5)
  {
    frameRate = 1000/frameTime;
    frameRefreshCount = 0;
  }
  display.setTextSize(0);
  display.setCursor(0,0);
  display.print(frameRate);
}

void player_movement()
{
  ///first, let's figure out one-knob movement

  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(!digitalRead(encButtPin))
  {
    if(++pSpeed > maxSpeed)    //will this sneaky auto increment my variable? i think maybe
    { pSpeed = maxSpeed;
    }
    pVel += pSpeed / frameTime / blockSize;	//will this give me framerate compensation? framerate = 1/frame time, so.....
    if(pVel > maxVel)
    {  pVel = maxVel;
    }
    pVelX = cos(pHeading)*pSpeed;
    pVelY = sin(pHeading)*pSpeed;

    pX += pVelX;    //these two can be +='d above to save ops but this is for debugging help 
    pY += pVelY;
    flag.encButtFlag = 0;
  }
  else
  {
  	pSpeed -= deceleration / frameTime;	// tune decel for comfort
  }
  if(flag.CWflag)
  {
    pHeading += turnSpeed;  
    if(pHeading >= PI*2)
      pHeading = 0;
    flag.CWflag = 0;
  }
  if(flag.CCflag)
  {
    pHeading -= turnSpeed;  
    if(pHeading < 0)
      pHeading = PI*2;
    flag.CCflag = 0;
  }
  if(pX > rightEdge -1)
    pX = rightEdge -1;
  else if (pX <= leftEdge)
    pX = leftEdge+1;
  if(pY > bottomEdge -1)
    pY = bottomEdge -1;
  else if(pY <= topEdge)
    pY = topEdge+1;
  
}
