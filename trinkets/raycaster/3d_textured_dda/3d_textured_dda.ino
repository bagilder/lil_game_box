///////////////////////////
// fuck it let's try a raycasting engine
// bgilder
// 23 sept 2022, 19 oct 2022
///////////////////////////

////3d rendering using a square map with square tiles. each tile has 4 sides, all a unit wide
// different values can correspond to different wall properties
// uses digital differential analysis instead of ray marching or line intersection eqns
// leaning heavily on https://lodev.org/cgtutor/raycasting.html

////incorporates affine texture mapping. and holy shit it actuall works?????
// perspective gets wonky when you're right up on the walls. might want to look into.. non.. affine mapping


#include <gamebox.h>

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait

const byte FOVhalf = 35;  //half of the player's total field of view in degrees, to save a single devision lol will be heading+this and heading-this (edge wrapped)
const float maxVel = .025;  //full run speed. we're just guessing to start
const float turnSpeed = PI/32;  //degrees. how far we tick in a direction on encoder blip. might have to change if converting everything to floats
const byte numWalls = 5;
const float maxSpeed = .1;
const byte deceleration = 2*maxSpeed;
const byte blockSize = 1;
//const int rightEdge = (SCREEN_WIDTH-1)/ blockSize;
const int rightEdge = (SCREEN_WIDTH-1)/ 4;
const int leftEdge = 0;
const int bottomEdge = (SCREEN_HEIGHT-1)/ blockSize;
const int topEdge = 0;
const int viewWidth = SCREEN_WIDTH-1 - rightEdge;   //these will have to change if we swap window orientations
const int viewHeight = SCREEN_HEIGHT-1;// - bottomEdge;
const int drawDistance = (SCREEN_HEIGHT-1);
const int mazeWidth = 24;
const int mazeHeight = 24;
const int textureHeight = 64;
const int textureWidth = 64;
const int numTextures = 5; // the number of textures we have. duh

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

byte texture[numTextures][textureHeight*textureWidth];

int frameRate;
byte frameRefreshCount = 0;

float pHeadingX = -1;       //center of player's vield of view
float pHeadingY = 0;      //making this a vector instead of an angle to lean on https://lodev.org/cgtutor/raycasting.html
float pHeading = pHeadingY/pHeadingX; //is this right?? man i'm bad at geometry
float pVelX = 0;
float pVelY = 0;
float pSpeed = 0;   //player's movement i guess?
float pVel = 0;
float pX = leftEdge+2;   //player position in x
float pY = topEdge+2;  //player position in y
float cPlaneX = 0;        //camera plane vector in x
float cPlaneY = .66;     //ditto y
unsigned long tPrevFrame, tCurrentFrame, frameTime;


void setup()   
{
  oled_setup(SCREEN_ROTATION);    //gamebox

  display.print("raycasting demo on the game box\n\nbgilder 23 sept 2022, 19 oct 2022\n\n\nwhy on gods green earth did i decide to\nbuild this from scratch");
  display.display();

  while(digitalRead(buttPin))  
  {  delay(1);   //take it easy, bud
  }
  display.clearDisplay();
  display.display();

  for(int x=0;x<textureWidth;x++) //generate some textures
  {
    for(int y=0;y<textureHeight;y++)
    {
      texture[0][textureWidth*y+x] = (x != y && x != textureWidth - y) ? GRAY_3 : GRAY_6;  //diagonal crosses
      texture[1][textureWidth*y+x] = (x*y%14) + 2;  //fun circle gradient thing
      texture[2][textureWidth*y+x] = (y%14) + 2;   //horizontal stripes
      texture[3][textureWidth*y+x] = (x%4 && y%4) ? GRAY_3: GRAY_6; //vertical crosses //(x%14) + 2;  //vertical stripes 
    }
  }

  randomSeed(millis());
}

