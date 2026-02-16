#include <SoftwareSerial.h>
#include <SD.h>

// Timing Configs
#define TEST_STAND_HZ 50
#define LOOP_DELAY_MS (1000 / TEST_STAND_HZ)

// XBee Configs
SoftwareSerial xbee(9, 8);

// SD Card Configs
#define SD_CS_PIN 4

File sd_file;
bool sd_initialized = false;


//
// Initialize SD card
//
bool sd_init()
{
    Serial.println("Initializing SD card...");

    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println("SD initialization failed!");
        sd_initialized = false;
        return false;
    }

    Serial.println("SD initialization successful.");
    sd_initialized = true;

    return true;
}

//
// Write float to file
//
bool sd_write_float(String output_destination, float value)
{
    if (!sd_initialized)
    {
        Serial.println("SD not initialized!");
        return false;
    }

    sd_file = SD.open(output_destination, FILE_WRITE);

    if (!sd_file)
    {
        Serial.println("Failed to open file!");
        return false;
    }

    sd_file.println(value, 6);  // 6 decimal precision
    sd_file.close();

    return true;
}


void setup() 
{
  Serial.begin(115200);   // USB to Mac (your Python script)
  //xbee.begin(115200);       // MUCH more reliable on Uno SoftwareSerial than 115200

  sd_init();

  sd_write_float("thrust.txt", 12.345678);
  sd_write_float("thrust.txt", 98.123456);
  Serial.println("SD Card Save Ready");
}


void loop() 
{

  // ---- USB Serial (Mac) -> echo back to Mac ----
  //while (xbee.available()) 
  //{
  //  char c = (char)xbee.read();
  //  xbee.write(c);       // send back over wireless
  //}

  delay(LOOP_DELAY_MS);
}
