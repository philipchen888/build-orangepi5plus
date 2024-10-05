"""
    This examples attempts to take a Mifare Classic 1K card that has been
    formatted for NDEF messages using mifareclassic_formatndef, and resets
    the authentication keys back to the Mifare Classic defaults

    To enable debug message, define DEBUG in nfc/pn532_debug.h
"""

import sys

if sys.argv[1] == "i2c":
    from pn532_i2c import *
else:
    from pn532_spi import *

NR_SHORTSECTOR          = 32    # Number of short sectors on Mifare 1K/4K
NR_LONGSECTOR           = 8     # Number of long sectors on Mifare 4K
NR_BLOCK_OF_SHORTSECTOR = 4     # Number of blocks in a short sector
NR_BLOCK_OF_LONGSECTOR  = 16    # Number of blocks in a long sector

# Determine the sector trailer block based on sector number
def BLOCK_NUMBER_OF_SECTOR_TRAILER(sector):
  return (sector)*NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR-1 if sector < NR_SHORTSECTOR else \
    NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR-1

# Determine the sector's first block based on the sector number
def BLOCK_NUMBER_OF_SECTOR_1ST_BLOCK(sector):
  (sector) * NR_BLOCK_OF_SHORTSECTOR if sector < NR_SHORTSECTOR else \
    NR_SHORTSECTOR * NR_BLOCK_OF_SHORTSECTOR + (sector - NR_SHORTSECTOR) * NR_BLOCK_OF_LONGSECTOR


# The default Mifare Classic key
KEY_DEFAULT_KEYAB = bytearray([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])

def setup():
  print("-------Looking for PN532--------")

  begin()

  versiondata = getFirmwareVersion()
  if not versiondata:
    print("Didn't find PN53x board")
    raise RuntimeError("Didn't find PN53x board")  # halt

  # Got ok data, print it out!
  print("Found chip PN5{:02x} Firmware ver. {:d}.{:d}".format((versiondata >> 24) & 0xFF, (versiondata >> 16) & 0xFF,
                                                             (versiondata >> 8) & 0xFF))

  # configure board to read RFID tags
  SAMConfig()
  
  print("Waiting for an ISO14443A Card ...")

def loop():
  authenticated = False               # Flag to indicate if the sector is authenticated
  blankAccessBits = bytearray([0xff, 0x07, 0x80 ])
  numOfSector = 16                 # Assume Mifare Classic 1K for now (16 4-block sectors)
  
  print("Place your NDEF formatted Mifare Classic 1K card on the reader")
  # Wait for user input before proceeding
  input("and press any key to continue ...")
  
    
  # Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  # 'uid' will be populated with the UID, and uidLength will indicate
  # if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success, uid = readPassiveTargetID(PN532_MIFARE_ISO14443A_106KBPS)

  if (success): 
    # We seem to have a tag ...
    # Display some basic information about it
    print("Found an ISO14443A card")
    print("UID Length: {:d}".format(len(uid)))
    print("UID Value: ", end =" ")
    PrintHex(uid)
    print("")
    
    # Make sure this is a Mifare Classic card
    if (len(uid) != 4):
      print("Ooops ... this doesn't seem to be a Mifare Classic card!") 
      return True
    
    print("Seems to be a Mifare Classic card (4 byte UID)")
    print("")
    print("Reformatting card for Mifare Classic (please don't touch it!) ... ")

    # Now run through the card sector by sector
    for idx in range(numOfSector):
      # Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      success = mifareclassic_AuthenticateBlock (uid, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, KEY_DEFAULT_KEYAB)
      if (not success):
        print("Authentication failed for sector {}".format(numOfSector))
        return True
      
      # Step 2: Write to the other blocks
      blockBuffer = bytearray([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
      if (idx == 16):
        if (not (mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer))):
          print("Unable to write to sector {}".format(numOfSector))
          return True
      if ((idx == 0) or (idx == 16)):
        if (not (mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer))):
          print("Unable to write to sector {}".format(numOfSector))
          return True
      else:
        if (not (mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer))):
          print("Unable to write to sector {}".format(numOfSector))
          return True
        if (not (mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer))):
          print("Unable to write to sector {}".format(numOfSector))
          return True

      blockBuffer = bytearray([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
      if (not(mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, blockBuffer))):
        print("Unable to write to sector {}".format(numOfSector))
        return True
      
      # Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      blockBuffer = KEY_DEFAULT_KEYAB + blankAccessBits + b'\x69' + KEY_DEFAULT_KEYAB

      # Step 4: Write the trailer block
      if (not (mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer))):
        print("Unable to write trailer block of sector ", end = " ")
        print(numOfSector)
        return True
  
  print("\nDone!")
  return True

if __name__ == '__main__':
    setup()
    found = loop()
    while not found:
        found = loop()
