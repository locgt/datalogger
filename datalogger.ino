//Time stamped datalogger onto SD Card 
//@vmfoo - 7/19/2014

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 10;    
//The RTC Library needs Wire
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;


//The SDlibrary needs SPI
#include <SPI.h>
#include "SD.h"
//Some SD Card code borrowed from Adafruit library examples
// set up variables using the SD utility library functions:

File dataFile;
#define LOGFILE "datalog.txt"

//library to read the temp/humid sensor
#include "DHT.h"
#define DHTPIN 2     // what pin the DHT is connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(19200); 
  //Initialize the Sensor
  Serial.println("Initializing datalogger with RTC version 1.0");
  
  Serial.println("Starting Temp and Humidity Sensor");
  dht.begin();
  
  Serial.println("Starting SDCard reader and card");
  pinMode(chipSelect, OUTPUT);
  pinMode(SS, OUTPUT);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  
  Serial.println("Opening logfile for write.");
  // Open up the file we're going to log to!
  dataFile = SD.open(LOGFILE, FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening log file");
    // Wait forever since we cant write data
    while (1) ;
  }
  
  
  Serial.println("Starting Real Time Clock");
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif

  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  //uncomment this once and upload. Then comment it out and upload again
  //This will set the time the first time you upload it and if you don't upload again with it commented out it will 
  //reset the clock to the time the code was compliled each time.
  
  if (! rtc.isrunning()) {  //code borrowed from adafruit example rtc code
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  DateTime now = rtc.now();
  Serial.print("Unixtime: ");
  Serial.println(now.unixtime());
}

void loop(){
  char temp[6]; //2 int, 2 dec, 1 point, and \0
  char hum[6];  
  char msg[13];
  
  //get the temp/humid into chars to format
  ftoa(temp,readTemp());
  ftoa(hum,readHumidity());
  
  //Compile a comma delimited string to send to the log
  sprintf(msg,"%s,%s",temp, hum);
  logThis(msg);  
  delay(3000); 
}


void logThis(char* logmessage){
  char message[120];
  DateTime now = rtc.now();
  long epoch = now.unixtime();
  int Year = now.year();
  int Month = now.month();
  int Day = now.day();
  int Hour = now.hour();
  int Minute = now.minute();
  int Second = now.second();
  sprintf(message, "%ld,%d/%d/%d %02d:%02d:%02d,%s",epoch,Year,Month,Day,Hour,Minute,Second,logmessage );
  //Write the entry to the log file and console
  dataFile.println(message);
  dataFile.flush();
  // print to the serial port too:
  Serial.println(message);
}


float readTemp(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t = dht.readTemperature();
  float tf = t * 1.8 +32;  //Convert from C to F 
  return   tf;
}

float readHumidity(){
  float h = dht.readHumidity();  
  return h;
}


int ftoa(char *a, float f)  //translates floating point readings into strings
{
  int left=int(f);
  float decimal = f-left;
  int right = decimal *100; //2 decimal points
  if (right > 10) {  //if the decimal has two places already. Otherwise
    sprintf(a, "%d.%d",left,right);
  } else { 
    sprintf(a, "%d.0%d",left,right); //pad with a leading 0
  }
}


