#include <magloop_tx.h>

//**************************************************************************************
//  Magnetic loop simple transmitter example.
//  The transmitter emmits a constant 5KHz signal that can be picked up by transmitters
//  placed inside the loop.
//**************************************************************************************

void setup()
{
  magLoopTX.begin();	//-Start transmitting a carrier
}

void loop()
{
}
