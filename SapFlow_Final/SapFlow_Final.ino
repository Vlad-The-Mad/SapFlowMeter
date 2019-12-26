///////////////////////////////////////////////////////////////////////////////

// This program samples lots of temperature data from a tree to measure the sap flow.
///////////////////////////////////////////////////////////////////////////////

#define ALARM_PIN 12
#define HEATER 11

#define DEBUG 1

#include <OPEnS_RTC.h>
#include <LowPower.h>
#include <sdios.h> //for ArduinoOutStream

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#include <Adafruit_MAX31865.h>

Adafruit_MAX31865 upper_rtd = Adafruit_MAX31865(A5);
Adafruit_MAX31865 lower_rtd = Adafruit_MAX31865(A4);
Adafruit_MAX31865 heater_rtd = Adafruit_MAX31865(A3);

#define Rnom 100
#define Rref 430


ArduinoOutStream cout(Serial);

// Instance of our RTC
RTC_DS3231 rtc_ds;

// Time used for state transitions
volatile uint32_t event_time;

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 10;

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 1000;

SdFat sd; // File system object.

/* Checks if the filename exists.
* If so, appends an integer.
* Returns the new filename it came up with.*/
String newfile( String fname, String suffix=".csv" ){
  String temp = fname + suffix;
  int i = 0;
  while(sd.exists(temp.c_str())){
    ++i;
    char ones = i%10 + '0';
    char tens = i/10 + '0';
    temp = fname + '(' + tens + ones + ')' + suffix;
  }
  return temp;
}

// Stores samples and relative time.
// Adapt this datatype to your measurement needs.
class sample{
  public:
  float upper, lower, heater;
  DateTime t;
  
  sample( const sample &a ){
    upper = a.upper;
    lower = a.lower;
    heater = a.heater;
    t = a.t;
  }
  sample( Adafruit_MAX31865 &rtd1, Adafruit_MAX31865 &rtd2){
    t = rtc_ds.now();
    upper = rtd1.temperature(Rnom, Rref);
    lower = rtd2.temperature(Rnom, Rref);
    heater = 0;
  }
  sample( Adafruit_MAX31865 &rtd1, Adafruit_MAX31865 &rtd2,
          Adafruit_MAX31865 &rtd3){
    t = rtc_ds.now();
    upper = rtd1.temperature(Rnom, Rref);
    lower = rtd2.temperature(Rnom, Rref);
    heater = rtd3.temperature(Rnom, Rref);
  }
  sample( size_t oversample ){
    t = rtc_ds.now();
    upper = 0;
    lower = 0;
    for( auto i = 0; i < oversample; ++i ){
      upper += analogRead(A0);
      lower += analogRead(A1);
    }
  }
  sample( void ){
    sample( 1 );
  }
  void print( ostream &stream){
    stream << t.text() << ", ";
    stream << setw(6) << upper << ", ";
    stream << setw(6) << lower << ", ";
    stream << setw(6) << heater << "\r\n";
  }
  // Print the header for a csv
  void header( ofstream &stream ){
    stream << setw(6) << "Upper Probe" << ',';
    stream << setw(6) << "Lower Probe" << ',';
    stream << setw(6) << "Heater Probe" << '\n';
  }
};

class datastream{
  private:
  String fname;
  ofstream sdout;
  bool file_open;

  public:
  datastream( String filename ){
    fname = filename;
  }
  void flush( void ){
    sdout.close();
    file_open = false;
  }
  void append( sample p ){
    if( !file_open ){
      sdout = ofstream( fname.c_str(), ios::out | ios::app );
      file_open = true;
    }
    p.print(sdout);
  }
};

//------------------------------------------------------------------------------

// Data storage object
datastream d("temperature_log.csv");

// Filename for calculated sap flow
String filename;
// File handle for the file
ofstream sapfile;

enum state{
  wake,
  heating,
  cooling,
  sleep
} measuring_state;

void alarmISR() {
  // Reset the alarm.
  rtc_ds.armAlarm(1, false);

  // Disable this interrupt
  detachInterrupt(digitalPinToInterrupt(ALARM_PIN));
  event_time = millis();
}

