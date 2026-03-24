//////////
// tunic translator
// bg
// 23 mar 2026
//////////


/*********/
// we're gonna one-hot encode the segments for quick masking at the draw stage.
// start at outer segments, top right, then counter clockwise to bottom right. 
// move to inner segments, left to right, top to bottom. and trailing digit for reversed pronunciation?
//
//      /       \             2           1
//      | \ | /               3  9  10 11
//       ___|___                    12
//      | / | \               4  13 14 15
//      \       /             5           6
//          0                       0
//
/*********/


/* let's do the dang alphabet gosh dang

reverse = xxxxxxxxxxxxxxx1

b 10,12,15                b100101000000000x
d 10,12,13,15             b101101000000000x
f 11,12,13,14             b011110000000000x
g 11,12,14,15             b110110000000000x
h 10,12,14,15             b110101000000000x
j 10,12,14                b010101000000000x
k 10,11,12,15             b100111000000000x
L 10,12,14                b010101000000000x
m 13,15                   b101000000000000x
n 9,13,15                 b101000100000000x
p 11,12,14                b010110000000000x
r 10,11,12,14             b010111000000000x
s 10,11,12,13,14          b011111000000000x
t 9,11,12,14              b010110100000000x
v 9,10,12,15              b100101100000000x
w 9,11                    b000010100000000x
y 9,10,12,14              b010101100000000x
z 9,10,12,14,15           b110101100000000x
ch 9,12,14                b010100100000000x
sh 9,11,13,14,15          b111010100000000x
th-ese 10,12,13,14,15     b111101000000000x
th-ink 9,10,11,12,14      b010111100000000x
ing 9,10,11,12,13,14,15   b111111100000000x
zh 9,10,11,12,13,15       b101111100000000x

eye 1             b000000000000001x
ay  2             b000000000000010x
uh  1,2           b000000000000011x
claw 2,3,4        b000000000001110x
hard 3,4          b000000000001100x
hat 1,2,3,4       b000000000001111x
who 1,2,3,4,5     b000000000011111x
oh 1,2,3,4,5,6    b000000000111111x
ee 2,3,4,5,6      b000000000111110x
eh 3,4,5,6        b000000000111100x
look 3,4,5        b000000000011100x
it 5,6           b000000000110000x
oy 5              b000000000010000x
ow 6              b000000000100000x

*/

#include <gamebox.h>


uint16_t charBuilder = 0;
uint8_t currentCharacter = 0;
uint16_t characters[7] = 0;

enum alphabet {b,d,f,g,h,j,k,l,m,n,p,r,s,t,v,w,y,z,ch,sh,these,think,ing,zh,
                eye,ay,uh,claw,hard,hat,who,oh,ee,eh,look,it,oy,ow,reverse};
alphabet consVar;
alphabet vowelVar;

void setup() {
  oled_setup(0);
  flag.buttFlag = 0;
  flag.resetFlag = 0;
  flag.encButtFlag = 0;
  flag.CCflag = 0;
  flag.CWflag = 0;
}

void loop() {
  // put your main code here, to run repeatedly:



}

void select_consonant()
{

  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif



    switch(consVar)
    {
      case b:

        break;

      case d:

        break;

      case f:

        break;

      case g:

        break;

      case h:

        break;

      case j:

        break;

      case k:

        break;

      case L:

        break;

      case m:

        break;

      case n:

        break;

      case p:

        break;

      case r:

        break;

      case s:

        break;

      case t:

        break;

      case v:

        break;

      case w:

        break;

      case y:

        break;

      case z:

        break;

      case ch:
      
        break;
        
      case sh:
      
        break;
        
      case these:
      
        break;
        
      case think:
      
        break;
        
      case ing:
      
        break;
        
      case zh:
      
        break;

    }
    drawChars();
  }
}


void select_vowel()
{

  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif

    

    if (vowelVar > ow)  //boundaries, folks
    { vowelVar = eye;
    }

    if(vowelVar < eye)  //healthy boundaries
    { vowelVar = ow;
    }

    switch(vowelVar)
    {
      case eye:
      
        break;
        
      case ay:
      
        break;
        
      case uh:
      
        break;
        
      case claw:
      
        break;
        
      case hard:
      
        break;
        
      case hat:
      
        break;
        
      case who:
      
        break;
        
      case oh:
      
        break;
        
      case ee:
      
        break;
        
      case eh:
      
        break;
        
      case look:
      
        break;
        
      case it:
      
        break;
        
      case oy:
      
        break;
        
      case ow:
      
        break;
  
    }
    drawChars();
  }
}

void drawChars()
{
  //delete current character's index bounding square. fresh slate, babyyy


  if(charBuilder & b0000000000000001)   //reverse circle
  {
    //draw a circle under the current character's index

  }
  if(charBuilder & b0000000000000010)
  {
    //draw segment 1

  }
  if(charBuilder & b0000000000000100)
  {
    //draw segment 2

  }
  if(charBuilder & b0000000000001000)
  {
    //draw segment 3

  }
  if(charBuilder & b0000000000010000)
  {
    //draw segment 4

  }
  if(charBuilder & b0000000000100000)
  {
    //draw segment 5

  }
  if(charBuilder & b0000000001000000)
  {
    //draw segment 6

  }
  if(charBuilder & b0000001000000000)
  {
    //draw segment 9

  }
  if(charBuilder & b0000001000000000)
  {
    //draw segment 10

  }
  if(charBuilder & b0000100000000000)
  {
    //draw segment 11

  }
  if(charBuilder & b0001000000000000)
  {
    //draw segment 12

  }
  if(charBuilder & b0010000000000000)
  {
    //draw segment 13

  }
  if(charBuilder & b0100000000000000)
  {
    //draw segment 14

  }
  if(charBuilder & b1000000000000000)
  {
    //draw segment 15

  }
  
  display.display();

}
