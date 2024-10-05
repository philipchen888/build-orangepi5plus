"""
    This example attempts to dump the contents of a Mifare Classic 1K card

    Note that you need the baud rate to be 115200 because we need to print
    out the data and read from the card at the same time!

    To enable debug message, define DEBUG in nfc/pn532_debug.h
"""

import sys

if sys.argv[1] == "i2c":
    from pn532_i2c import *
else:
    from pn532_spi import *

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

  # Keyb on NDEF and Mifare Classic should be the same
  keyuniversal = bytearray([ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF ])

  # Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  # 'uid' will be populated with the UID, and uidLength will indicate
  # if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = 0
  while not success:
      time.sleep(.0005)    # sleep 0.5 ms
      success, uid = readPassiveTargetID(PN532_MIFARE_ISO14443A_106KBPS)

  if (success):
    # Display some basic information about the card
    print("Found an ISO14443A card")
    print("UID Length: {:d}".format(len(uid)))
    print("UID Value: ", end =" ")
    PrintHex(uid)
    print("")

    if (len(uid) == 4):
      # We probably have a Mifare Classic card ...
      print("Seems to be a Mifare Classic card (4 byte UID)")

      # Now we try to go through all 16 sectors (each having 4 blocks)
      # authenticating each sector, and then dumping the blocks
      for currentblock in range(64):
        # Check if this is a new block so that we can reauthenticate
        if (mifareclassic_IsFirstBlock(currentblock)):
          authenticated = False

        # If the sector hasn't been authenticated, do so first
        if (not authenticated):
          # Starting of a new sector ... try to to authenticate
          print("------------------------Sector {:d}-------------------------".format(int(currentblock / 4)))
          if (currentblock == 0):
              # This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              # or 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 for NDEF formatted cards using key a,
              # but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = mifareclassic_AuthenticateBlock (uid, currentblock, 1, keyuniversal)
          else:
              # This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              # or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
              # but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = mifareclassic_AuthenticateBlock (uid, currentblock, 1, keyuniversal)
          if (success):
            authenticated = True
          else:
            print("Authentication error")
        # If we're still not authenticated just skip the block
        if (not authenticated):
          print("Block {:d}".format(currentblock), end = " ")
          print(" unable to authenticate")
        else:
          if currentblock >=4 and currentblock % 4 != 3:
            data = bytearray([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
            mifareclassic_WriteDataBlock(currentblock, data)
          # Authenticated ... we should be able to read the block now
          # Dump the data into the 'data' array
          success, data = mifareclassic_ReadDataBlock(currentblock)
          if (success):
            # Read successful
            print("Block {:d}".format(currentblock), end = " ")
            if (currentblock < 10):
              print("  ", end = " " )
            else:
              print(" ", end = " ")
            # Dump the raw data
            PrintHexChar(data, len(data))
          else:
            # Oops ... something happened
            print("Block {:d}".format(currentblock))
            print(" unable to read this block")
    elif (len(uid) == 7):
      #  We probably have a Mifare Ultralight card ...
      print("Seems to be a Mifare Ultralight tag (7 byte UID)")

      status, buf = mifareultralight_ReadPage(3)
      capacity = int(buf[2]) * 8
      print("Tag capacity {:d} bytes".format(capacity))

      data = bytearray([0,0,0,0])
      for i in range(4, int(capacity/4)):
        mifareultralight_WritePage(i, data)
 
      for i in range(4, int(capacity/4)):
        status, buf = mifareultralight_ReadPage(i)
        print("Reading page {:03d}: ".format(i), end =" ")
        for j in range( len(buf) ):
          buf[j] = int(buf[j])
        PrintHexChar(buf, len(buf))

    else:
      print("Ooops ... this doesn't seem to be a Mifare Classic card!")

    return True

if __name__ == '__main__':
    setup()
    found = loop()
    while not found:
        found = loop()
