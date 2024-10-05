/**************************************************************************/
/*! 
    @file     readMifare.pde
    @author   Adafruit Industries
	@license  BSD (see license.txt)

    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.


This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
This library works with the Adafruit NFC breakout 
  ----> https://www.adafruit.com/products/364
 
Check out the links above for our tutorials and wiring diagrams 
These chips use SPI or I2C to communicate.

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

*/
/**************************************************************************/
#include "pn532.h"

void setup(void) {
  begin();

  uint32_t versiondata = getFirmwareVersion();
  if (! versiondata) {
    printf("Didn't find PN53x board\n");
    while (1); // halt
  }
  // Got ok data, print it out!
  printf("Found chip PN5%x\n", (versiondata>>24) & 0xFF); 
  printf("Firmware ver. %d.%d\n", (versiondata>>16) & 0xFF, (versiondata>>8) & 0xFF); 
  
  // configure board to read RFID tags
  SAMConfig();
  
  printf("Waiting for an ISO14443A Card ...\n");
}


void loop(void) {
  uint8_t i;
  uint8_t currentblock;
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  printf("wait for a tag\n");
  // wait until a tag is present
  while (!readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { }

  success = readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false);
  
  if (success) {
    // Display some basic information about the card
    printf("Found an ISO14443A card\n");
    printf("  UID Length: %d bytes\n", uidLength);
    printf("  UID Value: ");
    PrintHex(uid, uidLength);
    printf("\n");
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      printf("Seems to be a Mifare Classic card (4 byte UID)\n");
	  
      // Now we need to try to authenticate it for read/write access
      // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      printf("Trying to authenticate block 16 with default KEYA value\n");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	  
	  // Start with block 52, since sector 0
	  // contains the manufacturer data and it's probably better just
	  // to leave it alone unless you know what you're doing
/*
      for (currentblock = 0; currentblock < 64; currentblock = currentblock+1)
      {
          success = mifareclassic_AuthenticateBlock(uid, uidLength, currentblock, 1, keya);
          if (success) {
              printf("block %d ok\n", currentblock);
          } else {
              printf("block %d\n", currentblock);
          }
      }
*/
      success = mifareclassic_AuthenticateBlock(uid, uidLength, 16, 1, keya);
	  
      if (success)
      {
        printf("Sector 4 (Blocks 16..19) has been authenticated\n");
        uint8_t data[16];
		
        memcpy(data, (const uint8_t[]){ 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0 }, sizeof data);
        success = mifareclassic_WriteDataBlock (16, data);

        success = mifareclassic_ReadDataBlock(16, data);
        if (success) 
        {
            printf("Reading Block 16:\n");
            PrintHexChar(data, 16);
            printf("\n");
        }

        for ( i = 16; i < 20; i++ ) 
        {	
            success = mifareclassic_ReadDataBlock(i, data);	
            if (success)
            {
              // Data seems to have been read ... spit it out
              printf("Reading Block %d:\n", i);
              PrintHexChar(data, 16);
              printf("\n");
		  
              // Wait a bit before reading the card again
              delay(100);
            }
            else
            {
              printf("Ooops ... unable to read the requested block.  Try another key?\n");
            }
        }
      }
      else
      {
        printf("Ooops ... authentication failed: Try another key?\n");
      }
    }
    
    if (uidLength == 7)
    {
      uint8_t data[16];
      // We probably have a Mifare Ultralight card ...
      printf("Seems to be a Mifare Ultralight tag (7 byte UID)\n");

      memcpy(data, (const uint8_t[]){ 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0 }, sizeof data);
      mifareultralight_WritePage (4, data);
      mifareultralight_WritePage (5, data+4);
      mifareultralight_WritePage (6, data+8);
      mifareultralight_WritePage (7, data+12);

      // Try to read the first general-purpose user page (#4)
      printf("Reading page 4\n");
      success = mifareultralight_ReadPage (4, data);
      if (success)
      {
        // Data seems to have been read ... spit it out
        PrintHexChar(data, 16);
        printf("\n");
		
        // Wait a bit before reading the card again
        delay(100);
      }
      else
      {
        printf("Ooops ... unable to read the requested page!?\n");
      }
    }
  }
}

int main ( void )
{
    setup();
    loop();
    return 0;
}
