# Clean resets a tag back to factory-like state
# For Mifare Classic, tag is zero'd and reformatted as Mifare Classic
# For Mifare Ultralight, tags is zero'd and left empty

import sys

if sys.argv[1] == "i2c":
    from pn532_i2c import *
else:
    from pn532_spi import *

password =  bytearray([ 0x12, 0x34, 0x56, 0x78])

def setup():
    print("NTAG21x R/W")
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
    print("wait for a tag")
    # wait until a tag is present
    tagPresent = False
    while not tagPresent:
        time.sleep(.0005)    # sleep 0.5 ms
        tagPresent, uid = readPassiveTargetID(PN532_MIFARE_ISO14443A_106KBPS)

    if (len(uid) == 7):
        #  We probably have a Mifare Ultralight card ...
        print("Seems to be a Mifare Ultralight tag (7 byte UID)")

        #  Display some basic information about the card
        print("Found an ISO14443A card")
        print("UID Length: {:d}".format(len(uid)))
        print("UID Value: ", end =" ")
        PrintHex(uid)
        print("")
    
        # if NTAG21x enables r/w protection, uncomment the following line 
        # ntag21x_auth(password)
   
        data = bytearray([ ord('g'), ord('o'), ord('o'), ord('g'), ord('l'), ord('e'), ord('.'), ord('c'), ord('o'), ord('m'), ord(' '), ord('@'), 0, 0, 0, 0])
        mifareultralight_WritePage(4, data)
        mifareultralight_WritePage(5, data[4:])
        mifareultralight_WritePage(6, data[8:])
        mifareultralight_WritePage(7, data[12:])
        data = bytearray([ ord('a'), ord('d'), ord('a'), ord('f'), ord('r'), ord('u'), ord('i'), ord('t'), ord('.'), ord('c'), ord('o'), ord('m'), 0, 0, 0, 0])
        mifareultralight_WritePage(8, data)
        mifareultralight_WritePage(9, data[4:])
        mifareultralight_WritePage(10, data[8:])
        mifareultralight_WritePage(11, data[12:])


        status, buf = mifareultralight_ReadPage(3)
        capacity = int(buf[2]) * 8
        print("Tag capacity {:d} bytes".format(capacity))
    
        for i in range(4, int(capacity/4)):
            status, buf = mifareultralight_ReadPage(i)
            print("Reading page {:03d}: ".format(i), end =" ")
            for j in range( len(buf) ):
                buf[j] = int(buf[j])
            PrintHexChar(buf, len(buf))
        return True
    else:
        print("It's not a Mifare Ultralight tag")
        return True

if __name__ == '__main__':
    setup()
    found = loop()
    while not found:
        found = loop()
