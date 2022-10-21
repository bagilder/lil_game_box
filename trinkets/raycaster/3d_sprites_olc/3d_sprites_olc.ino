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
//#include <iostream>
#include "imagedata.h"


#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait


struct sObject      //one lone coder sprite implementation
{
  float x;
  float y;
  byte *image;
};

class List {    //atmel not using the std list library smDh
  public:
    byte length;
    sObject data[16];
    void append(sObject item) {
        if (length < 16) data[length++] = item;
    }
    void remove(byte index) {
        if (index >= length) return;
        memmove(&data[index], &data[index+1], length - index - 1);
        length--;
    }
};


List objectList = 
{ .length = 3, .data ={
  {6.5, 8.5, sprite[0]},
  {16.5, 16.5, sprite[0]},
  {10.5, 3.5, sprite[0]}}
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
const int drawDistance = (SCREEN_HEIGHT-1);
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

float pHeadingX = -1;       //center of player's vield of view
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

/*
  for(int x=0;x<textureWidth;x++) //generate some textures
  {
    for(int y=0;y<textureHeight;y++)
    {
     // texture[0][textureWidth*y+x] = (x != y && x != textureWidth - y) ? GRAY_3 : GRAY_6;  //diagonal crosses
     // texture[1][textureWidth*y+x] = (x*y%14) + 2;  //fun circle gradient thing
     // texture[2][textureWidth*y+x] = (y%14) + 2;   //horizontal stripes
     // texture[3][textureWidth*y+x] = (x%4 && y%4) ? GRAY_3: GRAY_6; //vertical crosses //(x%14) + 2;  //vertical stripes 
    }
  }
  */
  
  randomSeed(millis());
}

void loop()
{
  delay(1000);
  tCurrentFrame = millis();
  while(digitalRead(buttPin))  
  {  
    //delay(10);   //take it easy, bud
    pHeadingX = cosf(pHeading);
    pHeadingY = sinf(pHeading);
    player_movement();
    render_frame();
  }
  pX = leftEdge + 2;
  pY = topEdge + 2;
}

// #include "texturedwalls.h"

// #include "castrays.h"

// //#include "wolfdda.h"

// #include "castraysolcdda.h"

// #include "castraysolc.h"

// #include "drawsprites.h"


void cast_rays_olc()
{

  for (int x = 0; x <viewWidth; x++)
    {
  // For each column, calculate the projected ray angle into world space
      float fRayAngle = -((pHeading - FOVhalf ) + ((float)x/(float)viewWidth)*FOVhalf*2)+PI/2;
//      display.setCursor(0,0);
//    display.print(degrees(fRayAngle));
      // Find distance to wall
      float fStepSize = 0.01;    // Increment size for ray casting, decrease to increase 
      float fDistanceToWall = 0.0; //                                      resolution

      bool bHitWall = false;    // Set when ray hits wall block
      bool bBoundary = false;   // Set when ray hits boundary between two wall blocks

      float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
      float fEyeY = cosf(fRayAngle);

      float fSampleX = 0.0;

      bool bLit = false;
        float nTestX = 0;
        float nTestY = 12;
      // Incrementally cast ray from player, along ray angle, testing for 
      // intersection with a block
      while (!bHitWall && fDistanceToWall < drawDistance)
      {
        fDistanceToWall += fStepSize;
        nTestX = (int)(pX + fEyeX * fDistanceToWall);
        nTestY = (int)(pY + fEyeY * fDistanceToWall);

        // Test if ray is out of bounds
        if (nTestX < 0 || nTestX > mazeWidth || nTestY < 0 || nTestY > mazeHeight)
        {
          bHitWall = true;      // Just set distance to maximum depth
          fDistanceToWall = drawDistance;
        }
        else
        {
          // Ray is inbounds so test to see if the ray cell is a wall block
          if (mapArray[(int)nTestX][(int)nTestY] != 0)
          {
            // Ray has hit wall
            bHitWall = true;
                  display.drawLine(pX,pY,nTestX,nTestY, GRAY_1);  //actually draw the ray
//    display.setCursor(0,16);
//    display.println(viewHeight / (fDistanceToWall));
//    display.println(viewHeight);
//    display.println( (fDistanceToWall));
//    display.println(pHeadingY);
//    display.println(nTestY);
            // Determine where ray has hit wall. Break Block boundary
            // int 4 line segments
            float fBlockMidX = (float)nTestX + 0.5f;
            float fBlockMidY = (float)nTestY + 0.5f;

            float fTestPointX = pX + fEyeX * fDistanceToWall;
            float fTestPointY = pY + fEyeY * fDistanceToWall;
//display.setCursor(5*8,5*8);
//display.println(fTestPointX);

            float fTestAngle = atan2f((fTestPointY - fBlockMidY), (fTestPointX - fBlockMidX));

            if (fTestAngle >= -PI * 0.25f && fTestAngle < PI * 0.25f)
              fSampleX = fTestPointY - (float)nTestY;
            if (fTestAngle >= PI * 0.25f && fTestAngle < PI * 0.75f)
              fSampleX = fTestPointX - (float)nTestX;
            if (fTestAngle < -PI * 0.25f && fTestAngle >= -PI * 0.75f)
              fSampleX = fTestPointX - (float)nTestX;
            if (fTestAngle >= PI * 0.75f || fTestAngle < -PI * 0.75f)
              fSampleX = fTestPointY - (float)nTestY;
//display.print(fSampleX);
//display.display();
////delay(100);
//display.clearDisplay();
          }
        }
      }
       // fDistanceToWall = cos(fRayAngle-pHeading)*nTestX + sin(fRayAngle-pHeading)*nTestY;   //fisheye correction?
      // Calculate distance to ceiling and floor
      int nCeiling = (float)(viewHeight / 2.0) - viewHeight / ((float)fDistanceToWall);
      int nFloor = viewHeight-nCeiling;
      int nHeight = nCeiling - nFloor;

       
//int drawEnd = viewHeight/2 + viewHeight/perpWallDist/2;
//int drawStart = viewHeight/2 - viewHeight/perpWallDist/2;



    for (int y = viewHeight-1; y >=0; y--)
      {
    // Each Row
        if (y <= nCeiling)
          display.drawFastVLine(rightEdge+x, nCeiling ,1, GRAY_BLACK);
        else        if (y > nCeiling && y <= nFloor)
        {
          // Draw Wall
          if (fDistanceToWall < drawDistance)
          {
            byte textureNum = mapArray[(int)nTestX][(int)nTestY];
            float fSampleY = ((float)y - (float)nCeiling) / ((float)nFloor - (float)nCeiling);
                  byte tempColor = texture[textureNum][textureHeight * (int)(fSampleY*textureHeight) + (int)(fSampleX*textureWidth)];

              display.drawPixel(rightEdge+x, y, tempColor);    //draw the textured column to the screen buffer
          }
        }
        if(y>nFloor)
        {
          
      display.drawFastVLine(rightEdge+x, nFloor ,viewHeight-1, GRAY_1);  //floor different color than ceiling
        }
      }

//    display.setCursor(0,8);
//    display.print(viewHeight / ((float)fDistanceToWall));
// //   display.display();
//    //delay(100);
// //   display.clearDisplay();
      
    }
}



