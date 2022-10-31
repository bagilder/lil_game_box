

///////////////////////////
// fuck it let's try a raycasting engine
// bgilder
// 23 sept 2022, 19-30 oct 2022
///////////////////////////

////3d rendering using a square map with square tiles. each tile has 4 sides, all a unit wide
// different values can correspond to different wall properties
// uses digital differential analysis instead of ray marching or line intersection eqns
// leaning heavily on https://lodev.org/cgtutor/raycasting.html

////incorporates affine texture mapping. and holy shit it actuall works?????
// perspective gets wonky when you're right up on the walls. might want to look into.. non.. affine mapping



#include <gamebox.h>
#include <vector>
#include "imagedata.h"


#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait
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

/*
class List {    //atmel not using the std list library smDh
  public:
    byte length;
    sObject data[SPRITE_LIMIT];
    void append(sObject item) {
        if (length < SPRITE_LIMIT) data[length++] = item;
    }
    void remove(byte index) {
        if (index >= length) return;
        memmove(&data[index], &data[index+1], length - index - 1);
        length--;
    }
};

List objectList = 
{ .length = 3, .data ={
  {6.5, 8.5, 0, 0, false, 0, sprite[0]},
  {16.5, 16.5, 0, 0, false, 0, sprite[0]},
  {10.5, 3.5, 0, 0, false, 0, sprite[0]}}
};
*/


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


void setup()   
{
  oled_setup(SCREEN_ROTATION);    //gamebox

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
}

void loop()
{
  //delay(1000);

//  while(digitalRead(buttPin))  
//  {  
    //delay(10);   //take it easy, bud
    pHeadingX = cosf(pHeading);
    pHeadingY = sinf(pHeading);
    player_movement();
    render_frame();
//  }
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
    if(object2.objectType == 1 && mapArray[(int)object2.x][(int)object2.y] > 0)	//if a projectile and it hits a wall
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
