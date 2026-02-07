#include <SoftwareSerial.h>

#define TEST_STAND_HZ 50
#define LOOP_DELAY_MS (1000 / TEST_STAND_HZ)

// XBee: Arduino RX=D9 (from XBee DOUT), Arduino TX=D8 (to XBee DIN, level-shift!)
SoftwareSerial xbee(9, 8);

void setup() 
{
  Serial.begin(115200);   // USB to Mac (your Python script)
  xbee.begin(115200);       // MUCH more reliable on Uno SoftwareSerial than 115200

  Serial.println("XBee/USB Echo Ready");
}

void loop() 
{

  // ---- USB Serial (Mac) -> echo back to Mac ----
  while (xbee.available()) 
  {
    char c = (char)xbee.read();
    xbee.write(c);       // send back over wireless
  }

  delay(LOOP_DELAY_MS);
}
