#include <SoftwareSerial.h>


// RX, TX
SoftwareSerial HM10(2, 3);

#define TEST_STAND_HZ 50
#define LOOP_DELAY_MS (1000 / TEST_STAND_HZ) 


void setup() 
{
  Serial.begin(115200);   // USB serial for debugging
  HM10.begin(115200);       // HM-10 default baud rate

  Serial.println("HM-10 Echo Ready");
}


void loop() 
{
  
  // If HM-10 sends data → echo it back
  if (HM10.available()) 
  {
    char c = HM10.read();

    HM10.write(c);    // send back to Bluetooth device
    Serial.write(c);  // also show on Serial Monitor (optional)
  }

  delay(LOOP_DELAY_MS);

}
