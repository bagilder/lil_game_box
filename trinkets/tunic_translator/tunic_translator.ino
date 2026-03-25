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

/* 
seg   start   stop    (note: all 10s changed to 9 and all 38s to 39 to get straight line continuations between characters)
1     15,0    30,9
2     0,9    15,0
3     0,9    0,24
4     0,30    0,39
5     0,39    15,48
6     15,48   30,39
mid   0,24    30,24
9     0,9     15,18
10    15,0    15,18
11    15,18   30,9
12    15,18   15,24
13    0,39    15,30
14    15,30   15,48
15    15,30   30,39


// i should include the option to 'add letter' each time so we can end words early ! note to self
// i need a "top R chicka" and a "bottom R chicka" option that modifies the neighboring segments also. note to self
// and a null for each option. pure vowel or pure consonant or neither. note to self.
// and if we're ambitious, we could have a double null delete the midline for a new word. note to self


/* let's do the dang alphabet gosh dang

reverse = xxxxxxxxxxxxxxx1

b 10,12,15                0b100101000000000x
d 10,12,13,15             0b101101000000000x
f 11,12,13,14             0b011110000000000x
g 11,12,14,15             0b110110000000000x
h 10,12,14,15             0b110101000000000x
j 10,12,13                0b001101000000000x
k 10,11,12,15             0b100111000000000x
L 10,12,14                0b010101000000000x
m 13,15                   0b101000000000000x
n 9,13,15                 0b101000100000000x
p 11,12,14                0b010110000000000x
r 10,11,12,14             0b010111000000000x
s 10,11,12,13,14          0b011111000000000x
t 9,11,12,14              0b010110100000000x
v 9,10,12,15              0b100101100000000x
w 9,11                    0b000010100000000x
y 9,10,12,14              0b010101100000000x
z 9,10,12,14,15           0b110101100000000x
ch 9,12,14                0b010100100000000x
sh 9,11,13,14,15          0b111010100000000x
th-ese 10,12,13,14,15     0b111101000000000x
th-ink 9,10,11,12,14      0b010111100000000x
ing 9,10,11,12,13,14,15   0b111111100000000x
zh 9,10,11,12,13,15       0b101111100000000x

eye 1             0b000000000000001x
ay  2             0b000000000000010x
uh  1,2           0b000000000000011x
claw 2,3,4        0b000000000001110x
hard 3,4          0b000000000001100x
hat 1,2,3,4       0b000000000001111x
who 1,2,3,4,5     0b000000000011111x
oh 1,2,3,4,5,6    0b000000000111111x
ee 2,3,4,5,6      0b000000000111110x
eh 3,4,5,6        0b000000000111100x
look 3,4,5        0b000000000011100x
it 5,6            0b000000000110000x
oy 5              0b000000000010000x
ow 6              0b000000000100000x

*/

#include <gamebox.h>


//uint16_t charBuilder = 0;
uint8_t currentCharacter = 0;
uint16_t characters[7] = {0,0,0,0,0,0,0};
int nDownCounter = 0;;
int nUpCounter = 0;
int nDownThreshold = 3;
int nUpThreshold = 3;

enum alphabet {b,d,f,g,h,j,k,L,m,n,p,r,s,t,v,w,y,z,ch,sh,these,think,ing,zh,
                eye,ay,uh,claw,hard,hat,who,oh,ee,eh,look,it,oy,ow,reverse};
alphabet consVar;
alphabet vowelVar;
String consonantList[24] = {"B","D","F","G","H","J","K","L","M","N","P","R","S","T","V","W","Y","Z","CH","SH","THese","THink","ING","ZH"};
String vowelList[14] = {"EYE","AY","UH","clAW","hArd","hAt","whO","OH","EE","EH","lOOk","It","OY","OW"};

void setup() 
{
  oled_setup(0);
  flag.buttFlag = 0;
  flag.resetFlag = 0;
  flag.encButtFlag = 0;
  flag.CCflag = 0;
  flag.CWflag = 0;
}

void loop() 
{

  display.clearDisplay();
  currentCharacter = 0;

  for(int j = 0; j<16;j++)    //segment test
  {
    characters[0] = 0b0000000000000001<<j;
    drawChars();
    delay(200);
  }

  for(int i = 0; i<8; i++)    //character test
  {
    currentCharacter = i;
    characters[i] = 0b1111111001111111;
    display.drawCircle((30*currentCharacter)+15, 54, 2, GRAY_WHITE); //x, y, r, color
    //display.drawCircle((30*currentCharacter)+15, 54, 3, GRAY_WHITE); //x, y, r, color
    drawChars();
    delay(200);
  }

  delay(4000);

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
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001010001111111;
        break;

      case d:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1011010001111111;
        break;

      case f:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0111100001111111;
        break;

      case g:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101100001111111;

        break;

      case h:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101010001111111;

        break;

      case j:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0011010001111111;

        break;

      case k:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001110001111111;

        break;

      case L:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101010001111111;

        break;

      case m:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1010000001111111;

        break;

      case n:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1010001001111111;

        break;

      case p:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101100001111111;

        break;

      case r:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101110001111111;

        break;

      case s:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0111110001111111;

        break;

      case t:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101101001111111;

        break;

      case v:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001011001111111;

        break;

      case w:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0000101001111111;

        break;

      case y:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101011001111111;

        break;

      case z:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101011001111111;

        break;

      case ch:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101001001111111;
      
        break;
        
      case sh:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1110101001111111;
      
        break;
        
      case these:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1111010001111111;
      
        break;
        
      case think:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101111001111111;
      
        break;
        
      case ing:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1111111001111111;
      
        break;
        
      case zh:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1011111001111111;
      
        break;

    }
    drawChars();
  }
  if(characters[currentCharacter] & 0b0000000000000001) //if it's reversed, this will be second so let's move to the next position
  {  currentCharacter++;
  }
  else  //otherwise, this will be first
  { select_vowel();
  }
  if(currentCharacter > 7)
  {
    display.clearDisplay();
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
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000011;

        break;
        
      case ay:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000101;
      
        break;
        
      case uh:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000111;
      
        break;
        
      case claw:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011101;
      
        break;
        
      case hard:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011001;
      
        break;
        
      case hat:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011111;
      
        break;
        
      case who:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100111111;
      
        break;
        
      case oh:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101111111;
      
        break;
        
      case ee:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101111101;
      
        break;
        
      case eh:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101111001;
      
        break;
        
      case look:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100111001;
      
        break;
        
      case it:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101100001;
      
        break;
        
      case oy:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100100001;
      
        break;
        
      case ow:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111101000001;
      
        break;
  
    }
    drawChars();
  }
  if(characters[currentCharacter] & 0b0000000000000001) //if it's reversed, this will be first
  {  select_consonant();
  }
  else  //otherwise, this will be second so let's move to the next position
  { currentCharacter++;
  }
  if(currentCharacter > 7)
  {
    display.clearDisplay();
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
    {  characters[currentCharacter] = 0b0000000000000001;
    }
  }
}

