"""
    This example will attempt to connect to an ISO14443A
    card or tag and retrieve some basic information about it
    that can be used to determine what type of card it is.   
   
    To enable debug message, set DEBUG in PN532_log.h
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

    # Got ok data, print it out!
    print("Found chip PN5{:02x} Firmware ver. {:d}.{:d}".format((versiondata >> 24) & 0xFF, (versiondata >> 16) & 0xFF,
                                                                (versiondata >> 8) & 0xFF))
    # Set the max number of retry attempts to read from a card
    # This prevents us from waiting forever for a card, which is
    # the default behaviour of the pn532.
    # setPassiveActivationRetries(0xFF)

    # configure board to read RFID tags
    SAMConfig()

    print("Waiting for an ISO14443A card")


def loop():
    # Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    # 'uid' will be populated with the UID, and uidLength will indicate
    # if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    success = 0
    while not success:
        time.sleep(.0005)    # sleep 0.5 ms
        success, uid = readPassiveTargetID(PN532_MIFARE_ISO14443A_106KBPS)


    if (success):
        print("Found a card!")
        print("UID Length: {:d}".format(len(uid)))
        print("UID Value: ", end = " ")
        PrintHex(uid)
        print("")
    else:
        # pn532 probably timed out waiting for a card
        print("Timed out waiting for a card")

    return True


if __name__ == '__main__':
    setup()
    found = loop()
    while not found:
      found = loop()
