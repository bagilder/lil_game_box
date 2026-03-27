//////////
// tunic translator
// bg
// 23-26 mar 2026
//////////


//future work: i need a "top R chicka" and a "bottom R chicka" option that modifies the neighboring segments. that will take a pretty significant edge case rewrite tho


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
aw 2,3,4          0b000000000001110x
hard 3,4          0b000000000001100x
hat 1,2,3,4       0b000000000001111x
shoo 1,2,3,4,5    0b000000000011111x
oh 1,2,3,4,5,6    0b000000000111111x
ee 2,3,4,5,6      0b000000000111110x
eh 3,4,5,6        0b000000000111100x
look 3,4,5        0b000000000011100x
ih 5,6            0b000000000110000x
oy 5              0b000000000010000x
ow 6              0b000000000100000x

*/

#include <gamebox.h>
//#define CHARACTERTESTMODE 1

//uint16_t charBuilder = 0;
uint8_t currentCharacter = 0;
uint16_t characters[8] = {0x5a00,0x5c1e,0xa200,0xd600,0x5404,0xea06,0xa200,0};  //"translation" for splashscreen
int nDownCounter = 0;
int nUpCounter = 0;
#define nDownThreshold 2
#define nUpThreshold 2

enum alphabet {BBB,DDD,FFF,GGG,HHH,JJJ,KKK,LLL,MMM,NNN,PPP,RRR,SSS,TTT,VVV,WWW,YYY,ZZZ,ch,sh,these,think,zh,ing,blank1,
                ay,uh,aw,hat,hard,ee,eh,look,eye,ih,shoo,oh,oy,ow,blank2,normal,invert};
alphabet alphaVar;
String characterList[42] = {"  B","  D","  F","  G","  H","  J","  K","  L","  M","  N","  P","  R","  S","  T","  V","  W","  Y","  Z",
                            " CH"," SH","(TH)ESE","(TH)INK"," ZH"," ING","[none]",
                            " AY"," UH"," AW","H(A)T","H(A)RD"," EE"," EH","L(OO)K"," EYE"," IH","SH(OO)"," OH"," OY"," OW","[none]","normal","invert"};

#define CONS_START BBB
#define CONS_END blank1
#define VOWEL_START ay
#define VOWEL_END blank2

void setup() 
{
  oled_setup(0);
  flag.buttFlag = 0;
  flag.resetFlag = 0;
  display.setTextColor(GRAY_WHITE, GRAY_BLACK);
  display.clearDisplay();
  for(currentCharacter = 0; currentCharacter<7; currentCharacter++)
  {
    draw_chars();
    display.fillRect(0, 56, SCREEN_WIDTH-1, 8,  GRAY_BLACK); //x0,y0,w,h,color 
    characters[currentCharacter] = 0;
  }
  currentCharacter = 0;
  display.setCursor(165, 56);
  display.print("bgilder 26mar26");
  display.display();
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
  }
  display.clearDisplay();
  display.display();
  flag.buttFlag = 0;
  flag.encButtFlag = 0;
  flag.CCflag = 0;
  flag.CWflag = 0;
}


void loop() 
{
  //shape_test_routine();
  //alphabet_test();
  alphaVar = normal;
  select_inversion();
  if(currentCharacter>0 && currentCharacter<8)
  {  ask_continue();
  }
}


void select_inversion()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
    if(flag.CWflag)
    {
      nDownCounter++;
      flag.CWflag = 0;
    }
    if(flag.CCflag)
    {
      nUpCounter++;
      flag.CCflag = 0;
    }
    if (nDownCounter > nDownThreshold+3)
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      characters[currentCharacter] = 0b0000000000000001; //yes inverted, vowel first
      alphaVar = invert;
    }
    else if(nUpCounter > nUpThreshold+3)
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      characters[currentCharacter] = 0;  //not inverted, cons first
      alphaVar = normal;
    }
    draw_chars();
  }
  flag.buttFlag = 0;

  if(characters[currentCharacter])  //inverted
  { select_vowel();
  }
  else
  { select_consonant();
  }
}


