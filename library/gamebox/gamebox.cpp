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


#include <gamebox.h>


Adafruit_SSD1322 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);

volatile flagfield8 flag = {0,0,1,1,0,0,1,0}; 
volatile unsigned long lResetTimer = 0;
#ifdef SOFTWAREENCODER
volatile unsigned long lCanonTimeA = 0;
//volatile unsigned long lCanonTimeB = 0;
#endif
#ifdef SOFTWAREBUTTON
volatile unsigned long lCanonTimeButt = 0;
#endif
#ifdef SOFTWAREENCBUTTON
volatile unsigned long lCanonTimeEncButt = 0;
#endif
#ifdef VOTINGENCODER
volatile int8_t nCWtick = 0;   //trying a consensus filtering strat. we'll see how it goes
volatile int8_t nCCtick = 0;
#endif

#ifdef ENCODERLIBRARY
Encoder knob(encApin, encBpin);
#endif


void oled_setup(byte r)
{
  //Serial.begin(115200);
  pinMode(buttPin, INPUT_PULLUP);
  pinMode(encButtPin, INPUT_PULLUP);
  attachInterrupt(buttPin, butt_isr, FALLING);
  attachInterrupt(encButtPin, enc_butt_isr, FALLING);
  pinMode(encApin, INPUT_PULLUP);
  pinMode(encBpin, INPUT_PULLUP);
  #ifndef ENCODERLIBRARY
  attachInterrupt(encApin, enc_a_isr, CHANGE);
  //attachInterrupt(encBpin, enc_b_isr, RISING);
  #endif

  if ( ! display.begin(0x3D) ) 
  {
    // Serial.print("Unable to initialize OLED");
     while (1) yield();
  }
  display.cp437(true);  //proper extended character set
  display.setRotation(r);	//portrait
  display.clearDisplay();
  //display.display();
  display.setTextSize(1);
  display.setTextColor(GRAY_WHITE);
  display.setTextWrap(false);

  //lResetTimer = 0;

/*
#ifdef SOFTWAREENCODER
lCanonTimeA = 0;
//lCanonTimeB = 0;
#endif*/
#ifdef SOFTWAREBUTTON
lCanonTimeButt = 0;
#endif
#ifdef SOFTWAREENCBUTTON
lCanonTimeEncButt = 0;
#endif

#ifdef VOTINGENCODER
nCWtick = 0;   //trying a consensus filtering strat. we'll see how it goes
nCCtick = 0;
#endif


}


void butt_isr()
{  
	#ifdef SOFTWAREBUTTON
	unsigned long now = millis();
	if(now >= lDebounceDelay + lCanonTimeButt)	//if software debounce
	{
		lCanonTimeButt = now;
	#endif
		flag.buttFlag = 1;
	#ifdef SOFTWAREBUTTON
	}
	#endif
}

void enc_butt_isr()
{  
	#ifdef SOFTWAREENCBUTTON
	unsigned long now = millis();
	if(now >= lDebounceDelay + lCanonTimeEncButt)
	{
		lCanonTimeEncButt = now;
	#endif
		flag.encButtFlag = 1;
	#ifdef SOFTWAREENCBUTTON
	}
	#endif
}


#ifdef VOTINGENCODER
void enc_a_isr()
{
  flag.encAstate = digitalRead(encApin); //maybe these digital reads are slow enough to kill bounce idk
  flag.encBstate = digitalRead(encBpin); //
  flag.encAstate = digitalRead(encApin); //
  flag.encBstate = digitalRead(encBpin); //lol doubt it tho. i should just suck it up and use a capacitor

  if(flag.encAstate)   	//encA is high
  {
  	if(flag.encBstate)
  	{	nCCtick++;
  	}
  	else
  	{	nCWtick++;
  	}
    
    if(nCCtick > nCWtick)
    {
    	//CC wins the vote
    	flag.CWflag = 0;
    	flag.CCflag = 1;
    	
    	/////////
    	// program specific logic for CC rotation
    	// be sure your vars are volatile
		
    	/////////
    }
    else if(nCWtick >= nCCtick)
    {
    	//CW wins the vote
    	flag.CWflag = 1;
    	flag.CCflag = 0;
    	/////////
    	// program specific logic for CW rotation
    	// be sure your vars are volatile

    	/////////
    }
    nCCtick = 0;
    nCWtick = 0;
  	flag.detentFlag = 1;
  }
  else					//encA is low
  {
  	if(flag.encBstate)
  	{	nCWtick++;
  	}
  	else
  	{	nCCtick++;
  	}
  	flag.detentFlag = 0;
  }
}
#endif