void feather_sleep( void ){
  while(!digitalRead(ALARM_PIN)){
    Serial.print("Waiting on alarm pin...");
    delay(10);
  }
  // Low-level so we can wake from sleep
  // I think calling this twice clears the interrupt.
  attachInterrupt(digitalPinToInterrupt(ALARM_PIN), alarmISR, LOW);
  attachInterrupt(digitalPinToInterrupt(ALARM_PIN), alarmISR, LOW);
  // Prep for sleep
  Serial.end();
  USBDevice.detach();
  // Disable SPI to save power
  pinMode(24, INPUT);
  pinMode(23, INPUT);
  pinMode(chipSelect, INPUT);
  // Turn off power rails
  digitalWrite(5, HIGH); digitalWrite(6, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  // Sleep
  LowPower.standby();

  // Prep to resume
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(5, LOW); digitalWrite(6, HIGH);
  pinMode(24, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  USBDevice.attach();
  Serial.begin(115200);
  sd.begin(chipSelect, SD_SCK_MHZ(1));
}

// Sleep until the time is a round multiple of the minute inteval.
// Produces unexpected bevahior for non-factors of 60 (7, 8, 9, 11, etc)
void sleep_cycle( int interval = 5 ){
  if( DEBUG == 1 ){
     Serial.println("Skipping sleep");
  } else {
    Serial.println("Sleeping until nearest multiple of 5 minutes");
    DateTime t = rtc_ds.now();
    t = t + TimeSpan( interval * 60 );
    uint8_t minutes = interval*(t.minute()/interval);
    rtc_ds.setAlarm(ALM2_MATCH_MINUTES, minutes, 0, 0);
    Serial.print("Alarm set to ");
    t = rtc_ds.getAlarm(2);
    Serial.println(t.text());
    delay(1000);
    feather_sleep();
  }
}

void setup() 
{
  // Enable outputs to control 5V and 3.3V rails
  pinMode(5, OUTPUT); pinMode(6, OUTPUT);
  digitalWrite(5, LOW); digitalWrite(6, HIGH);
  pinMode(HEATER, OUTPUT);
  pinMode(ALARM_PIN, INPUT_PULLUP);
  digitalWrite(HEATER, LOW);
  measuring_state = wake;
  event_time = millis();
  
  Serial.begin(115200);
  Serial.println("Starting setup");

  // RTC Timer settings here
  if (! rtc_ds.begin()) {
    Serial.println("Couldn't find RTC");
  }
  // This may end up causing a problem in practice - what if RTC loses power in field? Shouldn't happen with coin cell batt backup
  if (rtc_ds.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // Set the RTC to the date & time this sketch was compiled
    rtc_ds.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  Serial.println("Initializing SD Card");
  if (!sd.begin(chipSelect, SD_SCK_MHZ(1))) {
    sd.initErrorHalt();
  }
if( DEBUG != 1){
  // Make sure we're not overwriting an existing file.
  filename = newfile("sapflow");
  cout << "Creating new file " << filename.c_str() << "...";
  // Write the headers for the file
  sapfile = ofstream(filename.c_str(), ios::out);
  sapfile << "Date, Upper baseline, Lower baseline, Upper peak, Lower peak, calculated sapflow" << endl;
  sapfile.close();
  cout << "Success"<<endl;
}

  // set to 2WIRE or 4WIRE as necessary
  upper_rtd.begin(MAX31865_4WIRE);
  lower_rtd.begin(MAX31865_4WIRE);
  heater_rtd.begin(MAX31865_4WIRE);
  Serial.println("\n ** Setup Complete ** ");
  
  sleep_cycle();
}

float upper_baseline, lower_baseline;
float upper_peak, lower_peak, sapflow;
void loop() 
{
  sample s = sample(upper_rtd, lower_rtd, heater_rtd); // Sample RTDs
  s.print(cout); // Print to serial output
  if (DEBUG != 1){ // DEBUG = 1 is an RTD test
    d.append(s);  // Log to SD card
    if ((millis()-event_time) < 60000) {
      switch(measuring_state){
        case wake:
          Serial.print("Awoke at ");
          Serial.println(rtc_ds.now().text());
          // Sample for 3 seconds before heating
          event_time += 3000;
          measuring_state = heating;
          upper_baseline = s.upper;
          lower_baseline = s.lower;
          break;
        case heating:
          digitalWrite(HEATER, HIGH);
          Serial.print("Heater On at ");
          Serial.println(rtc_ds.now().text());
          //Set the alarm for heating time
          event_time += 6000;
          measuring_state = cooling;
          break;
        case cooling:
          digitalWrite(HEATER, LOW);
          Serial.print("Heater Off at ");
          Serial.println(rtc_ds.now().text());
          //set the alarm for cooling time
          event_time += 40000;
          measuring_state = sleep;
          break;
        default:
          // Make sure we're done logging
          d.flush();
          upper_peak = s.upper;
          lower_peak = s.lower;
          sapflow = log((upper_peak-upper_baseline)/(lower_peak-lower_baseline));
          // Write the sapflow to the file.
          sapfile = ofstream(filename.c_str(), ios::out | ios::app);
          sapfile << s.t.text() << ", ";
          sapfile << upper_baseline << ", ";
          sapfile << lower_baseline << ", ";
          sapfile << upper_peak << ", ";
          sapfile << lower_peak << ", ";
          sapfile << sapflow << endl;
          sapfile.close();
          //Sleep until the next 5-minute interval
          sleep_cycle( 5 );
          measuring_state = wake;
      }
    }
  }
}