void select_consonant()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
    if(flag.CCflag)
    {
      nDownCounter++;
      flag.CCflag = 0;
    }
    if(flag.CWflag)
    {
      nUpCounter++;
      flag.CWflag = 0;
    }
    if (nUpCounter > nUpThreshold+3)  //boundaries, folks
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      alphaVar = alphaVar+1;
    }
    if(nDownCounter > nDownThreshold+3)  //healthy boundaries
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      if(alphaVar == CONS_START)
        alphaVar = CONS_END;
      else
        alphaVar = alphaVar-1;
    }
    if (alphaVar > CONS_END)  //boundaries, folks 
    { alphaVar = CONS_START;
    }
    if(alphaVar < CONS_START)  //healthy boundaries    //this Should be caught in the previous Else but jic
    { alphaVar = CONS_END;
    }

    switch(alphaVar)
    {
      case BBB:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001010001111111;
        break;

      case DDD:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1011010001111111;
        break;

      case FFF:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0111100001111111;
        break;

      case GGG:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101100001111111;
        break;

      case HHH:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1101010001111111;
        break;

      case JJJ:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0011010001111111;
        break;

      case KKK:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001110001111111;
        break;

      case LLL:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101010001111111;
        break;

      case MMM:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1010000001111111;
        break;

      case NNN:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1010001001111111;
        break;

      case PPP:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101100001111111;
        break;

      case RRR:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101110001111111;
        break;

      case SSS:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0111110001111111;
        break;

      case TTT:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101101000111111;
        break;

      case VVV:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1001011001111111;
        break;

      case WWW:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0000101001111111;
        break;

      case YYY:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0101011001111111;
        break;

      case ZZZ:
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
        
      case zh:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1011111001111111;
        break;

      case ing:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b1111111001111111;
        break;
      
      case blank1:
        characters[currentCharacter] |= 0b1111111110000000;
        characters[currentCharacter] &= 0b0000000001111111;
        break;
    }
    draw_chars();
  }
  flag.buttFlag = 0;
  if(characters[currentCharacter] & 0b0000000000000001) //if it's reversed, this will be second so let's move to the next position
  {
    display.fillRect((30*currentCharacter)+0, 56, SCREEN_WIDTH-1-(30*currentCharacter), 8,  GRAY_BLACK); //x0,y0,w,h,color
    if(characters[currentCharacter] < 2)  //if double null, erase the midline. multiple words~!
    {  display.fillRect((30*currentCharacter)+0, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
    }
    else if((characters[currentCharacter] > 255) && (characters[currentCharacter] & 0b0000000001111110)) //there's actually a consonant-vowel pair
    {  
      display.drawCircle((30*currentCharacter)+15, 51, 2, GRAY_WHITE); //x, y, r, color
      display.drawCircle((30*currentCharacter)+15, 51, 3, GRAY_WHITE); //x, y, r, color
    }
    display.display();
    currentCharacter++;
    if(currentCharacter > 7)
    {  end_character_row();
    }
  }
  else  //otherwise, this will be first
  { select_vowel();
  }
}


void select_vowel()
{
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif
    if(flag.CCflag)
    {
      nDownCounter++;
      flag.CCflag = 0;
    }
    if(flag.CWflag)
    {
      nUpCounter++;
      flag.CWflag = 0;
    }
    if (nUpCounter > nUpThreshold+3)  //boundaries, folks
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      alphaVar = alphaVar+1;
    }
    if(nDownCounter > nDownThreshold+3)  //healthy boundaries
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      alphaVar = alphaVar-1;
    }
    if (alphaVar > VOWEL_END)  //boundaries, folks     ////i'm sure we could integrate the switches and just keep track of which phoneme type we're on for choosing where our boundaries are
    { alphaVar = VOWEL_START;
    }
    if(alphaVar < VOWEL_START)  //healthy boundaries
    { alphaVar = VOWEL_END;
    }

    switch(alphaVar)
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
        
      case aw:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011101;
        break;
        
      case hat:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011111;
        break;

      case hard:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100011001;
        break;

      case shoo:
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
        
      case ih:
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

      case blank2:
        characters[currentCharacter] |= 0b0000000001111110;
        characters[currentCharacter] &= 0b1111111100000001;
        break;  
    }
    draw_chars();
  }
  flag.buttFlag = 0;
  if(characters[currentCharacter] & 0b0000000000000001) //if it's reversed, this will be first
  {  
    alphaVar = CONS_START;
    select_consonant();
  }
  else  //otherwise, this will be second so let's delete the english letters and move to the next position
  { 
    display.fillRect((30*currentCharacter)+0, 56, SCREEN_WIDTH-1-(30*currentCharacter), 8,  GRAY_BLACK); //x0,y0,w,h,color 
    if(characters[currentCharacter] < 2)  //if double null, erase the midline. multiple words~!
    {  display.fillRect((30*currentCharacter)+0, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
    }
    display.display();
    currentCharacter++;
  }
  if(currentCharacter > 7)
  {  end_character_row();
  }
}


