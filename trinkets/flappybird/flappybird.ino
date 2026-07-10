////////////////////
// do the dang game
// bgilder
// 2026-07-09
////////////////////


#include <gamebox.h>
#include <vector>
#include <avr/pgmspace.h>
#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait

struct sObject      //one lone coder sprite implementation
{
  float x;
  float y;
  float vx; //item velocity for movement
  float vy; 
  bool removed;
  byte *image;
  double rotationLast;
  double rotationNext;
  int spriteHeight;
  int spriteWidth;
};

sObject bird={
  SCREEN_WIDTH/3,
  SCREEN_HEIGHT/3,
  0,
  0,
  false,
  sprite,
  0,
  0};


std::vector<sObject> objectVector =       //bird in slot 0, pipes thereafter
{
  {SCREEN_WIDTH/3, SCREEN_HEIGHT/2, 0, 0, false, 0, sprite[0], 1, 1}/*,  //x,y,vx,vy,removed,objectType,image,height,width. bird will not be sprite[0] if we use combined image file
  {16.5, 16.5, 0, 0, false, 0, sprite[0]},
  {10.5, 3.5, 0, 0, false, 0, sprite[0]}*/
};


void draw_sprites()
{
  
    display.clearDisplay();



    //delay(1000);
     
  for(int each = 0; each < objectVector.size(); each++)  
  {
    sObject object2 = objectVector[each];		///is there a reason i'm not using the vector directly? i can't remember 
    display.drawPixel(object2.x,object2.y,GRAY_3);  //show object origin on screen 

    int viewHeight = SCREEN_HEIGHT-1;
	int sDist = 2; //scaling factor for our purposes today. 2 is normal. 1 is double size.  larger number is smaller sprite
 	//int spriteWidth = 64; //sprite width
 	//int spriteHeight = 64;

	float sHead = (viewHeight / 2) - viewHeight/sDist; //idr why the +1 on the walls but i'm putting it here just in cases (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistanceFromPlayer)
	float sToe = viewHeight-sHead;
	float sHeight = sToe - sHead;   //olc method
	//float sHeight =2*viewHeight/sDist;  //yonny zohar lecture method (approximately i guess)
	/*if(sHeight >= spriteHeight)
  		sHeight = spriteHeight-1;*/
	float getRatioed = object2.spriteHeight/object2.spriteWidth;  //for square sprites this will be 1. i don't remember why we do this. if it looks weird maybe let's just change back to straight height & width
	float sWidth = sHeight / getRatioed;    //for square sprites this will also be 1

    //update object physics
    object2.x += object2.vx;
    object2.y += object2.vy;
      
    ///this is likely where i would check if i've collided with pipes




      for(int sX = sWidth-1; sX >=0; sX--)   //because of the mirroring problem
      {
        for(int sY = sHeight-1; sY >= 0; sY--)   //draw sprite pixels    ///because of the mirroring problem
        {
          float sSampleX = sX/sWidth;   //normalized for screen space. a proportion i guess? 
          float sSampleY = sY/sHeight;
          int sColumn = (int)(object2.x + sX - (sWidth/2));
          int sRow = (int)(object2.y - sY + (sHeight/2));
          if(object2.rotationLast)
          {	
          	double theta = object2.rotationLast * PI/180;
          	float sintheta = sin(theta);  //save repeat operations
          	float costheta = cos(theta);
          }
          else
          {
          	float sintheta = 0;
          	float costheta = 1;
          }
          int offsetX = object2.x;	//this is for legibility in drawPixel but we can substitute it back to reclaim the extra 4 bytes if we want to
          int offsetY = object2.y;
          //if(sColumn>=0 && sColumn < SCREEN_WIDTH)
          //{
          byte texel = object2.image[(object2.spriteWidth * (int)(sSampleY*object2.spriteHeight) + (int)(sSampleX*object2.spriteWidth))]; //sampleX is percentage, need to scale it back up to full texture size
          if(texel)
          {	display.drawPixel((int)((sColumn-offsetX)*costheta-(sRow-offsetY)*sintheta)+offsetX,(int)((sColumn-offsetX)*sintheta+(sRow-offsetY)*costheta)+offsetY,texel);  
          }
          /*  display.setCursor(0,0);
            display.println(sColumn);
            display.println(SCREEN_WIDTH-sY);
            display.print(texel);
            display.display();*/
         // }
        }
      }
}