void draw_sprites()
{
   
  //is the object within the user's FOV?
  for(int each = 0; each < objectList.length; each++)  //for(auto &object : objectList)
  {
    sObject object = objectList.data[each];  //this is so dumb. why doesn't atmel support lists
    float sVecX = object.x - pX;
    float sVecY = object.y - pY;
    float sDist = sqrtf(sVecX*sVecX + sVecY*sVecY);  //pythagoras!
    
    //calculate angle between sprite and player
    float sAngle = -(atan2f(pHeadingY,pHeadingX) - atan2f(sVecY, sVecX));    //do we need to include the camera plane vectors here????   
    if(sAngle < -PI)        //this is currently 0->2pi, but his was -pi -> +pi. 
    {  sAngle += PI*2;
    }
    if(sAngle > PI)
    {  sAngle -= PI*2;
    }

    display.drawPixel(object.x,object.y,GRAY_3);  //show object on screen 
    
    bool canSee = fabs(sAngle) < FOVhalf;

      display.setCursor(4*8,(4+each)*8);
      display.print(degrees(sAngle));
      display.print(" ");
      display.print(canSee);
      display.setCursor(4*8,7*8);
      if(each==2)
      display.print(sDist);
          
    if(canSee && sDist>clippingDist && sDist < drawDistance)
    {

      float sHead = (viewHeight / 2) - viewHeight/sDist; //idr why the +1 on the walls but i'm putting it here just in cases (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistanceFromPlayer)
      float sToe = viewHeight-sHead;
      float sHeight = sToe - sHead;
      if(sHeight >= spriteHeight)
        sHeight = spriteHeight-1;
      float getRatioed = spriteHeight/spriteWidth;  //for square sprites this will be 1
      float sWidth = sHeight / getRatioed;    //for square sprites this will also be 1
      if(sWidth >= spriteWidth)
        sWidth = spriteWidth-1;
      float sPos = ((sAngle/FOVhalf)*0.5 + 0.5)*(float)viewWidth;   //center location of sprite //view width because we aren't ray tracing columns anymore?
      
      for(int sX = sWidth-1; sX >=0; sX--)   //because of the mirroring problem
      {
        for(int sY = sHeight-1; sY >= 0; sY--)   //draw sprite pixels    ///because of the mirroring problem
        {
          float sSampleX = sX/sWidth;   //normalized for screen space. a proportion i guess? but ours is based on an array so i'll have to change this. handy for png tho
          float sSampleY = sY/sHeight;
          int sColumn = (int)(sPos + sX - (sWidth/2));
          if(sColumn>=0 && sColumn<viewWidth)
          {
            byte texel = object.image[spriteWidth * (int)(sSampleY*spriteHeight) + (int)(sSampleX*spriteWidth)]; //sampleX is percentage, need to scale it back up to full texture size
            if(texel)
            { display.drawPixel(sColumn+rightEdge,sToe-sY,texel);
            }
          }
        }
      }
    }
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
  display.drawLine(pX,pY,pX+3*cosf(pHeading),pY+3*sinf(pHeading), GRAY_3);  //shows player heading, relies on heading being an angle
  cast_rays_olc();
  draw_sprites();
  display.drawPixel(pX,pY, GRAY_WHITE);
  //display.drawFastVLine(rightEdge, topEdge, bottomEdge, GRAY_WHITE);  //view area boundary
  frame_rate();     
  display.display();
  display.clearDisplay();
  tPrevFrame = tCurrentFrame;
}



