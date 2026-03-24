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
j 10,12,13                b001101000000000x
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
it 5,6            b000000000110000x
oy 5              b000000000010000x
ow 6              b000000000100000x

*/

#include <gamebox.h>


//uint16_t charBuilder = 0;
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

    if (consVar > ow && nDownCounter > nDownThreshold+2)  //boundaries, folks
    { 
      nUpThreshold = 0;
      nDownThreshold = 0;
      consVar = eye;
    }

    if(consVar == b && nUpCounter > nUpThreshold+2)  //healthy boundaries
    { 
      nUpThreshold = 0;
      nDownThreshold = 0;
      consVar = zh;
    }

    switch(consVar)
    {
      case b:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1001010001111111;
        break;

      case d:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1011010001111111;
        break;

      case f:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0111100001111111;
        break;

      case g:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1101100001111111;

        break;

      case h:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1101010001111111;

        break;

      case j:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0011010001111111;

        break;

      case k:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1001110001111111;

        break;

      case L:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101010001111111;

        break;

      case m:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1010000001111111;

        break;

      case n:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1010001001111111;

        break;

      case p:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101100001111111;

        break;

      case r:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101110001111111;

        break;

      case s:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0111110001111111;

        break;

      case t:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101101001111111;

        break;

      case v:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1001011001111111;

        break;

      case w:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0000101001111111;

        break;

      case y:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101011001111111;

        break;

      case z:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1101011001111111;

        break;

      case ch:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101001001111111;
      
        break;
        
      case sh:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1110101001111111;
      
        break;
        
      case these:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1111010001111111;
      
        break;
        
      case think:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b0101111001111111;
      
        break;
        
      case ing:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1111111001111111;
      
        break;
        
      case zh:
        characters[currentCharacter] |= b1111111110000000;
        characters[currentCharacter] &= b1011111001111111;
      
        break;

    }
    drawChars();
  }
  if(characters[currentCharacter] & b0000000000000001) //if it's reversed, this will be second so let's move to the next position
  {  currentCharacter++;
  }
  else  //otherwise, this will be first
  { select_vowel();
  }
  if(currentCharacter > 7)
  {
    display.clear();
    currentCharacter = 0;
  }
}


void select_vowel()
{

  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif



    if (vowelVar > ow)  //boundaries, folks     ////i'm sure we could integrate the switches and just keep track of which phoneme type we're on for choosing where our boundaries are
    { vowelVar = eye;
    }

    if(vowelVar < eye)  //healthy boundaries
    { vowelVar = ow;
    }

    switch(vowelVar)
    {
      case eye:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100000011;

        break;
        
      case ay:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100000101;
      
        break;
        
      case uh:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100000111;
      
        break;
        
      case claw:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100011101;
      
        break;
        
      case hard:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100011001;
      
        break;
        
      case hat:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100011111;
      
        break;
        
      case who:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100111111;
      
        break;
        
      case oh:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111101111111;
      
        break;
        
      case ee:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111101111101;
      
        break;
        
      case eh:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111101111001;
      
        break;
        
      case look:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100111001;
      
        break;
        
      case it:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111101100001;
      
        break;
        
      case oy:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111100100001;
      
        break;
        
      case ow:
        characters[currentCharacter] |= b0000000001111110;
        characters[currentCharacter] &= b1111111101000001;
      
        break;
  
    }
    drawChars();
  }
  if(characters[currentCharacter] & b0000000000000001) //if it's reversed, this will be first
  {  select_consonant();
  }
  else  //otherwise, this will be second so let's move to the next position
  { currentCharacter++;
  }
  if(currentCharacter > 7)
  {
    display.clear();
    currentCharacter = 0;
  }
}

void selectReverse()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif

    if(characters[currentCharacter])
    {  characters[currentCharacter] = 0;
    }
    else
    {  characters[currentCharacter] = b0000000000000001;
    }

}

void drawChars()
{
  //delete current character's index bounding square. fresh slate, babyyy


  if(characters[currentCharacter] & b0000000000000001)   //reverse circle
  {
    //draw a circle under the current character's index

  }
  if(characters[currentCharacter] & b0000000000000010)
  {
    //draw segment 1

  }
  if(characters[currentCharacter] & b0000000000000100)
  {
    //draw segment 2

  }
  if(characters[currentCharacter] & b0000000000001000)
  {
    //draw segment 3

  }
  if(characters[currentCharacter] & b0000000000010000)
  {
    //draw segment 4

  }
  if(characters[currentCharacter] & b0000000000100000)
  {
    //draw segment 5

  }
  if(characters[currentCharacter] & b0000000001000000)
  {
    //draw segment 6

  }
  if(characters[currentCharacter] & b0000001000000000)
  {
    //draw segment 9

  }
  if(characters[currentCharacter] & b0000001000000000)
  {
    //draw segment 10

  }
  if(characters[currentCharacter] & b0000100000000000)
  {
    //draw segment 11

  }
  if(characters[currentCharacter] & b0001000000000000)
  {
    //draw segment 12

  }
  if(characters[currentCharacter] & b0010000000000000)
  {
    //draw segment 13

  }
  if(characters[currentCharacter] & b0100000000000000)
  {
    //draw segment 14

  }
  if(characters[currentCharacter] & b1000000000000000)
  {
    //draw segment 15

  }
  
  display.display();

}
