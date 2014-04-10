#include <magloop_rx.h>

//**************************************************************************************
//  Magnetic loop signal detector.
//  The receiver picks up the 5KHz signal emmitted by the magnetic loop and displays the
//  relative signal level.
//  Use e.g. the simple_transmitter example for emitting the signal.
//**************************************************************************************

void setup()
{
  Serial.begin(19200);
  magLoopRX.begin();				//-Start receiver
}

void loop()
{
  Serial.print("Signal level: ");
  Serial.println(magLoopRX.getSignal());	//-Read signal
  delay(500);
}
