#include <magloop_rx.h>

//**************************************************************************************
//  Magnetic loop data receiver example.
//  The receiver picks up the 5KHz signal emmitted by the magnetic loop and displays data
//  communication modulated onto the signal.
//**************************************************************************************

void setup()
{
  Serial.begin(19200);
  magLoopRX.begin();
}

void loop()
{
  if(magLoopRX.available())
    Serial.write(magLoopRX.read());
}
