///////////////////////////
// 1-knob etch a sketch for er-oledm032
// shamelessly stolen from my and ruby's audrey2
// bgilder ported 13 july 2022
///////////////////////////


//dissolves on encoder press.
//changes directions on button hold. since we're double dipping on encoder reads, must write alternate direction position to encoder buffer on change
//added draw_line to fix draw stutter issue


#include <gamebox.h>

#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait


// THESE ARE THE CONSTANTS
const char holdDelay  = 75; //ms
const char randThreshold = 30;
const char eraseIteration = 10;   //number of times the randomized clearing function will run before blacking the screen
const long centerHoriz = SCREEN_WIDTH/2-1;
const long centerVert = SCREEN_HEIGHT/2-1;
const int vertOffset = SCREEN_HEIGHT-1;   //for vertical encoder behavior switching. change to 0 or SCREEN_HEIGHT-1 to reverse vertical movement behavior


// THESE ARE THE VARIABLES
bool dissolveState = true;
bool dissolveStatePrev = dissolveState;
bool holdState = false;
bool holdStatePrev = false;
long newVert, newHoriz;
long positionHoriz = centerHoriz;
long positionVert = centerVert;
unsigned long holdTime = 0;
long startLastVert = centerVert;   //for draw_line. trying to kill the line stutter effect.
long startLastHoriz = centerHoriz;

void setup() 
{
    oled_setup(SCREEN_ROTATION);
    display.clearDisplay();
    knob.write(-1*centerHoriz);  //listen idk. but it keeps it in bounds so i'm not gonna complain 
    //knob.write(centerVert);

    display.setCursor(0,9);
    display.println("etch a sketch!~\n");
    display.println("turn knob to move cursor");
    display.println("hold button to move vertically\n");
    display.println("press knob to erase. erase to begin!");
}


void loop() 
{

  if(flag.buttFlag)   //the button done gone been pressed
  {
    bool quickCheck = digitalRead(buttPin);
    if(quickCheck)
    {
      holdState = 0;    //oops not anymore
      flag.buttFlag = 0;
      knob.write(-1*positionHoriz);
    }
    else
    {
      if(holdStatePrev)     //we were holding the button and still are, so vertical mode
      {
        newVert = knob.read();  //up and down       //absolute
        
        if(newVert < 0)
        { 
          newVert = 0;
          knob.write(0);  //directly overwrites encoder position
        }
        else if(newVert > SCREEN_HEIGHT-1)
        { 
          newVert = SCREEN_HEIGHT-1;
          knob.write(SCREEN_HEIGHT-1);
        }
        if(positionVert != newVert)
        { 
          draw_point(GRAY_2);   //kills the white dot
          draw_line(newVert,GRAY_2,true);
          positionVert = newVert;
        } 
      }
      else
      {
        unsigned long currentNow = millis();
        if(currentNow >= holdTime + holdDelay)
        {
          holdTime = millis();
          holdState = true;    //this is some dumb shenanigans to make sure first entrance doesn't trigger a hold
          knob.write(positionVert);
        }
      }
    }
    holdStatePrev = holdState;   //yes, there has to be a better way to do this
  }
  else
  {
    newHoriz = knob.read();  //left and right  //absolute
    newHoriz *= -1;

    if(newHoriz < 0)
    { 
      newHoriz = 0;
      knob.write(0); //directly overwrites encoder position
    }
    else if(newHoriz > SCREEN_WIDTH-1)
    { 
      newHoriz = SCREEN_WIDTH-1;
      knob.write(-1*(SCREEN_WIDTH-1));
    }
    if(positionHoriz != newHoriz)
    { 
      draw_point(GRAY_3);   //kills the white dot     ///gray3 because of pwm latency on oled itself. horiz lines get darker the more the row is illuminated
      draw_line(newHoriz,GRAY_3,false);
      positionHoriz = newHoriz;
    }
  }

  draw_point(GRAY_WHITE);   //draw current cursor location

  dissolveState = digitalRead(encButtPin);
  if(!dissolveState && dissolveStatePrev)
  {
    delay(holdDelay);
    if(!digitalRead(encButtPin))
    { pixel_dissolve();
    }  
  }
  dissolveStatePrev = dissolveState;

}


void draw_point(char Color)
{
  display.drawPixel(positionHoriz,vertOffset-positionVert, Color);   //(height-1 - vert) reverses direction of movement on vertical knob spin. do what feels good
  display.display();
}


void draw_line(long stop, char Color, bool dir)    //0 = horiz, 1 = vert. this fixes stuttering issue with point draw scheme
{
  if(dir) // vertical
  {
    display.drawLine(positionHoriz, vertOffset-startLastVert, positionHoriz, vertOffset-stop, Color);   //start x, start y, end x, end y, color 
    startLastVert = stop;
  }
  else  // horizontal
  {
    display.drawLine(startLastHoriz, vertOffset-positionVert, stop, vertOffset-positionVert, Color);   //start x, start y, end x, end y, color  
    startLastHoriz = stop;
  }
  display.display();
}


void pixel_dissolve() 
{
  for(int iteration = 0; iteration < eraseIteration; iteration++)
  {
    for(int row = 0; row < SCREEN_HEIGHT; row++)
    {
      for(int col = 0; col < SCREEN_WIDTH; col++)
      {
        if(random(100) < randThreshold) 
        { display.drawPixel(col, row, GRAY_BLACK);
        }
      }
    }
    display.display();
  }
  display.clearDisplay();
  display.display();
}
