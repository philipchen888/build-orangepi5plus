/**************************************************************************/
/*! 
    This example will attempt to connect to an ISO14443A
    card or tag and retrieve some basic information about it
    that can be used to determine what type of card it is.   
   
    Note that you need the baud rate to be 115200 because we need to print
    out the data and read from the card at the same time!

    To enable debug message, define DEBUG in PN532/PN532_debug.h
    
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

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  SAMConfig();
  
  printf("Waiting for an ISO14443A Card ...\n");
}

void loop(void) {
  bool success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  printf("wait for a tag\n");
  // wait until a tag is present
  while (!readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { delay( 1 ); }

  success = readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false);
  
  //success = readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 200, false);
  
  if (success) {
    printf("Found a card!\n");
    printf("UID Length: %d bytes\n", uidLength);
    printf("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) 
    {
      printf(" 0x%02x", uid[i]); 
    }
    printf("\n");
  }
  else
  {
    // PN532 probably timed out waiting for a card
    printf("Timed out waiting for a card\n");
  }
}

int main ( void )
{
    setup();
    loop();
    return 0;
}
