/**************************************************************************/
/*!
    This example attempts to dump the contents of a Mifare Classic 1K card

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

  // configure board to read RFID tags
  SAMConfig();

  printf("Waiting for an ISO14443A Card ...\n");
}


void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;                     // Counter to keep track of which block we're on
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  uint8_t data[16];                         // Array to store block data during reads

  // Keyb on NDEF and Mifare Classic should be the same
  uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  printf("wait for a tag\n");
  // wait until a tag is present
  while (!readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { delay( 1 ); }

  success = readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false);
  
  if (success) {
    // Display some basic information about the card
    printf("Found an ISO14443A card\n");
    printf("  UID Length: ");
    printf("%d bytes\n", uidLength);
    printf("  UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      printf("%x ", uid[i]);
    }
    printf("\n");

    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ...
      printf("Seems to be a Mifare Classic card (4 byte UID)\n");

      // Now we try to go through all 16 sectors (each having 4 blocks)
      // authenticating each sector, and then dumping the blocks
      for (currentblock = 0; currentblock < 64; currentblock++)
      { 
        // if sector 1 corrupted, bypass it.
        //if (currentblock == 4) 
        //    currentblock = 8;
        // Check if this is a new block so that we can reauthenticate
        if (mifareclassic_IsFirstBlock(currentblock)) authenticated = false;

        // If the sector hasn't been authenticated, do so first
        if (!authenticated)
        {
          // Starting of a new sector ... try to to authenticate
          printf("------------------------Sector %d", currentblock/4);
          printf("-------------------------\n");
          if (currentblock == 0)
          {
              // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              // or 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 for NDEF formatted cards using key a,
              // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, keyuniversal);
          }
          else
          {
              // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              // or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
              // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, keyuniversal);
          }
          if (success)
          {
            authenticated = true;
          }
          else
          {
            printf("Authentication error\n");
          }
        }
        // If we're still not authenticated just skip the block
        if (!authenticated)
        {
          printf("Block %d unable to authenticate\n", currentblock);
        }
        else
        {
          // Authenticated ... we should be able to write the block now
          if ( currentblock >=4 && (currentblock %4) != 3 ) {
              memcpy(data, (const uint8_t[]){ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, sizeof data);
              mifareclassic_WriteDataBlock (currentblock, data);
          }
          // Dump the data into the 'data' array
          success = mifareclassic_ReadDataBlock(currentblock, data);
          if (success)
          {
            // Read successful
            printf("Block %d", currentblock);
            if (currentblock < 10)
            {
              printf("  ");
            }
            else
            {
              printf(" ");
            }
            // Dump the raw data
            PrintHexChar(data, 16);
          }
          else
          {
            // Oops ... something happened
            printf("Block %d", currentblock);
            printf(" unable to read this block\n");
          }
        }
      }
    }
    else if (uidLength == 7)
    {
        uint8_t data[4];
        // We probably have a Mifare Ultralight card ...
        printf("Seems to be a Mifare Ultralight tag (7 byte UID)\n");

        mifareultralight_ReadPage(3, data);
        int capacity = data[2] * 8;
        printf("Tag capacity %d bytes\n", capacity);

        memcpy(data, (const uint8_t[]){ 0, 0, 0, 0 }, sizeof data);
        for (int i=4; i<capacity/4; i++) {
            mifareultralight_WritePage (i, data);
        }
        for (int i=4; i<capacity/4; i++) {
            mifareultralight_ReadPage(i, data);
            printf("page %03d: ", i);
            PrintHexChar(data, 4);
        }
    }
    else
    {
      printf("Ooops ... this doesn't seem to be a Mifare Classic card!\n");
    }
  }
}

int main ( void )
{
    setup();
    loop();
    return 0;
}

