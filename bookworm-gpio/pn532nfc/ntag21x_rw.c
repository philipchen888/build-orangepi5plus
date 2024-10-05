
// Clean resets a tag back to factory-like state
// For Mifare Classic, tag is zero'd and reformatted as Mifare Classic
// For Mifare Ultralight, tags is zero'd and left empty

#include "pn532.h"

uint8_t password[4] =  {0x12, 0x34, 0x56, 0x78};
uint8_t buf[4];
uint8_t uid[7]; 
uint8_t uidLength;

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
    uint8_t data[16];
    printf("wait for a tag\n");
    // wait until a tag is present
    while (!readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { delay( 1 ); }

    // if NTAG21x enables r/w protection, uncomment the following line 
    // nfc.ntag21x_auth(password);

    memcpy(data, (const uint8_t[]){ 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0 }, sizeof data);
    mifareultralight_WritePage (4, data);
    mifareultralight_WritePage (5, data+4);
    mifareultralight_WritePage (6, data+8);
    mifareultralight_WritePage (7, data+12);
    memcpy(data, (const uint8_t[]){ 'y', 'a', 'h', 'o', 'o', '.', 'c', 'o', 'm', ',', 'g', 'o', 'o', 'g', 'l', 'e' }, sizeof data);
    mifareultralight_WritePage (8, data);
    mifareultralight_WritePage (9, data+4);
    mifareultralight_WritePage (10, data+8);
    mifareultralight_WritePage (11, data+12);

    mifareultralight_ReadPage(3, buf);
    int capacity = buf[2] * 8;
    printf("Tag capacity %d bytes\n", capacity);

    for (int i=4; i<capacity/4; i++) {
        mifareultralight_ReadPage(i, buf);
        printf("page %03d: ", i);
        PrintHexChar(buf, 4);
    }

    // wait until the tag is removed
    //while (readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { }
}

int main ( void )
{
    setup();
    loop();
    return 0;
}
