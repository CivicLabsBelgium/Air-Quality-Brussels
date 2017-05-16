/*
  Data logger for PM2.5 and PM10 particles using the SDS011 particle sensor.
  Connect sensor to primary UART of the board.
  Designed for Arduino UNO / Sparkfun Redboard platform.
  Written by Yannick Verbelen (2017). See https://github.com/CivicLabsBelgium/Air-Quality-Brussels
*/

#include "SDS011.h"
#include "RTClib.h"
#include <SPI.h> // needs to be included again for PlatformIO
#include <SD.h>

// pin to which the slave select of the SD card is connected
#define SD_SS_PIN 10

// set up variables using the SD utility library functions:
Sd2Card card;      // SD card instance
SdVolume volume;   // volume instance
SdFile root;       // file system instance

float p10, p25;    // particle concentration in ppm
SDS011 SDS;        // sensor instance
RTC_DS1307 rtc;    // rtc instance

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Starting logger...");

  pinMode(SD_SS_PIN, OUTPUT);

  Serial.print("Initializing SD card... ");

  // see if the card is present and can be initialized:
  if (!SD.begin(SD_SS_PIN)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // initialize the SDS011 particle sensor
  SDS.begin(0,1);

  // initialize RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // configure RTC if necessary
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  Serial.println("Initialization complete!");
}

uint16_t year;
uint8_t month, day, hour, minute, second;
uint8_t sensor_err;

void loop() {

  // try to read samples from the SDS011 sensor
  sensor_err = SDS.read(&p25,&p10);

  // if samples could be acquired, log them to the SD card together with a time stamp
  if (sensor_err) {
    Serial.println("Unable to acquire samples from sensor, check wiring. Retrying in 5 seconds...");
    delay(4000);
  } else {

  DateTime now = rtc.now();
  year = now.year();
  month = now.month();
  day = now.day();
  hour = now.hour();
  minute = now.minute();
  second = now.second();

  // make a string for assembling the data to log:
  String dataString = "";
  // add the time stamp
  dataString += String(year) + "," + String(month) + "," + String(day) + "," + String(hour) + "," + String(minute) + "," + String(second);
  // add particle data
  dataString += "," + String(p25) + "," + String(p10);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("data", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

    }

  delay(1000); // take one measurement per second

}
