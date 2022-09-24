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
const byte maxVel = 2;  //full run speed. we're just guessing to start
const float turnSpeed = PI/32;  //degrees. how far we tick in a direction on encoder blip. might have to change if converting everything to floats
const byte numWalls = 5;

float pHeading = 0;   //an angle, center of player's vield of view
float pVelX = 0;
float pVelY = 0;
float pSpeed = 0;   //player's movement i guess?
float pVel = 0;
float pX = SCREEN_WIDTH/2;
float pY = SCREEN_HEIGHT/2;

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
  while(digitalRead(buttPin))  
  {  delay(10);   //take it easy, bud
      player_movement();
        render_frame();
  }
  pX = SCREEN_WIDTH/2;
  pY = SCREEN_HEIGHT/2;
  

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
    wallVectorX.push_back(random(SCREEN_WIDTH));
    wallVectorY.push_back(random(SCREEN_HEIGHT));
  }
}

void cast_rays()
{

}

void render_frame()
{
  display.clearDisplay();
  for(int i = 0; i < 2*numWalls; i+=2)    //start point, end point, start point, end point, color (i think??)
  { display.drawLine(wallVectorX[i],wallVectorY[i],wallVectorX[i+1],wallVectorY[i+1], GRAY_WHITE);
  }
  display.drawLine(pX,pY,pX+5*cos(pHeading),pY+5*sin(pHeading), GRAY_3);
  display.drawPixel(pX,pY, GRAY_WHITE);
  display.display();
}

void player_movement()
{
  ///first, let's figure out one-knob movement

  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif

  if(flag.encButtFlag)
  {
    if(++pSpeed > 5)    //will this sneaky auto increment my variable? i think maybe
    { pSpeed = 5;
    }
    pVel += pSpeed;
    if(pVel > maxVel)
    {  pVel = maxVel;
    }
    pVelX = cos(pHeading)*pSpeed;
    pVelY = sin(pHeading)*pSpeed;

    pX += pVelX;    //these two can be +='d above to save ops but this is for debugging help 
    pY += pVelY;
    flag.encButtFlag = 0;
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
}
