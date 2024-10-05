"""
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

    To enable debug message, define DEBUG in nfc/pn532_log.h
"""

import sys

if sys.argv[1] == "i2c":
    from pn532_i2c import *
else:
    from pn532_spi import *

def setup():
    begin()

    versiondata = getFirmwareVersion()
    if not versiondata:
        print("Didn't find PN53x board")
        raise RuntimeError("Didn't find PN53x board")  # halt

    #  Got ok data, print it out!
    print("Found chip PN5{:02x} Firmware ver. {:d}.{:d}".format((versiondata >> 24) & 0xFF, (versiondata >> 16) & 0xFF,
                                                                (versiondata >> 8) & 0xFF))

    #  configure board to read RFID tags
    SAMConfig()

    print("Waiting for an ISO14443A Card ...")


def loop():
    #  Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    #  'uid' will be populated with the UID, and uidLength will indicate
    #  if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    success = 0
    while not success:
        time.sleep(.0005)    # sleep 0.5 ms
        success, uid = readPassiveTargetID(PN532_MIFARE_ISO14443A_106KBPS)

    if (success):
        #  Display some basic information about the card
        print("Found an ISO14443A card")
        print("UID Length: {:d}".format(len(uid)))
        print("UID Value: ", end =" ")
        PrintHex(uid)
        print("")

        if (len(uid) == 4):
            #  We probably have a Mifare Classic card ...
            print("Seems to be a Mifare Classic card (4 byte UID)")

            #  Now we need to try to authenticate it for read/write access
            #  Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
            print("Trying to authenticate block 4 with default KEYA value")
            keya = bytearray([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])

            #  Start with block 4 (the first block of sector 1) since sector 0
            #  contains the manufacturer data and it's probably better just
            #  to leave it alone unless you know what you're doing
            success = mifareclassic_AuthenticateBlock(uid, 4, 0, keya)

            if (success):
                print("Sector 1 (Blocks 4..7) has been authenticated")

                #  If you want to write something to block 4 to test with, uncomment
                #  the following line and this text should be read back in a minute
                data = bytearray([ ord('a'), ord('d'), ord('a'), ord('f'), ord('r'), ord('u'), ord('i'), ord('t'), ord('.'), ord('c'), ord('o'), ord('m'), 0, 0, 0, 0])
                success = mifareclassic_WriteDataBlock (4, data)

                #  Try to read the contents of block 4
                success, data = mifareclassic_ReadDataBlock(4)

                if (success):
                    #  Data seems to have been read ... spit it out
                    #print("Reading Block 4: {}".format(binascii.hexlify(data)))
                    print("Reading Block 4: ", end =" ")
                    PrintHexChar(data, len(data))

                else:
                    print("Ooops ... unable to read the requested block.  Try another key?")
            else:
                print("Ooops ... authentication failed: Try another key?")

        elif (len(uid) == 7):
            #  We probably have a Mifare Ultralight card ...
            print("Seems to be a Mifare Ultralight tag (7 byte UID)")

            data = bytearray([ ord('a'), ord('d'), ord('a'), ord('f'), ord('r'), ord('u'), ord('i'), ord('t'), ord('.'), ord('c'), ord('o'), ord('m'), 0, 0, 0, 0])
            mifareultralight_WritePage(4, data)
            mifareultralight_WritePage(5, data[4:])
            mifareultralight_WritePage(6, data[8:])
            mifareultralight_WritePage(7, data[12:])

            #  Try to read general-purpose user page 4-7
            print("Reading page 4-7")
            for i in range( 4, 8 ):
                success, data = mifareultralight_ReadPage(i)
                if (success):
                    #  Data seems to have been read ... spit it out
                    print("Reading page {:03d}: ".format(i), end =" ")
                    for j in range( len(data) ):
                       data[j] = int(data[j])
                    PrintHexChar(data, len(data))

                else:
                    print("Ooops ... unable to read the requested page!?")

    return True

if __name__ == '__main__':
    setup()
    found = loop()
    while not found:
        found = loop()
