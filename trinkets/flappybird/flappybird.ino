////////////////////
// do the dang game
// bgilder
// 2026-07-09
////////////////////

// add bottom pipes to the sprite list first so i can use if each%1 to filter heights

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
  byte objectType;	//0 for bird, 1 for pipe
  bool removed;
  byte *image;
  double rotationLast;
  double rotationNext;
  int spriteHeight;
  int spriteWidth;
};

byte flapSpriteCounter = 0;
unsigned long tFlapCounter = 0;
unsigned long tFlapNow = 0;
int flapCountInterval = 100; //ms, how long each flap sprite is on screen before switching to the next
int pipeGap = SCREEN_HEIGHT/4;	//will be how far apart the pipes are. might change with difficulty. should probably have a lower bound for playability
int pipeVx = 1;
int pipeSpeed = 1; 
int birdFlyDist = 5;	//this will be a gamefeel thing

/*sObject bird={
  SCREEN_WIDTH/3,
  SCREEN_HEIGHT/3,
  0,
  0,
  false,
  sprite,
  0,
  0};*/


std::vector<sObject> objectVector =       //bird in slot 0, pipes thereafter
{
  {SCREEN_WIDTH/3, SCREEN_HEIGHT/2, 0, 0, 0, false, 0, sprite[0], 10, 10}/*,  //x,y,vx,vy,removed,objectType,image,height,width. bird will not be sprite[0] if we use combined image file
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
      



    //this is likely where i would check if i've collided with pipes if i don't do it somewhere else




    bool conditionToRemovePipes;		//if they go off the screen. this can be combined with below but is broken out for clarity at the moment
    if(object2.x < (0 - (object2.spriteWidth/2)))	// scrolled off the left side of the screen, with extra room to avoid popout. unsure what negative x locations will do. might need to change to 0 and deal with popout
    {	conditionToRemovePipes = true;
	}

    if(object2.objectType && conditionToRemovePipes)	//if a pipe and should be gone
    {	object2.removed = true;		//requires garbage collection later. could probs handle that here and add an else? ///yes the else below is in anticipation of garbage collecting here 
    }
    else
	{
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
          if(!object2.removed)		//untested. might eff up some shiz. can remove if troublesome 
          {
          	byte texel = object2.image[(object2.spriteWidth * (int)(sSampleY*object2.spriteHeight) + (int)(sSampleX*object2.spriteWidth))]; //sampleX is percentage, need to scale it back up to full texture size
          	if(texel)
          	{	display.drawPixel((int)((sColumn-offsetX)*costheta-(sRow-offsetY)*sintheta)+offsetX,(int)((sColumn-offsetX)*sintheta+(sRow-offsetY)*costheta)+offsetY,texel);  
          	}
          /*  display.setCursor(0,0);
            display.println(sColumn);
            display.println(SCREEN_WIDTH-sY);
            display.print(texel);
            display.display();*/
          }
        }
      }
  	}
    objectVector[each].x = object2.x;
    objectVector[each].y = object2.y;
    objectVector[each].removed = object2.removed;		//still don't remember why we aren't just using the object vector explicitly
  }
	
}

animate_sprites()
{
  //byte flapSpriteList[4] = {sprite[6], sprite[7], sprite[8], sprite[7]};
  byte flapSpriteListNumerical[4] = {6,7,8,7};
  tFlapNow = millis();
  if((tFlapCounter + flapCountInterval <= tFlapNow) || (tFlapNow - tFlapCounter <= 0))		//should catch underflow
  {
  	flapSpriteCounter++%3;	//this is gonna give me an off-by-one i just know it
  	objectVector[0].image = sprite[flapSpriteListNumerical[flapSpriteCounter]];
  	tFlapCounter = millis();
  	pipeVx = (int) tFlapCounter/10000;	//change pipe speed every 10 sec. this isn't the way to do this but i'll fix it later
  }	

  //bird rotates



}



void loop()
{
	//check for input
	#ifdef ENCODERLIBRARY
  	check_encoder();  //update rotational encoder flags
  	#endif
	
  	if(flag.buttFlag)
  	{
  	    bird_fly();
  	    flag.buttFlag = 0;
  	}

	//calculate game state
	for(int each = 1; each < objectVector.size(); each++)
  	{
  		bool horizOverlap = 0;	//this is a naive approach. it might feel better gameplaywise if we just have a circle hitbox from the center of the bird sprite (esp with rotations)
  		if(objectVector[0].x + objectVector[0].spriteWidth/2 >= (objectVector[each].x - objectVector[each].spriteWidth/2))	//right of the left boundary
  		{	horizOverlap = (objectVector[0].x - objectVector[0].spriteWidth/2 <= (objectVector[each].x + objectVector[each].spriteWidth/2))?1:0;	//and left of the right boundary
  		}
  		bool vertOverlap = 0;
  		if(each%2)	//an odd numbered sprite (bottom pipe)
  		{
  			if(objectVector[0].y <= objectVector[each].y)
  			{	vertOverlap = 1;
  			}
  		}
  		else	//an even numbered sprite (top pipe)
  		{
  			if(objectVector[0].y >= objectVector[each].y)
  			{	vertOverlap = 1;
  			}
  		}

  		if(horizOverlap && vertOverlap)
  		{	game_over();
  		}

  		objectVector[each].x -= pipeVx;	//move the pipes left
  	}

	animate_sprites();
	draw_sprites();
}

void game_over()
{
	while(objectVector[0].x > 0)
	{
		;
		draw_sprites();
		objectVector[0].removed = true;
		//and then the garbage collection for removing sprites either here or generically where it happens afterwards
	}
	delay(100);
	//add game over sprite to end of sprite vector
	//
	sObject o;
  	o.x = SCREEN_HEIGHT/2;
  	o.y = SCREEN_WIDTH/2;
  	o.image = sprite[11];
  	o.removed = false;
  	o.objectType = 1;
  	objectVector.push_back(o);
	draw_sprites();
}


void add_pipes()
{
	int newPipeHeight = SCREEN_HEIGHT/3; 	//this will be dynamic eventually
	//randomize height of new pipes
	//might want to put some bounds eventually so it remains possible to play it as time goes on


	sObject newPipe1 = {SCREEN_WIDTH-1, newPipeHeight, 0, 0, 0, false, 1, sprite[9], 10, 10}  //x,y,vx,vy,removed,objectType,image,height,width.
	objectVector.push_back(newPipe1);	//bottom pipe
	sObject newPipe2 = {SCREEN_WIDTH-1, newPipeHeight+pipeGap, 0, 0, 0, false, 1, sprite[10], 10, 10}  //x,y,vx,vy,removed,objectType,image,height,width.
	objectVector.push_back(newPipe2);	//top pipe
}


void bird_fly()
{
	//bird goes up
	objectVector[0].rotationNext = PI/4;
	objectVector[0].vx += birdFlyDist;
	objectVector[0].image = sprite[6]; 	//the down-wing flap sprite
}