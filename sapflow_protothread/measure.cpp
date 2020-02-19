#include "measure.h"
#include "lora.h"

float maxtemp; ///< Used to store the max heater temperature

int sample_timer(struct pt *pt)
{
  PT_BEGIN(pt);
  Serial.println("Initialized timer thread");
  while(1)
  {
    sample_trigger = true;
    // Serial.println("Trigger");
    PT_YIELD(pt); // Give everyone a chance to see the trigger
    sample_trigger = false; // Clear the trigger
    PT_TIMER_DELAY(pt,1000); // Sample every 1000ms
  }
  PT_END(pt);
}

int measure(struct pt *pt)
{
  PT_BEGIN(pt);
  Serial.print("Initializing measurement thread... ");
  static Adafruit_MAX31865 upper_rtd = Adafruit_MAX31865(A5);
  static Adafruit_MAX31865 lower_rtd = Adafruit_MAX31865(A4);
  static Adafruit_MAX31865 heater_rtd = Adafruit_MAX31865(A3);
  upper_rtd.begin(MAX31865_2WIRE);
  lower_rtd.begin(MAX31865_2WIRE);
  heater_rtd.begin(MAX31865_2WIRE);
  Serial.println("Done");
  PT_YIELD(pt);
  while(1)
  {      
    // Wait for next sample trigger
    PT_WAIT_UNTIL(pt, (sample_trigger || sleep));
    // Check if we prepare for sleep
    if( sleep )
    {
      // This seems to never be called.
      Serial.println("Sleeping");
      PT_WAIT_WHILE(pt, sleep); // sleep occurs here
      break;
    }
    PT_WAIT_WHILE(pt, sample_trigger);
    //Serial.println("Sampling...");
    // Get the latest temperature
    latest.upper = upper_rtd.temperature(Rnom, Rref);
    latest.lower = lower_rtd.temperature(Rnom, Rref);
    latest.heater = heater_rtd.temperature(Rnom, Rref);
    DateTime t = rtc_ds.now();
    // Print to Serial terminal
    cout << "Upper: " << latest.upper << " Lower: " 
    << latest.lower << " Heater: " <<latest.heater 
    << " Time: " << t.text() << endl;
    // Save calculated sapflow
    cout<<"Logfile...";
    ofstream logfile = ofstream("demo_log.csv", 
                                     ios::out | ios::app );
    cout<<"opened...";
    logfile << t.text() << ", ";
    logfile << setw(6) << latest.upper << ", "
    << setw(6) << latest.lower << ", "
    << setw(6) << latest.heater << endl;
    logfile.close(); // Ensure the file is closed
    cout<<"done"<<endl;
  }
  PT_END(pt);
}

// Calculates baseline temperature
int baseline(struct pt *pt)
{
  PT_BEGIN(pt);
  Serial.print("Initializing baseline thread... ");
  // Declare persistant variable for this thread
  static int i;
  // Initialize the baseline (reference) temperature
  reference.upper = 0;
  reference.lower = 0;
  Serial.println("Done");
  for(i = 0; i < 10; ++i){
    PT_WAIT_UNTIL(pt, sample_trigger);
    PT_WAIT_WHILE(pt, sample_trigger);
    reference.upper += latest.upper;
    reference.lower += latest.lower;
  }while(millis()<(pt)->t);
  reference.upper /= i;
  reference.lower /= i;
  cout<<"Baseline: "<<reference.upper<<", "<<reference.lower<<endl;
  PT_END(pt);
}


// Calculates temperature delta and sapflow
int delta(struct pt *pt)
{
  PT_BEGIN(pt);
  Serial.print("Initializing delta thread... ");
  // Declare persistent variables for this thread
  static int i;
  static float flow;
  maxtemp = latest.heater; // This should be the heater peak temperature
  // Initialize the flow value
  flow = 0;
  Serial.println("Done");
  for(i = 0; i < 5; ++i ){
    PT_WAIT_UNTIL(pt, sample_trigger);
    PT_WAIT_WHILE(pt, sample_trigger);
    // Ratio of upper delta over lower delta
    float udelt = latest.upper - reference.upper;
    float ldelt = latest.lower - reference.lower;
    cout << "Delta: " << udelt <<", " << ldelt << endl;
    flow += udelt / ldelt;
  };
  cout<<"Finished measurements."<<endl;
  flow /= i;
  flow = log(flow) * (3600.*2e-6/7e-3);
  cout<<"Flow is "<<flow<<endl;
  // Write the sapflow to the file.
  cout<<"Opening logfile...";
  ofstream sapfile = ofstream("demo.csv", ios::out | ios::app);
  cout<<"Done"<<endl;
  char * time = rtc_ds.now().text();
  char * weight = read_weight();
  cout << time << ", "
  << reference.upper << ", "
  << reference.lower << ", "
  << flow << ", " << endl;
  sapfile << time << ", "<< reference.upper << ", "
  << reference.lower << ", "<< flow << ", "<< weight
  << endl;
  sapfile.close();
  lora_init();
  build_msg(flow, weight, reference.upper, maxtemp);
  send_msg();
  
  PT_END(pt);
}