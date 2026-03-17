#include <SoftwareSerial.h>
#include "HX711.h"
#include <SD.h>


// Timing Configs
#define TEST_STAND_HZ 50
#define LOOP_DELAY_MS (1000 / TEST_STAND_HZ)

#define MAX_ENGINE_TIME_SEC 30
#define MAX_ENGINE_TIME_MS  MAX_ENGINE_TIME_SEC * 1000
#define LOAD_CELL_HZ        80
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
const int HX711_DOUT = 2;
const int HX711_SCK  = 3;
const int HX711_RATE = 5;

// Calibration factor (use your own value after calibration).
// Increase to reduce reported weight; decrease to increase reported weight.
float calibration_factor = 51700.0;

// Mosfet switch pin
const uint8_t MOSFET_PIN = 7;

void xbee_init()
{
  xbee.begin(9600); 
  Serial.println("Xbee init completed");
}

String xbee_read()
{
  static String line = "";

  while (xbee.available())
  {
    char c = (char)xbee.read();

    if (c == '\r') continue;   // ignore CR

    if (c == '\n')
    {
      String out = line;
      line = "";
      out.trim();
      return out;
    }

    line += c;
  }

  return "";  // no complete line yet
}

void xbee_send(float value)
{
    xbee.print(value, 4);   // 4 digits after decimal
    xbee.print('\n');       // newline so receiver knows message ended
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
bool sd_write_float(const String& output_destination, float value)
{
  if (!sd_initialized)
  {
    Serial.println("SD not initialized!");
    return false;
  }

  File f = SD.open(output_destination, FILE_WRITE);
  if (!f)
  {
    Serial.println("Failed to open file!");
    return false;
  }

  f.println(value, 6);
  f.close();
  return true;
}

// =========================================================================
// Called from setup() — initializes the load cell interface
// =========================================================================
void load_cell_init() 
{
  Serial.println("Initializing Load Cell...");

  pinMode(HX711_RATE, OUTPUT);
  digitalWrite(HX711_RATE, HIGH);   // 80 Hz mode

  scale.begin(HX711_DOUT, HX711_SCK);
  scale.set_scale(calibration_factor);
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
    float weight = scale.get_units(1);

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
  xbee_init();
  sd_init();
  load_cell_init();
  Serial.println("Test Stand Init Complete");
}

void collect_engine_data(const String& test_name)
{
  Serial.print("Logging to: ");
  Serial.println(test_name);

  for (int time = 0; time < MAX_ENGINE_TIME_MS; time += LOAD_DELAY_MS)
  {
      float weight = load_cell_read();
      Serial.println("Read in weight from load cell:");
      Serial.println(weight);

      //sd_write_float(test_name, weight);
      Serial.println(time);
      Serial.println("Saved weight to SD card");

      xbee_send(weight);

      delay(12);   // about 83 Hz
   }
}

// Test State Machine
void start_engine_ignition(const String& test_name)
{
  Serial.println("Ignition Command Received");
  Serial.print("Test name: ");
  Serial.println(test_name);

  send_electric_sparker_command();

  collect_engine_data(test_name);

  Serial.println("Engine Firing Complete");
}


void setup() 
{
  test_stand_init();
}


void loop()
{
  String cmd = xbee_read();   // expects newline-terminated command

  if (cmd.length() > 0)
  {
    Serial.print("Received command: ");
    Serial.println(cmd);

    // You said: if it receives "st" first part => start ignition
    // Using "st_" based on your example "st_test1"
    if (cmd.startsWith("st_"))
    {
      String test_name = cmd.substring(3);  // everything after "st_"
      test_name.trim();
      test_name += ".txt";
      start_engine_ignition(test_name);
    }
  }

  delay(LOOP_DELAY_MS);
}
