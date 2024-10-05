/**************************************************************************/
/*!
    This example attempts to format a clean Mifare Classic 1K card as
    an NFC Forum tag (to store NDEF messages that can be read by any
    NFC enabled Android phone, etc.)

    Note that you need the baud rate to be 115200 because we need to print
    out the data and read from the card at the same time!

    To enable debug message, define DEBUG in PN532/PN532_debug.h
*/
/**************************************************************************/

#include "pn532.h"

/*
    We can encode many different kinds of pointers to the card,
    from a URL, to an Email address, to a phone number, and many more
    check the library header .h file to see the large # of supported
    prefixes!
*/
// For a http://www. url:
const char * url = "elechouse.com";
uint8_t ndefprefix = NDEF_URIPREFIX_HTTP_WWWDOT;

// for an email address
//const char * url = "mail@example.com";
//uint8_t ndefprefix = NDEF_URIPREFIX_MAILTO;

// for a phone number
//const char * url = "+1 212 555 1212";
//uint8_t ndefprefix = NDEF_URIPREFIX_TEL;

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

  // Use the default key
  uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  printf("\nPLEASE NOTE: Formatting your card for NDEF records will change the\n");
  printf("authentication keys.  To reformat your NDEF tag as a clean Mifare\n");
  printf("Classic tag, use the mifareclassic_ndeftoclassic example!\n");
  printf("\n");

  // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  printf("wait for a tag\n");
  // wait until a tag is present
  while (!readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false)) { delay( 1 ); }

  success = readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200, false);
  
  if (success)
  {
    // Display some basic information about the card
    printf("Found an ISO14443A card\n");
    printf("  UID Length: %d bytes\n", uidLength);
    printf("  UID Value: ");
    PrintHex(uid, uidLength);
    for (uint8_t i = 0; i < uidLength; i++) {
      printf("%02x ", uid[i]);
    }
    printf("\n");

    // Make sure this is a Mifare Classic card
    if (uidLength != 4)
    {
      printf("Ooops ... this doesn't seem to be a Mifare Classic card!\n");
      return;
    }

    // We probably have a Mifare Classic card ...
    printf("Seems to be a Mifare Classic card (4 byte UID)\n");

    // Try to format the card for NDEF data
    success = mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (!success)
    {
      printf("Unable to authenticate block 0 to enable card formatting!\n");
      return;
    }
    success = mifareclassic_FormatNDEF();
    if (!success)
    {
      printf("Unable to format the card for NDEF\n");
      return;
    }

    printf("Card has been formatted for NDEF data using MAD1\n");

    // Try to authenticate block 4 (first block of sector 1) using our key
    success = mifareclassic_AuthenticateBlock (uid, uidLength, 4, 0, keya);

    // Make sure the authentification process didn't fail
    if (!success)
    {
      printf("Authentication failed.\n");
      return;
    }

    // Try to write a URL
    printf("Writing URI to sector 1 as an NDEF Message\n");

    // Authenticated seems to have worked
    // Try to write an NDEF record to sector 1
    // Use 0x01 for the URI Identifier Code to prepend "http://www."
    // to the url (and save some space).  For information on URI ID Codes
    // see http://www.ladyada.net/wiki/private/articlestaging/nfc/ndef
    if (strlen(url) > 38)
    {
      // The length is also checked in the WriteNDEFURI function, but lets
      // warn users here just in case they change the value and it's bigger
      // than it should be
      printf("URI is too long ... must be less than 38 characters long\n");
      return;
    }

    // URI is within size limits ... write it to the card and report success/failure
    success = mifareclassic_WriteNDEFURI(1, ndefprefix, url);
    if (success)
    {
      printf("NDEF URI Record written to sector 1\n");
    }
    else
    {
      printf("NDEF Record creation failed! :(\n");
    }
  }
}

int main ( void )
{
    setup();
    loop();
    return 0;
}
