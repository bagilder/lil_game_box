/////////////////////////////////
// choose your own adventure 
// dummy struct array for execution code testing
// bgilder 25 april 2022
/////////////////////////////////


#include <avr/pgmspace.h>


typedef struct Data{
  char narrativeString[5][8][41]; //MAX_CONTINUES,MAX_ROW_COUNT,MAX_CHAR_COUNT
  char choiceString[8][41];     //MAX_CHOICES
  uint8_t selectNum[8];        //MAX_CHOICES
  uint8_t soundtrack;           //sound selection index
} Data;


const Data theData[5] PROGMEM = {  //text file states total + 1 //state 0 will eventually be reserved
  
  {       //begin state 0
    {     //narrative string
      {   //screen 0
        "this is state 0 screen 0 line 1", "this is line two", "here is line 3", "4",
        "5", "here's 6", "and seven", "eight" 
      },
      {   //screen 1
        "this is state 0 screen 1 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 2
        "this is state 0 screen 2 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 3
        "this is state 0 screen 3 line 1", "this is line two", "here is line 3", "0123456789012345678901234567890123456789",
        "", "here's 6", "and seven", "eight"
      },  
      {   //screen 4
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789",
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789"
      },  
    },
    {     //choice string
      "here's your first choice", "here's your second choice", "here's choice 3", "here's four",
      "here's five", "here's six", "here's seven", "this is choice 8"
    },
    {     //select num
      1,2,3,4,1,2,3,4 
    },
    0 	//soundtrack
  },
  
  {       //begin state 1
    {     //narrative string
      {   //screen 0
        "this is state 1 screen 0 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight" 
      },
      {   //screen 1
        "this is state 1 screen 1 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 2
        "this is state 1 screen 2 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 3
        "this is state 1 screen 3 line 1", "this is line two", "here is line 3", "0123456789012345678901234567890123456789",
        "", "here's 6", "and seven", "eight"
      },  
      {   //screen 4
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789",
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789"
      },  
    },
    {     //choice string
      "here's your first choice", "here's your second choice", "here's choice 3", "here's four",
      "here's five", "here's six", "here's seven", "this is choice 8"
    },
    {     //select num
      0,2,3,4,0,2,3,4 
    },
    0 	//soundtrack
  },
  
  {       //begin state 2
    {     //narrative string
      {   //screen 0
        "this is state 2 screen 0 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight" 
      },
      {   //screen 1
        "this is state 2 screen 1 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 2
        "this is state 2 screen 2 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 3
        "this is state 2 screen 3 line 1", "this is line two", "here is line 3", "0123456789012345678901234567890123456789",
        "", "here's 6", "and seven", "eight"
      },  
      {   //screen 4
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789",
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789"
      },  
    },
    {     //choice string
      "here's your first choice", "here's your second choice", "here's choice 3", "here's four",
      "here's five", "here's six", "here's seven", "this is choice 8"
    },
    {     //select num
      0,1,3,4,0,1,3,4 
    },
    0 	//soundtrack
  },
  
  {       //begin state 3
    {     //narrative string
      {   //screen 0
        "this is state 3 screen 0 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight" 
      },
      {   //screen 1
        "this is state 3 screen 1 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 2
        "this is state 3 screen 2 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 3
        "this is state 3 screen 3 line 1", "this is line two", "here is line 3", "0123456789012345678901234567890123456789",
        "", "here's 6", "and seven", "eight"
      },  
      {   //screen 4
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789",
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789"
      },  
    },
    {     //choice string
      "here's your first choice", "here's your second choice", "here's choice 3", "here's four",
      "here's five", "here's six", "here's seven", "this is choice 8"
    },
    {     //select num
      0,1,2,4,NULL,NULL,NULL,NULL 
    },
    1 	//soundtrack
  },
  
  {       //begin state 4
    {     //narrative string
      {   //screen 0
        "this is state 4 screen 0 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight" 
      },
      {   //screen 1
        "this is state 4 screen 1 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 2
        "this is state 4 screen 2 line 1", "this is line two", "here is line 3", "",
        "", "here's 6", "and seven", "eight"
      },
      {   //screen 3'
        "this is state 4 screen 3 line 1", "\n", "\t", "\"\"",
        "", "\0", NULL, "eight"
      },  
      {   //screen 4
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789",
        "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789", "0123456789012345678901234567890123456789"
      },  
    },
    {     //choice string
      "here's your first choice", "here's your second choice", "here's choice 3", "here's four",
      "here's five", "here's six", "here's seven", "this is choice 8"
    },
    {     //select num
      0,1,2,3,0,1,2,3 
    },
    1 	//soundtrack
  }

};  //end struct array theData




