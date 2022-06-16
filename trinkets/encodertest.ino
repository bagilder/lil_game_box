///////////////////////////
// encoder debounce test for er-oledm032
// bgilder 29 may 2022
///////////////////////////

//weirdly enough, it seems to work best with a plain digital read scheme on any changing edge of encA
//and it is more responsive and less proportionally inaccurate without software debouncing.
//eventually, use a capacitor but like. dang. surprising. 



#include <gamebox.h>


#define SCREEN_ROTATION 0 //0=regular landscape, 1=box turn CC 90* portrait, 2=upside down, 3=box turn CW 90* portrait


void setup()   
{
  oled_setup(SCREEN_ROTATION);
  title_screen_encoder_test();
/*
  while(digitalRead(buttPin))
  {
    // kill time
  }
*/

  display.setTextWrap(true);
  //display.clearDisplay();
  display.setCursor(0,0);
  //display.print("hello");
  display.display();
}

void title_screen_encoder_test()
{
  display.print("press button to clear screen");
  display.display();
}


void loop()
{
  #ifdef ENCODERLIBRARY
  check_encoder();  //update rotational encoder flags
  #endif
  
  if(flag.CWflag)
  {
    display.print("CW ");
    display.display();
    flag.CWflag = 0;
  }

  if(flag.CCflag)
  {
    display.print("CC ");
    display.display();
    flag.CCflag = 0;
  }

      if(flag.encButtFlag)
  {
    display.print('#');
    display.display();
    //display.setCursor(0,0);
    flag.encButtFlag = 0;
  }

    if(flag.buttFlag)
  {
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    flag.buttFlag = 0;
  }

  if(flag.detentFlag)
  {
    display.print(",");
    display.display();
    flag.detentFlag = 0;
  }

}
