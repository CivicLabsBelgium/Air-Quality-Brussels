// SD card access
#include "sdcard/sdcardutils.h"
#include "sdcard/Fat16util.h"
#include "sdcard/SdCard.h"
#include "sdcard/Fat16.h"
#include "sdcard/SdInfo.h"
#include "sdcard/Fat16Config.h"
#include "sdcard/Fat16mainpage.h"

#include "sds011/SDS011.h"

#include "ds1307/RTClib.h"

SdCard card;
Fat16 fs;

float p10,p25;
int error;

SDS011 my_sds;

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


/*
 * Attempts to initialize an SD card with a FAT16 FS, and create a new file to write data to.
 * Up to 10 files will be created following the naming pattern data0.csv - data9.csv
 */
bool initCard(SdCard *sd, Fat16 *file)
{
  if (!sd->begin(SD_SS_PIN))
  {
    Serial.println("Problem initializing SD card.");
  }
  else
  {
    Serial.println("SD card intialized successfully.");

    if (!Fat16::init(sd))
    {
      Serial.println("Unable to initialize the FAT16 file system on the card.");
    }
    else
    {
      Serial.println("FAT16 file system initialized successfully.");

      // create a new file
      char name[] = "data0.csv";
      for (uint8_t i = 0; i < 10; i++)
      {
        name[7] = i + '0';
        // O_CREAT - create the file if it does not exist
        // O_EXCL - fail if the file exists
        // O_WRITE - open for write only
        if (file->open(name, O_CREAT | O_EXCL | O_WRITE))
          break;
      }

      if (!file->isOpen())
      {
        Serial.println("Failed to open file on the card.");
      }
      else
      {
        Serial.print("Data will be saved to file "); Serial.print(name); Serial.println(".");
        return true;
      }
    }
  }
  return false;
}

uint32_t prevSyncTime = millis();
#define SYNC_INT 60000 // write data to card every minute

/*
 * Flushes data in the SD card's RAM buffer to the flash memory
 */
void syncData(Fat16 *file)
{
  if (millis() > prevSyncTime + SYNC_INT)
  {
      prevSyncTime = millis();
      if (!file->sync())
      {
        Serial.println(F("Error logging to SD card."));
      }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Starting logger...");

  pinMode(SD_SS_PIN, OUTPUT);

      if (!initCard(&card, &fs)) {
        Serial.println("ERROR: card couldn't be initialized. Wrong file system?");
        while (1);
      } else {
        Serial.println("SD card initialized and ready to accept data.");
      }

  my_sds.begin(0,1);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

}

uint16_t year;
uint8_t month, day, hour, minute, second;

void loop() {

  error = my_sds.read(&p25,&p10);
  if (! error) {
    Serial.println("P2.5: "+String(p25));
    Serial.println("P10:  "+String(p10));
  }

  DateTime now = rtc.now();
  year = now.year();
  month = now.month();
  day = now.day();
  hour = now.hour();
  minute = now.minute();
  second = now.second();

  fs.write(year); fs.write(","); fs.write(month); fs.write(","); fs.write(day); fs.write(","); fs.write(hour); fs.write(","); fs.write(minute); fs.write(","); fs.write(second);
  fs.write(p25);
  fs.write(",");
  fs.write(p10);

  syncData(&fs);
}
