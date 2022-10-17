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
const int viewWidth = SCREEN_WIDTH-1 - rightEdge;   //these will have to change if we swap window orientations
const int viewHeight = SCREEN_HEIGHT-1;// - bottomEdge;

float frameRate;
byte frameRefreshCount = 0;

float pHeadingX = -1;       //center of player's vield of view
float pHeadingY = 0;      //making this a vector instead of an angle to lean on https://lodev.org/cgtutor/raycasting.html
float pHeading = pHeadingY/pHeadingX; //is this right?? man i'm bad at geometry
float pVelX = 0;
float pVelY = 0;
float pSpeed = 0;   //player's movement i guess?
float pVel = 0;
float pX = rightEdge/2;   //player position in x
float pY = bottomEdge/2;  //player position in y
float cPlaneX = 0;        //camera plane vector in x
float cPlaneY = 0.66;     //ditto y
unsigned long tPrevFrame, tCurrentFrame, frameTime;

std::vector<int> wallVectorX;
std::vector<int> wallVectorY;

byte mapArray[viewWidth][viewHeight];  //this will eventually be used properly but right now i'm hacking it in so the random wall test can.. be.. tested


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
    pHeadingX = cos(pHeading);
    pHeadingY = sin(pHeading);
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

  //this is a hack to populate the map array
  for(int i = 0; i < 2*numWalls; i+=2)    //start point, end point, start point, end point, color (i think??)
  { display.drawLine(wallVectorX[i],wallVectorY[i],wallVectorX[i+1],wallVectorY[i+1], GRAY_WHITE);
  }

  for(int y =  topEdge; y<bottomEdge - topEdge; y++)   //trying to future proof if i move the window arrangement around
  {
    for(int x = leftEdge; x < rightEdge - leftEdge; x++)
    {  mapArray[x-leftEdge][y-topEdge] = (customGetPixel(x,y)!=0)?1:0;  //yes this sucks and yes i have to put my own dang getpixel function in but whatever
    }
  }
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
      //if the x component of the ray direction is negative
      if(rayDirX < 0) 
      {
        stepX = -1;   //we'll be checking negatively
        rayStepDistX = (pX - pMapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        rayStepDistX = (pMapX + 1.0 - pX) * deltaDistX;
      }
      //if the y component of the ray direction is negative
      if(rayDirY < 0) 
      {
        stepY = -1;   //we'll be checking negatively
        rayStepDistY = (pY - pMapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        rayStepDistY = (pMapY + 1.0 - pY) * deltaDistY;
      }

      /***and now we do the actual DDA***/

      float perpWallDist; //used to calculate the length of the array
      bool hit = 0; //was there a wall hit?
      bool side;    //was a NS or a EW wall hit? ;;x grid side = 0, y grid side = 1

      while(!hit)
      {
        //jump to next map square, either in x-direction, or in y-direction
        if(rayStepDistX < rayStepDistY)
        {
          rayStepDistX += deltaDistX;	//if x traversal is shorter, move in x direction
          pMapX += stepX;
          side = 0;
        }
        else
        {
          rayStepDistY += deltaDistY;	//if y traversal is shorter, move in y direction
          pMapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if(mapArray[pMapX][pMapY] > 0) hit = 1;
      }


      //Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
      if(side == 0) 
      {
      	perpWallDist = (rayStepDistX - deltaDistX);
       display.drawLine(pX,pY,rayStepDistX,rayStepDistY-deltaDistY, GRAY_1);
      }
      else          
      {
      	perpWallDist = (rayStepDistY - deltaDistY);
      }

      
      display.drawLine(pX,pY,pX+rayStepDistX,pY+rayStepDistY, GRAY_1);
      

  }
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
  //display.drawLine(pX,pY,pX+5*cos(pHeading),pY+5*sin(pHeading), GRAY_3);  //shows player heading, relies on heading being an angle
  cast_rays();
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
    pVel += pSpeed / frameTime / blockSize; //will this give me framerate compensation? framerate = 1/frame time, so.....
    if(pVel > maxVel)
    {  pVel = maxVel;
    }
    pVelX = pHeadingX*pSpeed;
    pVelY = pHeadingY*pSpeed;

    pX += pVelX;    //these two can be +='d above to save ops but this is for debugging help 
    pY += pVelY;
    flag.encButtFlag = 0;
  }
  else
  {
    pSpeed -= deceleration / frameTime; // tune decel for comfort
  }
  if(flag.CWflag)
  {
    pHeading += turnSpeed;  
    if(pHeading >= PI*2)
      pHeading = 0;
         	float oldPlaneX = cPlaneX;
      cPlaneX = cPlaneX * cos(-turnSpeed) - cPlaneY * sin(-turnSpeed);
      cPlaneY = oldPlaneX * sin(-turnSpeed) + cPlaneY * cos(-turnSpeed);
    flag.CWflag = 0;
  }
  if(flag.CCflag)
  {
    pHeading -= turnSpeed;  
    if(pHeading < 0)
      pHeading = PI*2;
    float oldPlaneX = cPlaneX;
      cPlaneX = cPlaneX * cos(turnSpeed) - cPlaneY * sin(turnSpeed);
      cPlaneY = oldPlaneX * sin(turnSpeed) + cPlaneY * cos(turnSpeed);
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