#ifndef ENCODERLIBRARY	////if not encoder voting scheme or encoder library
void enc_a_isr()		//called on any changing edge
{
	bool state = digitalRead(encApin);
	#ifdef SOFTWAREENCODER
	unsigned long now = millis();
	if(now >= lDebounceDelay + lCanonTimeA) //if our transition was long enough ago to be real. it is now different! and we can believe it is the first edge of the transition!
	{
		lCanonTimeA = now;
	#endif
		flag.encAstate = state;
		flag.encBstate = digitalRead(encBpin);
		flag.detentFlag = state;	//if we're on a rising A edge in either direction, we're headed toward the detent, else no //except we miss that last CW pulse from b-rising hmm
	
		if(state)	//rising edge (encA high)
		{
			flag.CCflag = flag.encBstate;	//encB high (==), CC
			flag.CWflag = !flag.encBstate;	//encB low (!=), CW
		}
		else	//falling edge	(encA low)
		{
			flag.CWflag = flag.encBstate;	//encB high(!=), CW
			flag.CCflag = !flag.encBstate;	//encB low (==), CC
		}
	#ifdef SOFTWAREENCODER
	}
	#endif

	if(flag.CWflag)
	{
		/////////
    	// program specific logic for CW rotation
    	// be sure your vars are volatile
		
    	/////////

    	//flag.CWflag = 0;
	}
	else if(flag.CCflag)
	{
		/////////
    	// program specific logic for CC rotation
    	// be sure your vars are volatile

    	/////////

    	//flag.CCflag = 0;
	}
}

#ifdef ENCODERLIBRARY
void check_encoder()
{
	if(knob.read() > 0)
	{
		flag.CWflag = true;
		flag.CCflag = false;
		knob.write(0);
	}
	else if(knob.read() < 0)
	{
		flag.CWflag = false;
		flag.CCflag = true;	
		knob.write(0);
	}
}
#endif


/*
void enc_a_isr2()		//only called on rising edge
{
	bool state = digitalRead(encApin);
	#ifdef SOFTWAREENCODER
	unsigned long now = millis();
	if(now >= lDebounceDelay + lCanonTimeA) //if our transition was long enough ago to be real. it is now different! and we can believe it is the first edge of the transition!
	{
		lCanonTimeA = now;
	#endif
		flag.encAstate = state;
		flag.encBstate = digitalRead(encBpin);
		flag.detentFlag = flag.encBstate;	//rising A and B is high, detent from CC direction. else, no
	
		if(state)	//rising edge (encA high)
		{
			flag.CCflag = flag.encBstate;	//encB high (==), CC
			flag.CWflag = !flag.encBstate;	//encB low (!=), CW
		}
	#ifdef SOFTWAREENCODER
	}
	#endif
	*/
/*
	if(flag.CWflag)
	{
		/////////
    	// program specific logic for CW rotation
    	// be sure your vars are volatile
		
    	/////////

    	//flag.CWflag = 0;
	}
	else if(flag.CCflag)
	{
		/////////
    	// program specific logic for CC rotation
    	// be sure your vars are volatile

    	/////////

    	//flag.CCflag = 0;
	}
	
}
*/


/*
void enc_b_isr()		//only called on rising edge
{
	bool state = digitalRead(encBpin);
	#ifdef SOFTWAREENCODER
	unsigned long now = millis();
	if(now >= lDebounceDelay + lCanonTimeB) //if our transition was long enough ago to be real. it is now different! and we can believe it is the first edge of the transition!
	{
		lCanonTimeB = now;
	#endif
		flag.encBstate = state;
		flag.encAstate = digitalRead(encApin);
		flag.detentFlag = flag.encAstate;	//rising B and A is high, detent from CW direction. else, no.
	
		if(state)	//rising edge (encB high)
		{
			flag.CWflag = flag.encAstate;	//encA high (==), CW
			flag.CCflag = !flag.encAstate;	//encA low (!=), CC
		}
	#ifdef SOFTWAREENCODER
	}
	#endif

//	if(flag.CWflag)
//	{
//		/////////
//    	// program specific logic for CW rotation
//    	// be sure your vars are volatile
//		
//    	/////////
//
//    	//flag.CWflag = 0;
//	}
//	else if(flag.CCflag)
//	{
//		/////////
//    	// program specific logic for CC rotation
//    	// be sure your vars are volatile
//
//    	/////////
//
//    	//flag.CCflag = 0;
//	}
	
}
*/


/*
void enc_a_isr3()		//only on rising
{
	bool state = digitalRead(encApin);
	#ifdef SOFTWAREENCODER
	unsigned long now = millis();
	if(now >= lDebounceDelay + lCanonTimeA) //if our transition was long enough ago to be real. it is now different! and we can believe it is the first edge of the transition!
	{
		lCanonTimeA = now;
	#endif
		flag.encAstate = state;
		flag.encBstate = digitalRead(encBpin);
		flag.detentFlag = state;	//if we're on a rising A edge in either direction, we're headed toward the detent, else no //except we miss that last CW pulse from b-rising hmm
		flag.CCflag = flag.encBstate;	//encB high (==), CC
		flag.CWflag = !flag.encBstate;	//encB low (!=), CW

	#ifdef SOFTWAREENCODER
	}
	#endif
}
*/

#endif	//resolve which encA isr to use



