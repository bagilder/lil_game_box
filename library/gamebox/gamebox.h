//////////////////////////////
// convenient header since i'm doing all sorts of shit on the same board
// bgilder 29 may 2022
//////////////////////////////


//game box with:
// er-oledm032 3-bit-grayscale white-on-black 4-spi oled
// pec12r-3220f-s0024 quad incremental 24ppr mechanical rotary encoder
// a pushbutton
// teensy 4.0
// pc speaker


#ifndef GAMEBOX_H
#define GAMEBOX_H


#include <Arduino.h>
#include <avr/pgmspace.h>
//#include <Strings.h>
#include <SPI.h>
#include <SSD1322_for_Adafruit_GFX.h>
//#include <Bounce2.h>


//#define SOFTWAREBUTTON 			//we are software debouncing the pushbutton
//#define SOFTWAREENCBUTTON		//we are software debouncing the encoder button
//#define VOTINGENCODER			//we are using the voting scheme to check encoder rotation
#ifndef VOTINGENCODER
//#define SOFTWAREENCODER			//we are software debouncing encoder rotation. incompatible with voting scheme
#ifndef SOFTWAREENCODER
#define ENCODERLIBRARY
#include <Encoder.h>
#define ENCODER_USE_INTERRUPTS
#endif
#endif

#define SCREEN_WIDTH 256 // in pixels
#define SCREEN_HEIGHT 64 // in pixels
#define GRAY_BLACK 0x0
#define GRAY_1 0x1
#define GRAY_2 0x2
#define GRAY_3 0x3
#define GRAY_4 0x4
#define GRAY_5 0x5
#define GRAY_6 0x6
#define GRAY_7 0x8
#define GRAY_8 0xC
#define GRAY_WHITE 0xF

// use hardware SPI
/*#define OLED_DC     7   //modified pinout 
#define OLED_CS     9
#define OLED_RESET  8*/
#define OLED_RESET 8    //using original library pinout
#define OLED_DC  9
#define OLED_CS 10
extern Adafruit_SSD1322 display;


typedef struct 
{
 unsigned  										
  	buttFlag:1,     //pushbutton has been pressed
    encButtFlag:1,  //encoder button has been pressed
    encAstate:1,     //encoder output A (quad)
    encBstate:1,     //encoder output B (quad)
    CWflag:1,       //clockwise flag
    CCflag:1,       //counterclockwise flag
    detentFlag:1,   //encoder's natural rest spot. would be extra helpful if tracking encB interrupts
    resetFlag:1;   	//if the reset is successful
} flagfield8;

#define buttPin 2
#define encButtPin 3   
#define encApin 4
#define encBpin 5
#define soundPin 19       
/*
volatile bool bButtFlag = false;
volatile bool bEncButtFlag = false;
volatile bool bCWflag = false;
volatile bool bCCflag = false;
volatile bool bDetent = true;   //we are at the encoder's natural rest
volatile bool bDetentPrev = true; //historical
*/
extern volatile flagfield8 flag; //f,f,s,s,f,f,f,f	
#ifdef VOTINGENCODER
extern volatile int8_t nCWtick;   //trying a consensus filtering strat. we'll see how it goes ///lol not well, apparently
extern volatile int8_t nCCtick;
#endif


#define lResetStall 10000;  //ms. how long to wait before the reset button does something
extern volatile unsigned long lResetTimer;

#define lDebounceDelay 15	//ms.
#ifdef SOFTWAREENCODER
extern volatile unsigned long lcanonTimeA;
//extern volatile unsigned long lcanonTimeB;
#endif
#ifdef SOFTWAREBUTTON
extern volatile unsigned long lcanonTimeButt;
#endif
#ifdef SOFTWAREENCBUTTON
extern volatile unsigned long lcanonTimeEncButt;
#endif

void oled_setup(byte r);
void butt_isr();
void enc_butt_isr();
#ifndef ENCODERLIBRARY
void enc_a_isr();
void enc_a_isr2();
void enc_a_isr3();
//void enc_b_isr();
#else
void check_encoder();
#endif


#endif	//header gamebox