void loop()
{
  delay(1000);
  tCurrentFrame = millis();
  while(digitalRead(buttPin))  
  {  
    //delay(10);   //take it easy, bud
    pHeadingX = cos(pHeading);
    pHeadingY = sin(pHeading);
    player_movement();
    render_frame();
  }
  pX = leftEdge + 2;
  pY = topEdge + 2;
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
    float wallSpot; // figure out where exactly on the wall the ray hit, to figure out x-coord of texture map

    //Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
    if(side) 
    {  perpWallDist = (rayStepDistY - deltaDistY);
    }
    else          
    {  perpWallDist = (rayStepDistX - deltaDistX);
    }
    if(perpWallDist > drawDistance)
    {  perpWallDist = drawDistance;
    } 

    destinationX = pX + pHeadingX+rayDirX*perpWallDist;
    destinationY = pY + pHeadingY+rayDirY*perpWallDist;

    if(perpWallDist > 0 && perpWallDist <= drawDistance) //if we can see walls
    {  
      display.drawLine(pX,pY,destinationX,destinationY, GRAY_1);  //actually draw the ray
      
      //and now draw some "3d" walls?????? oh boy!
      int colHeight = viewHeight/perpWallDist;  //cos gives projection correction instead of euclidian distance. byebye fisheye

      if(colHeight > viewHeight)
      {  colHeight = viewHeight;  //seriously trippy glitches when we clip thru walls otherwise
      }
      if(colHeight < 0)
      {  colHeight = 0;
      }
      int drawEnd = viewHeight/2 + colHeight/2;
      int drawStart = viewHeight/2 - colHeight/2 + 1;


      
    //// textures!~
    int textureSpot;
    if(side)  //i super fucked the readability of these to try and maximize the number of things done in each check. could probably redo this more clearly
    {
      wallSpot = pX + perpWallDist * rayDirX;
      wallSpot -= floor(wallSpot);  //keep fractional portion
      if(rayDirY < 0) 
      {  textureSpot = textureWidth - int(wallSpot * float(textureWidth)) - 1;  //texture x coordinate
      }
      else
      {  textureSpot = int(wallSpot * float(textureWidth));
      }
    }
    else
    {
      wallSpot = pY + perpWallDist * rayDirY;
      wallSpot -= floor(wallSpot);  //keep fractional portion
      if(rayDirX > 0)
      {  textureSpot = textureWidth - int(wallSpot * float(textureWidth)) - 1;  //texture x coordinate
      }
      else
      {  textureSpot = int(wallSpot * float(textureWidth));
      }
    }


    byte textureNum = mapArray[pMapX][pMapY] - 1; //to 0 align with textures array
    //scale texture coordinate per screen pixel??
    float step = 1.0*textureHeight/colHeight;
    //starting texture coordinate
    float texturePos = (drawStart - viewHeight / 2 + colHeight / 2) * step;
    for(int y = drawStart; y<drawEnd; y++)
    {
      // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
      int texY = (int)texturePos & (textureHeight - 1);
      texturePos += step;
      byte tempColor = texture[textureNum][textureHeight * texY + textureSpot];
      display.drawPixel(rightEdge+xCols,y,tempColor);
    }

      // map the different distances onto different shades of gray
      //byte tempColor = map( colHeight, 0, viewHeight, 2,15);   //16 colors isn't as distinct, but it still looks a bit better than just my 8 //reserving ceiling & floor colors
      //byte tempColor = 3*mapArray[pMapX][pMapY];
            
      display.drawFastVLine(rightEdge+xCols, viewHeight/2 + colHeight/2 ,viewHeight/2 - colHeight/2 + 1, GRAY_1);  //floor different color than ceiling
      //display.drawFastVLine(rightEdge+xCols, viewHeight/2 - colHeight/2, colHeight, tempColor); //side? GRAY_3:GRAY_6); ///this will have to change if we add textures
    }
    else  //no walls in our view
    {
      display.drawFastVLine(rightEdge+xCols, viewHeight/2, viewHeight/2+1, GRAY_1);
      //display.drawPixel(rightEdge+xCols,viewHeight/2,GRAY_BLACK); //some vanishing point fakery
    }
  }
}
/*
void bresenham(int x0, int y0, int x1, int y1) 
{
  int dx = abs(x1-x0);
  int sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0);
  int sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2;
  int e2;

  for(;;){
    setPixel(x0,y0);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}*/


void render_frame()
{
  tCurrentFrame = millis();
  frameTime = tCurrentFrame - tPrevFrame;
  for(int row = 0; row < mazeHeight; row++)
  {
    for(int col = 0; col < mazeWidth; col++)
    {  display.drawPixel(row,col,mapArray[row][col]);
    }
  }
  display.drawLine(pX,pY,pX+3*cos(pHeading),pY+3*sin(pHeading), GRAY_3);  //shows player heading, relies on heading being an angle
  cast_rays();
  display.drawPixel(pX,pY, GRAY_WHITE);
  //display.drawFastVLine(rightEdge, topEdge, bottomEdge, GRAY_WHITE);  //view area boundary
  frame_rate();
  display.display();
  display.clearDisplay();
  tPrevFrame = tCurrentFrame;
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

    if(!mapArray[int(pX+pVelX)][int(pY)]) //if the space we're going to isn't occupied
    {  pX += pVelX;    //these two can be +='d above to save ops but this is for debugging help 
    }
    if(!mapArray[int(pX)][int(pY+pVelY)]) 
    {  pY += pVelY;
    }
    flag.encButtFlag = 0;
  }
  else
  {  pSpeed -= deceleration / frameTime; // tune decel for comfort
  }

  if(flag.CWflag)   //turn right
  {
    pHeading += turnSpeed;  
    if(pHeading >= PI*2)
    {  pHeading -= PI*2;
    }
    float oldPlaneX = cPlaneX;  //realign camera plane
    cPlaneX = cPlaneX * cos(turnSpeed) - cPlaneY * sin(turnSpeed);
    cPlaneY = oldPlaneX * sin(turnSpeed) + cPlaneY * cos(turnSpeed);
    flag.CWflag = 0;
  }

  if(flag.CCflag)   //turn left
  {
    pHeading -= turnSpeed;  
    if(pHeading < 0)
    {  pHeading += PI*2;
    }
    float oldPlaneX = cPlaneX;  //realign camera plane
    cPlaneX = cPlaneX * cos(-turnSpeed) - cPlaneY * sin(-turnSpeed);
    cPlaneY = oldPlaneX * sin(-turnSpeed) + cPlaneY * cos(-turnSpeed);
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
