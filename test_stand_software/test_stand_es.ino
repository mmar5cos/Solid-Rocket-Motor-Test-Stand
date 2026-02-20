#include <SoftwareSerial.h>
#include "HX711.h"
#include <SD.h>


// Timing Configs
#define TEST_STAND_HZ 50
#define LOOP_DELAY_MS (1000 / TEST_STAND_HZ)

#define MAX_ENGINE_TIME_SEC 10
#define MAX_ENGINE_TIME_MS  MAX_ENGINE_TIME_SEC * 1000
#define LOAD_CELL_HZ        10
#define LOAD_DELAY_MS       (1000 / LOAD_CELL_HZ)

// XBee Configs
SoftwareSerial xbee(9, 8);
static const char STAR_IGNITION_KEY = 'f';

// SD Card Configs
#define SD_CS_PIN 4

File sd_file;
bool sd_initialized = false;

// HX711 Load Cell Configs
// Create HX711 object
HX711 scale;

// Calibration factor (use your own value after calibration).
// Increase to reduce reported weight; decrease to increase reported weight.
float calibration_factor = 2200.0;

// Mosfet switch pin
const uint8_t MOSFET_PIN = 7;

void xbee_init()
{
  xbee.begin(115200); 
  Serial.println("Xbee init completed");
}

void xbee_read()
{
  
}

void xbee_send()
{
  
}

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
// example how to call: sd_write_float("thrust.txt", 12.345678);
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

// =========================================================================
// Called from setup() — initializes the load cell interface
// =========================================================================
void load_cell_init() 
{
  Serial.println("Initializing Load Cell...");

  // Initialize HX711 with data pin & clock pin
  scale.begin(A1, A0);

  // Set calibration factor (tune this for your specific load cell)
  scale.set_scale(calibration_factor);

  // Reset the scale to zero with no weight
  scale.tare();

  Serial.println("Load cell initialized");
}

// =========================================================================
// Reads the current value from the load cell.
// Returns the measurement (units based on calibration)
// =========================================================================
float load_cell_read() 
{
  // Check if HX711 is ready
  if (scale.is_ready()) 
  {
    // Get average of 10 readings
    float weight = scale.get_units(10);

    return weight;
  } 
  else 
  {
    Serial.println("HX711 not ready yet!");
    return NAN;
  }
}

void mosfet_init()
{
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);  // keep off to start
  Serial.println("Mosfer Pin Init Complete");
}

void send_electric_sparker_command()
{
  // Turn it ON
  digitalWrite(MOSFET_PIN, HIGH);
  delay(1000);

  // Turn it OFF
  digitalWrite(MOSFET_PIN, LOW);
  delay(1000);
  Serial.println("Electric Sparker Command Sent");
}

void test_stand_init()
{
  Serial.begin(115200); // USB to Mac 
  
  Serial.println("Starting Test Stand Init");
  //xbee_init();
  //sd_init();
  //load_cell_init();
  //mosfet_init();
  Serial.println("Test Stand Init Complete");
}

void collect_engine_data()
{
  for(int time=0; time<MAX_ENGINE_TIME_MS; time+=LOAD_DELAY_MS)
  {

    // read in load data
    float weight = (float)time;
    Serial.println("Read in weight from load cell:");
    Serial.println(weight);

    // save to SD card
    //sd_write_float("file_name_based_off_xbee_command", weight);
    Serial.println("Saved weight to SD card");
    
    // send back by xbee
    //send_xbee();
    Serial.println("Sent data through xbee to ground station");

    delay(LOAD_DELAY_MS);
  }
}

// Test State Machine
void start_engine_ignition()
{
  Serial.println("Ignition Command Recevied");

  send_electric_sparker_command();

  collect_engine_data();

  Serial.println("Engine Firing Complete");
}


void setup() 
{
  test_stand_init();
}


void loop() 
{
  char c = (char)Serial.read();   // change this read in xbee command -> xbee command will contain information the burn to save the data on the sd card with that name

  if (c == STAR_IGNITION_KEY) 
  {
      start_engine_ignition();
  } 
  else
  {
    Serial.println("Test Stand Waiting Command");
  }

  delay(LOOP_DELAY_MS);
}
