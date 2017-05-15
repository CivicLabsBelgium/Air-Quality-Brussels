// SD card access
#include "sdcard/sdcardutils.h"
#include "sdcard/Fat16util.h"
#include "sdcard/SdCard.h"
#include "sdcard/Fat16.h"
#include "sdcard/SdInfo.h"
#include "sdcard/Fat16Config.h"
#include "sdcard/Fat16mainpage.h"

#include "sds011/SDS011.h"

SdCard card;
Fat16 fs;

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

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("starting logger");

  // configure pins of card detection and write protection
  pinMode(SD_DETECT_PIN, INPUT);
  pinMode(SD_WP_PIN, INPUT);
  pinMode(SD_SS_PIN, OUTPUT);

  bool cardInstalled = !digitalRead(SD_DETECT_PIN);
  bool cardProtected = digitalRead(SD_WP_PIN);

  // check if a card is present
  if (cardInstalled)
  {
    // check if the card is read only
    if (!cardProtected)
    {
      if (!initCard(&card, &fs)) {
        Serial.println("card init failed");
      } else {
        Serial.println("card init succeeded");
      }
    } else {
      Serial.println("ERROR: card is read only! Flip write protection switch and reboot.");
      while (1);
    }
  } else {
    Serial.println("ERROR: no SD card installed. Insert a card and reboot.");
    while (1);
  }

}

void loop() {




}
