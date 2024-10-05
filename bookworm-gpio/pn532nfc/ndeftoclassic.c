/**************************************************************************/
/*! 
    This examples attempts to take a Mifare Classic 1K card that has been
    formatted for NDEF messages using mifareclassic_formatndef, and resets
    the authentication keys back to the Mifare Classic defaults

    To enable debug message, define DEBUG in PN532/PN532_debug.h
*/
/**************************************************************************/
#include "pn532.h"

#define NR_SHORTSECTOR          (32)    // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR           (8)     // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4)     // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR  (16)    // Number of blocks in a long sector

// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector)<NR_SHORTSECTOR)? \
  ((sector)*NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR-1):\
  (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR-1))

// Determine the sector's first block based on the sector number
#define BLOCK_NUMBER_OF_SECTOR_1ST_BLOCK(sector) (((sector)<NR_SHORTSECTOR)? \
  ((sector)*NR_BLOCK_OF_SHORTSECTOR):\
  (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR))

// The default Mifare Classic key
static const uint8_t KEY_DEFAULT_KEYAB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  uint8_t blockBuffer[16];                  // Buffer to store block contents
  uint8_t blankAccessBits[3] = { 0xff, 0x07, 0x80 };
  uint8_t idx = 0;
  uint8_t numOfSector = 16;                 // Assume Mifare Classic 1K for now (16 4-block sectors)
  
  printf("Place your NDEF formatted Mifare Classic 1K card on the reader\n");
  printf("and press any key to continue ...\n");
  
  // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  printf("wait for a tag\n");
  // wait until a tag is present
  while (!readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { delay( 1 ); }

  success = readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false);
  
  if (success) 
  {
    // We seem to have a tag ...
    // Display some basic information about it
    printf("Found an ISO14443A card/tag\n");
    printf("  UID Length: %d bytes\n", uidLength);
    printf("  UID Value: ");
    PrintHex(uid, uidLength);
    printf("\n");
    
    // Make sure this is a Mifare Classic card
    if (uidLength != 4)
    {
      printf("Ooops ... this doesn't seem to be a Mifare Classic card!\n"); 
      return;
    }    
    
    printf("Seems to be a Mifare Classic card (4 byte UID)\n");
    printf("Reformatting card for Mifare Classic (please don't touch it!) ... \n");

    // Now run through the card sector by sector
    for (idx = 0; idx < numOfSector; idx++)
    {
      // Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      success = mifareclassic_AuthenticateBlock (uid, uidLength, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, (uint8_t *)KEY_DEFAULT_KEYAB);
      if (!success)
      {
        printf("Authentication failed for sector %d\n", numOfSector);
        return;
      }
      
      // Step 2: Write to the other blocks
      if (idx == 16)
      {
        memset(blockBuffer, 0, sizeof(blockBuffer));
        if (!(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer)))
        {
          printf("Unable to write to sector %d\n", numOfSector);
          return;
        }
      }
      if ((idx == 0) || (idx == 16))
      {
        memset(blockBuffer, 0, sizeof(blockBuffer));
        if (!(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
        {
          printf("Unable to write to sector %d\n", numOfSector);
          return;
        }
      }
      else
      {
        memset(blockBuffer, 0, sizeof(blockBuffer));
        if (!(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer)))
        {
          printf("Unable to write to sector %d\n", numOfSector);
          return;
        }
        if (!(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
        {
          printf("Unable to write to sector %d\n", numOfSector);
          return;
        }
      }
      memset(blockBuffer, 0, sizeof(blockBuffer));
      if (!(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, blockBuffer)))
      {
        printf("Unable to write to sector %d\n", numOfSector);
        return;
      }
      
      // Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      memcpy(blockBuffer, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));
      memcpy(blockBuffer + 6, blankAccessBits, sizeof(blankAccessBits));
      blockBuffer[9] = 0x69;
      memcpy(blockBuffer + 10, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));

      // Step 4: Write the trailer block
      if (!(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer)))
      {
        printf("Unable to write trailer block of sector %d\n", numOfSector);
        return;
      }
    }
  }
  
  printf("\nDone!\n");
}

int main ( void )
{
    setup();
    loop();
    return 0;
}