/* ignore everything below. it is for a different feature
#define B0  31
#define C1  33
#define CS1 35
#define D1  37
#define DS1 39
#define E1  41
#define F1  44
#define FS1 46
#define G1  49
#define GS1 52
#define _A1  55
#define AS1 58
#define B1  62
#define C2  65
#define CS2 69
#define D2  73
#define DS2 78
#define E2  82
#define F2  87
#define FS2 93
#define G2  98
#define GS2 104
#define _A2  110
#define AS2 117
#define B2  123
#define C3  131
#define CS3 139
#define D3  147
#define DS3 156
#define E3  165
#define F3  175
#define FS3 185
#define G3  196
#define GS3 208
#define _A3  220
#define AS3 233
#define B3  247
#define C4  262
#define CS4 277
#define D4  294
#define DS4 311
#define E4  330
#define F4  349
#define FS4 370
#define G4  392
#define GS4 415
#define _A4  440
#define AS4 466
#define B4  494
#define C5  523
#define CS5 554
#define D5  587
#define DS5 622
#define E5  659
#define F5  698
#define FS5 740
#define G5  784
#define GS5 831
#define _A5  880
#define AS5 932
#define B5  988
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define E6  1319
#define F6  1397
#define FS6 1480
#define G6  1568
#define GS6 1661
#define _A6  1760
#define AS6 1865
#define B6  1976
#define C7  2093
#define CS7 2217
#define D7  2349
#define DS7 2489
#define E7  2637
#define F7  2794
#define FS7 2960
#define G7  3136
#define GS7 3322
#define _A7  3520
#define AS7 3729
#define B7  3951
#define C8  4186
#define CS8 4435
#define D8  4699
#define DS8 4978


const unsigned int sound0[] PROGMEM = {
  440,70,
  560,30,
  0,500,
  440,50,
  330,50,
  0,300,
  880,50,
  659,50,
  0,500,
  1000,50,
  880,100
  };

const unsigned int sound1[] PROGMEM = {
  560,100,
  880,70
};

const unsigned int sound2[] PROGMEM = {
  B4,1000, 
  AS4,500, 
  B4 ,500, 
  C5 ,250, 
  C5 ,500, 
  B4 ,250, 
  C5 ,1000,
  C5 ,250, 
  C5 ,500, 
  C5 ,250, 
  B4 ,500, 
  C5 ,500, 
  D5 ,250, 
  D5 ,500, 
  CS5,250, 
  D5 ,1000,
  E5 ,500, 
  G5 ,500, 
  FS5 ,500,
  E5 ,500, 
  D5 ,500, 
  B4 ,500, 
  G4 ,1000,
  B4 ,250, 
  _A4,500, 
  GS4,250, 
  B4 ,500, 
  _A4,500, 
  GS4,250, 
  _A4,500, 
  D5 ,1000,
  B4 ,250, 
  B4 ,500, 
  B4 ,250, 
  AS4,500, 
  B4 ,500, 
  C5 ,750, 
  B4 ,250, 
  C5 ,1000,
  C5 ,250, 
  C5 ,500, 
  C5 ,250, 
  B4 ,500, 
  C5 ,500, 
  D5 ,250, 
  D5 ,500, 
  CS5,250, 
  D5 ,1000,
  E5 ,500, 
  G5 ,500, 
  FS5 ,500,
  E5 ,500, 
  D5 ,500, 
  B4 ,500, 
  G4 ,500, 
  _A4,500, 
  B4 ,250, 
  D5 ,500, 
  C5 ,250, 
  B4 ,500, 
  _A4,500, 
  G4,1000  
};

const unsigned int sounds[] PROGMEM = {
  sound0, sound1,sound2
};

const unsigned int sound0Length = sizeof(sound0)/sizeof(unsigned int);
const unsigned int sound1Length = sizeof(sound1)/sizeof(unsigned int);
const unsigned int sound2Length = sizeof(sound2)/sizeof(unsigned int);
const unsigned int soundLengths[] PROGMEM = {
  sound0Length, sound1Length, sound2Length
};

bool donezo = false;	//for sound testing
*/