void end_character_row()
{
    while(!flag.buttFlag)
    {
    }
    flag.buttFlag = 0;
    for(byte a = 0; a < 8; a++)
    {  characters[a] = 0;
    }
    display.clearDisplay();
    display.display();
    currentCharacter = 0;
}


void draw_chars()  //this feels pretty inelegant and brute force but let's do it 
{
  int charDisplayOffset = (30*currentCharacter);
  //delete current character's index bounding square. fresh slate, babyyy
  display.fillRect(charDisplayOffset+0, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
  display.drawLine(charDisplayOffset+0, 23, charDisplayOffset+30, 23, GRAY_WHITE);  //draw midline
  display.drawLine(charDisplayOffset+0, 24, charDisplayOffset+30, 24, GRAY_WHITE);
  display.fillRect(charDisplayOffset+0, 56, SCREEN_WIDTH-1-charDisplayOffset, 8,  GRAY_BLACK); //x0,y0,w,h,color    //just in cases
  display.setCursor(charDisplayOffset+0, 56); //type which state we're choosing
  if(alphaVar == think)
  {
    display.print("TH");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("ink");
  }
  else if (alphaVar == these)
  {
    display.print("TH");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("ese");
  }
  else if (alphaVar == hat)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("h");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("A");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("t");
  }
  else if (alphaVar == hard)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("h");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("A");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("rd");
  }
  else if (alphaVar == look)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("L");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("OO");
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("K");
  }
  else if (alphaVar == shoo)
  {
    display.setTextColor(GRAY_1, GRAY_BLACK);
    display.print("sh");
    display.setTextColor(GRAY_WHITE, GRAY_BLACK);
    display.print("OO");
  }
  else
  {  display.print(characterList[alphaVar]);
  }
  display.setTextColor(GRAY_WHITE, GRAY_BLACK);
  
  //actually draw the shapes dawg
  if(characters[currentCharacter] & 0b0000000000000010)
  {
    display.drawLine(charDisplayOffset+15, 0, charDisplayOffset+30, 9, GRAY_WHITE);  //draw segment 1
    display.drawLine(charDisplayOffset+15, 1, charDisplayOffset+30, 10, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000000100)
  { 
    display.drawLine(charDisplayOffset+0, 9, charDisplayOffset+15, 0, GRAY_WHITE);   //draw segment 2
    display.drawLine(charDisplayOffset+0, 10, charDisplayOffset+15, 1, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000001000)
  { 
    display.drawLine(charDisplayOffset+0, 9, charDisplayOffset+0, 23, GRAY_WHITE);   //draw segment 3
    display.drawLine(charDisplayOffset+1, 9, charDisplayOffset+1, 23, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000010000)
  { 
    display.drawLine(charDisplayOffset+0, 30, charDisplayOffset+0, 39, GRAY_WHITE);   //draw segment 4
    display.drawLine(charDisplayOffset+1, 30, charDisplayOffset+1, 39, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000000100000)
  { 
    display.drawLine(charDisplayOffset+0, 39, charDisplayOffset+15, 48, GRAY_WHITE);  //draw segment 5
    display.drawLine(charDisplayOffset+0, 40, charDisplayOffset+15, 49, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000000001000000)
  { 
    display.drawLine(charDisplayOffset+15, 48, charDisplayOffset+30, 39, GRAY_WHITE);  //draw segment 6
    display.drawLine(charDisplayOffset+15, 49, charDisplayOffset+30, 40, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000001000000000)
  { 
    display.drawLine(charDisplayOffset+0, 9, charDisplayOffset+15, 18, GRAY_WHITE);  //draw segment 9
    display.drawLine(charDisplayOffset+0, 10, charDisplayOffset+15, 19, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000010000000000)
  { 
    display.drawLine(charDisplayOffset+15, 0, charDisplayOffset+15, 18, GRAY_WHITE);  //draw segment 10
    display.drawLine(charDisplayOffset+16, 0, charDisplayOffset+16, 18, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0000100000000000)
  { 
    display.drawLine(charDisplayOffset+15, 18, charDisplayOffset+30, 9, GRAY_WHITE); //draw segment 11
    display.drawLine(charDisplayOffset+15, 19, charDisplayOffset+30, 10, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0001000000000000)
  { 
    display.drawLine(charDisplayOffset+15, 18, charDisplayOffset+15, 23, GRAY_WHITE); //draw segment 12
    display.drawLine(charDisplayOffset+16, 18, charDisplayOffset+16, 23, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0010000000000000)
  { 
    display.drawLine(charDisplayOffset+0, 39, charDisplayOffset+15, 30, GRAY_WHITE);  //draw segment 13
    display.drawLine(charDisplayOffset+0, 40, charDisplayOffset+15, 31, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b0100000000000000)
  { 
    display.drawLine(charDisplayOffset+15, 30, charDisplayOffset+15, 48, GRAY_WHITE); //draw segment 14
    display.drawLine(charDisplayOffset+16, 30, charDisplayOffset+16, 48, GRAY_WHITE);
  }
  if(characters[currentCharacter] & 0b1000000000000000)
  { 
    display.drawLine(charDisplayOffset+15, 30, charDisplayOffset+30, 39, GRAY_WHITE); //draw segment 15
    display.drawLine(charDisplayOffset+15, 31, charDisplayOffset+30, 40, GRAY_WHITE);
  }
  display.display();
}


void ask_continue()
{
  bool discontinuationate = 0;
  display.setCursor((30*currentCharacter)+2, 28);
  display.print("MORE?");
  display.setCursor((30*currentCharacter)+5, 38);
  display.print("yes");
  while(!flag.buttFlag)
  {
    #ifdef ENCODERLIBRARY
    check_encoder();  //update rotational encoder flags
    #endif

    if(flag.CCflag)
    {
      nDownCounter++;
      flag.CCflag = 0;
    }
    if(flag.CWflag)
    {
      nUpCounter++;
      flag.CWflag = 0;
    }

    if (nDownCounter > nDownThreshold+3)  //boundaries, folks
    { 
      nUpCounter = 0;
      nDownCounter = 0;  
      display.setCursor((30*currentCharacter)+5, 38);
      display.print("yes");
      discontinuationate = 0;
    }
    else if(nUpCounter > nUpThreshold+3)  //healthy boundaries
    { 
      nUpCounter = 0;
      nDownCounter = 0;
      display.setCursor((30*currentCharacter)+5, 38);
      display.print("no  ");
      discontinuationate = 1;
    }
    display.display();
  }
  flag.buttFlag = 0;
  if (discontinuationate)
  {
    display.fillRect((30*currentCharacter)+1, 0, 31, SCREEN_WIDTH-1,  GRAY_BLACK); //x0,y0,w,h,color
    display.display();
    end_character_row();
  }
}

#ifdef CHARACTERTESTMODE
void shape_test_routine()
{
  display.clearDisplay();
  currentCharacter = 0;

  for(int j = 0; j<16;j++)    //segment test
  {
    characters[0] = 0b0000000000000001<<j;
    draw_chars();
    delay(200);
  }

  for(int i = 0; i<8; i++)    //character test
  {
    currentCharacter = i;
    characters[i] = 0b1111111001111111;
    display.drawCircle((30*currentCharacter)+15, 54, 2, GRAY_WHITE); //x, y, r, color
    //display.drawCircle((30*currentCharacter)+15, 54, 3, GRAY_WHITE); //x, y, r, color
    draw_chars();
    delay(200);
  }

  delay(4000);
}


void alphabet_test()    //this requires killing everything after buttFlag = 0 in cons and vowel selec fxns
{
  if(alphaVar > CONS_END)
  {  select_vowel();
  }
  else
  {  select_consonant();
  }
  //display.clearDisplay();
  alphaVar = alphaVar + 1;
}
#endif
