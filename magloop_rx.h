#ifndef _MAGLOOPRX
#define _MAGLOOPRX

#ifdef _MAGLOOPTX
#error Cannot implement receiver and transmitter on same device! only include magloop_rx.h _or_ magloop_tx.h
#endif

//**************************************************************************************
//  5KHz ASK receiver for magnetic loop communications 
//  Runs on ATMEGA88, ATMEGA168 and ATMEGA328P based boards, possiply others too,
//  Dzl 2014
//**************************************************************************************

#include <avr/signal.h>
#include <math.h>

#define SET(x,y) (x |=(1<<y))
#define CLR(x,y) (x &= (~(1<<y)))
#define CHK(x,y) (x & (1<<y))
#define TOG(x,y) (x^=(1<<y))

//*****************************************************************************
//	Timer interrupt
//*****************************************************************************

class TMagloopRX
{
public:

  volatile float I;                        //-Inphase detector signal
  volatile float Q;                        //-Quadrature detector signal
  volatile float signal;                   //-Detector signal magnitude
  volatile float smax;                     //-Signal peak detector w. decay

  volatile unsigned char TFLAG;            //-Sync flag
  volatile unsigned char tstate;           //-Sampler sequencer
  volatile float II;                       //-Intermediate inphase signal
  volatile float QQ;                       //-Intermediate quadrature signal
  volatile float x0;                       //-Intermediate signal

  volatile unsigned char data_enabled;     //-Data decoder enabled flag
  volatile unsigned char tcnt;             //-Sample time divider for serial decoder
  volatile unsigned char rxstate;          //-Data decoder state
  volatile unsigned char data;             //-Data decoder shift register
  volatile unsigned char cnt;              //-Data decoder time divider
  volatile float thresh;                   //-Data slicer threshold
  volatile unsigned char data_ready;       //-Data decoded flag
  volatile unsigned char analog_channel;   //-Current analog channel

//---------------------------------------------------
//  Constructor
//---------------------------------------------------

  TMagloopRX()
  {
    TFLAG=0;
    tcnt=100;
    tstate=0;
    signal=0;
    x0=0;
    smax=0;
    data_enabled=0;
    rxstate=0;
    data=0;
    cnt=1;
    thresh=8.0;
    data_ready=0;
    analog_channel=1;
  }
//---------------------------------------------------
//  Set up
//---------------------------------------------------

  void begin()
  {
    data_enabled=false;
    ADMUX=0xC0|analog_channel;
    ADCSRA=0xA6;
    ADCSRB=0x05;
    TCCR1A=0x00;
    TCCR1B=0x09;
    TCCR1C=0;
    OCR1A=2403;
//    OCR1A=2404;
    OCR1B=1200;
    SET(TIMSK1,OCIE1A);
  }

//---------------------------------------------------
//  Read signal magnitude
//---------------------------------------------------

  float getSignal()
  {
    cli();
    float f=signal;
    sei();
    return f;
  }

//---------------------------------------------------
//  Read decayed peak signal
//---------------------------------------------------

  float getMax()
  {
    cli();
    float f=smax;
    sei();
    return f;
  }

//---------------------------------------------------
//  Synchronize to receiver
//---------------------------------------------------

  unsigned char sync()
  {
    if(TFLAG)
    {
      TFLAG=0;
      return 1;
    }
    return 0;
  }

//---------------------------------------------------
//  Set analog channel for next aqusition
//---------------------------------------------------

  void setChannel(unsigned char channel)
  {
    analog_channel=channel;
  }

//---------------------------------------------------
//  Check for decoded data
//---------------------------------------------------

  unsigned char available()
  {
    data_enabled=true;
    return data_ready;  
  }

//---------------------------------------------------
//  Read decoded data
//---------------------------------------------------
  
  unsigned char read()
  {

    data_ready=false;
    return data;
  }
};

//---------------------------------------------------
//  MagLoop instance
//---------------------------------------------------

TMagloopRX magLoopRX;

//---------------------------------------------------
//  Interrupt service routine
//---------------------------------------------------

SIGNAL(TIMER1_COMPA_vect)
{
  ADMUX=0xC0|magLoopRX.analog_channel;
  
  SET(TIFR1,OCF1B);			//-Retrigger ADC
  SET(TIFR1,OCF1A);			//-Retrigger INT

  switch(magLoopRX.tstate++)
  {
  case 0:
    magLoopRX.x0=sqrt(magLoopRX.II+magLoopRX.QQ);
    magLoopRX.II=(float)ADC;
    break;
  case 1:
    magLoopRX.QQ=(float)ADC;
    magLoopRX.signal=magLoopRX.signal*0.9+magLoopRX.x0*0.1;
    break;
  case 2:
    magLoopRX.I=magLoopRX.II-=(float)ADC;
    magLoopRX.II*=magLoopRX.II;
    break;
  case 3:
    magLoopRX.Q=magLoopRX.QQ-=(float)ADC;
    magLoopRX.QQ*=magLoopRX.QQ;
    magLoopRX.tstate=0;

    if(magLoopRX.tcnt)
      magLoopRX.tcnt--;
    else
    {
      magLoopRX.tcnt=3;
      if(magLoopRX.signal>magLoopRX.smax)
        magLoopRX.smax=magLoopRX.signal;
      magLoopRX.smax*=0.995;
      magLoopRX.TFLAG=1;			//-Measurement done

      if(magLoopRX.data_enabled)
      {
        switch(magLoopRX.rxstate)
        {
        case 0:
          magLoopRX.thresh=magLoopRX.smax/2;
          if(magLoopRX.signal<magLoopRX.thresh)
          {

            magLoopRX.cnt=14;
            magLoopRX.data=0;
            magLoopRX.rxstate=1;
            magLoopRX.data_ready=false;
          }
          break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
          if(magLoopRX.cnt--)
            break;
          else
          {
            magLoopRX.cnt=9;
            magLoopRX.rxstate++;
            magLoopRX.data>>=1;
            if(magLoopRX.signal>magLoopRX.thresh)
            {
              magLoopRX.data|=0x80;
            }
            if(magLoopRX.rxstate==9)
              magLoopRX.data_ready=true;
          }
          break;
        case 9:
          if(magLoopRX.cnt--)
            break;
          else
          {
            magLoopRX.cnt=150;
            magLoopRX.rxstate=10;
          }
          break;
        case 10:
          if(magLoopRX.signal>magLoopRX.thresh)
            magLoopRX.rxstate=0;

          if(magLoopRX.cnt)
            magLoopRX.cnt--;
          else
            magLoopRX.rxstate=0;
          break;
        }
      }
    }
    break;
  }
}
#endif



