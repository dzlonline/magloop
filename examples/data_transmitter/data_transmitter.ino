#include <magloop_tx.h>

//**************************************************************************************
//  Magnetic loop data transmitter example.
//  The transmitter emmits a 5KHz signal that can be picked up by transmitters
//  placed inside the loop.
//  The signal is modulated with data from a text string.
//**************************************************************************************

void setup()
{
  magLoopTX.begin();
}

unsigned char text[]={
  "-=< Magnetic loop library Dzl 2014 >=-\r\n"};
unsigned char tptr=0;

void loop()
{
  if(magLoopTX.ready())
  {
    if(text[tptr])  {
      magLoopTX.write(text[tptr++]);
    }
    else
    {
      tptr=0;
      delay(1000);
    }
  }
}