void drawChars()  //this feels pretty inelegant and brute force but let's do it 
{
  //delete current character's index bounding square. fresh slate, babyyy
  display.fillRect((30*currentCharacter)+0, 0, 31, 50,  GRAY_BLACK); //x0,y0,w,h,color
  display.drawLine((30*currentCharacter)+0, 23, (30*currentCharacter)+30, 23, GRAY_WHITE);  //draw midline
  display.drawLine((30*currentCharacter)+0, 24, (30*currentCharacter)+30, 24, GRAY_WHITE);
 /* if(characters[currentCharacter] & 0b0000000000000001)   //reverse circle         ---maybe this should be after we move on to the next char. on button press
  {
    //draw a circle under the current character's index
    display.drawCircle((15*currentCharacter)+15, 52, 2, GRAY_WHITE); //x, y, r, color
  }*/
  if(characters[currentCharacter] & 0b0000000000000010)
  {
    display.drawLine((30*currentCharacter)+15, 0, (30*currentCharacter)+30, 9, GRAY_WHITE);  //draw segment 1
    display.drawLine((30*currentCharacter)+15, 1, (30*currentCharacter)+30, 10, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000000100)
  { 
    display.drawLine((30*currentCharacter)+0, 9, (30*currentCharacter)+15, 0, GRAY_WHITE);   //draw segment 2
    display.drawLine((30*currentCharacter)+0, 10, (30*currentCharacter)+15, 1, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000001000)
  { 
    display.drawLine((30*currentCharacter)+0, 9, (30*currentCharacter)+0, 23, GRAY_WHITE);   //draw segment 3
    display.drawLine((30*currentCharacter)+1, 9, (30*currentCharacter)+1, 23, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000010000)
  { 
    display.drawLine((30*currentCharacter)+0, 30, (30*currentCharacter)+0, 39, GRAY_WHITE);   //draw segment 4
    display.drawLine((30*currentCharacter)+1, 30, (30*currentCharacter)+1, 39, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000100000)
  { 
    display.drawLine((30*currentCharacter)+0, 39, (30*currentCharacter)+15, 48, GRAY_WHITE);  //draw segment 5
    display.drawLine((30*currentCharacter)+0, 40, (30*currentCharacter)+15, 49, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000001000000)
  { 
    display.drawLine((30*currentCharacter)+15, 48, (30*currentCharacter)+30, 39, GRAY_WHITE);  //draw segment 6
    display.drawLine((30*currentCharacter)+15, 49, (30*currentCharacter)+30, 40, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000001000000000)
  { 
    display.drawLine((30*currentCharacter)+0, 9, (30*currentCharacter)+15, 18, GRAY_WHITE);  //draw segment 9
    display.drawLine((30*currentCharacter)+0, 10, (30*currentCharacter)+15, 19, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000010000000000)
  { 
    display.drawLine((30*currentCharacter)+15, 0, (30*currentCharacter)+15, 18, GRAY_WHITE);  //draw segment 10
    display.drawLine((30*currentCharacter)+16, 0, (30*currentCharacter)+16, 18, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000100000000000)
  { 
    display.drawLine((30*currentCharacter)+15, 18, (30*currentCharacter)+30, 9, GRAY_WHITE); //draw segment 11
    display.drawLine((30*currentCharacter)+15, 19, (30*currentCharacter)+30, 10, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0001000000000000)
  { 
    display.drawLine((30*currentCharacter)+15, 18, (30*currentCharacter)+15, 23, GRAY_WHITE); //draw segment 12
    display.drawLine((30*currentCharacter)+16, 18, (30*currentCharacter)+16, 23, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0010000000000000)
  { 
    display.drawLine((30*currentCharacter)+0, 39, (30*currentCharacter)+15, 30, GRAY_WHITE);  //draw segment 13
    display.drawLine((30*currentCharacter)+0, 40, (30*currentCharacter)+15, 31, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0100000000000000)
  { 
    display.drawLine((30*currentCharacter)+15, 30, (30*currentCharacter)+15, 48, GRAY_WHITE); //draw segment 14
    display.drawLine((30*currentCharacter)+16, 30, (30*currentCharacter)+16, 48, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b1000000000000000)
  { 
    display.drawLine((30*currentCharacter)+15, 30, (30*currentCharacter)+30, 39, GRAY_WHITE); //draw segment 15
    display.drawLine((30*currentCharacter)+15, 31, (30*currentCharacter)+30, 40, GRAY_WHITE);
  }
  
  display.display();

}
