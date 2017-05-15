#ifndef _SDCARD_H_
    #define _SDCARD_H_

   #define SD_DETECT_PIN 8
   #define SD_WP_PIN 9
   #define SD_SS_PIN 10

    #include "Arduino.h"

    #define cardInserted() (digitalRead(SD_DETECT_PIN) == 0)
    #define cardProtected() (digitalRead(SD_WP_PIN) != 0)

    //#include "board-v3.h"

    void setLEDstate(int state);
    //bool initCard(SdCard *card);

#endif
