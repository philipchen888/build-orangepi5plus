
import time
import binascii
from periphery import SPI

# Open spidev0.0 with mode 0 and max speed 1MHz
spi = SPI("/dev/spidev0.0", 0, 1000000)

STATUS_READ                   = (0x2)
DATA_WRITE                    = (0x1)
DATA_READ                     = (0x3)
PN532_PREAMBLE                = (0x00)
PN532_STARTCODE1              = (0x00)
PN532_STARTCODE2              = (0xFF)
PN532_POSTAMBLE               = (0x00)
PN532_HOSTTOPN532             = (0xD4)
PN532_PN532TOHOST             = (0xD5)
PN532_ACK_WAIT_TIME           = (10)  # ms, timeout of waiting for ACK
PN532_INVALID_ACK             = (-1)
PN532_TIMEOUT                 = (-2)
PN532_INVALID_FRAME           = (-3)
PN532_NO_SPACE                = (-4)

PN532_COMMAND_DIAGNOSE              = (0x00)
PN532_COMMAND_GETFIRMWAREVERSION    = (0x02)
PN532_COMMAND_GETGENERALSTATUS      = (0x04)
PN532_COMMAND_READREGISTER          = (0x06)
PN532_COMMAND_WRITEREGISTER         = (0x08)
PN532_COMMAND_READGPIO              = (0x0C)
PN532_COMMAND_WRITEGPIO             = (0x0E)
PN532_COMMAND_SETSERIALBAUDRATE     = (0x10)
PN532_COMMAND_SETPARAMETERS         = (0x12)
PN532_COMMAND_SAMCONFIGURATION      = (0x14)
PN532_COMMAND_POWERDOWN             = (0x16)
PN532_COMMAND_RFCONFIGURATION       = (0x32)
PN532_COMMAND_RFREGULATIONTEST      = (0x58)
PN532_COMMAND_INJUMPFORDEP          = (0x56)
PN532_COMMAND_INJUMPFORPSL          = (0x46)
PN532_COMMAND_INLISTPASSIVETARGET   = (0x4A)
PN532_COMMAND_INATR                 = (0x50)
PN532_COMMAND_INPSL                 = (0x4E)
PN532_COMMAND_INDATAEXCHANGE        = (0x40)
PN532_COMMAND_INCOMMUNICATETHRU     = (0x42)
PN532_COMMAND_INDESELECT            = (0x44)
PN532_COMMAND_INRELEASE             = (0x52)
PN532_COMMAND_INSELECT              = (0x54)
PN532_COMMAND_INAUTOPOLL            = (0x60)
PN532_COMMAND_TGINITASTARGET        = (0x8C)
PN532_COMMAND_TGSETGENERALBYTES     = (0x92)
PN532_COMMAND_TGGETDATA             = (0x86)
PN532_COMMAND_TGSETDATA             = (0x8E)
PN532_COMMAND_TGSETMETADATA         = (0x94)
PN532_COMMAND_TGGETINITIATORCOMMAND = (0x88)
PN532_COMMAND_TGRESPONSETOINITIATOR = (0x90)
PN532_COMMAND_TGGETTARGETSTATUS     = (0x8A)

PN532_RESPONSE_INDATAEXCHANGE       = (0x41)
PN532_RESPONSE_INLISTPASSIVETARGET  = (0x4B)

# Baud Rate selectors
PN532_MIFARE_ISO14443A_106KBPS      = (0x00)
PN532_FELICA_212KBPS                = (0x01)
PN532_FELICA_424KBPS                = (0x02)
PN532_MIFARE_ISO14443B_106KBPS      = (0x03)
PN532_JEWEL_106KBPS                 = (0x04)

# Mifare Commands
MIFARE_CMD_AUTH_A                   = (0x60)
MIFARE_CMD_AUTH_B                   = (0x61)
MIFARE_CMD_READ                     = (0x30)
MIFARE_CMD_WRITE                    = (0xA0)
MIFARE_CMD_WRITE_ULTRALIGHT         = (0xA2)
MIFARE_CMD_TRANSFER                 = (0xB0)
MIFARE_CMD_DECREMENT                = (0xC0)
MIFARE_CMD_INCREMENT                = (0xC1)
MIFARE_CMD_STORE                    = (0xC2)

# FeliCa Commands
FELICA_CMD_POLLING                  = (0x00)
FELICA_CMD_REQUEST_SERVICE          = (0x02)
FELICA_CMD_REQUEST_RESPONSE         = (0x04)
FELICA_CMD_READ_WITHOUT_ENCRYPTION  = (0x06)
FELICA_CMD_WRITE_WITHOUT_ENCRYPTION = (0x08)
FELICA_CMD_REQUEST_SYSTEM_CODE      = (0x0C)

# Prefixes for NDEF Records (to identify record type)
NDEF_URIPREFIX_NONE                 = (0x00)
NDEF_URIPREFIX_HTTP_WWWDOT          = (0x01)
NDEF_URIPREFIX_HTTPS_WWWDOT         = (0x02)
NDEF_URIPREFIX_HTTP                 = (0x03)
NDEF_URIPREFIX_HTTPS                = (0x04)
NDEF_URIPREFIX_TEL                  = (0x05)
NDEF_URIPREFIX_MAILTO               = (0x06)
NDEF_URIPREFIX_FTP_ANONAT           = (0x07)
NDEF_URIPREFIX_FTP_FTPDOT           = (0x08)
NDEF_URIPREFIX_FTPS                 = (0x09)
NDEF_URIPREFIX_SFTP                 = (0x0A)
NDEF_URIPREFIX_SMB                  = (0x0B)
NDEF_URIPREFIX_NFS                  = (0x0C)
NDEF_URIPREFIX_FTP                  = (0x0D)
NDEF_URIPREFIX_DAV                  = (0x0E)
NDEF_URIPREFIX_NEWS                 = (0x0F)
NDEF_URIPREFIX_TELNET               = (0x10)
NDEF_URIPREFIX_IMAP                 = (0x11)
NDEF_URIPREFIX_RTSP                 = (0x12)
NDEF_URIPREFIX_URN                  = (0x13)
NDEF_URIPREFIX_POP                  = (0x14)
NDEF_URIPREFIX_SIP                  = (0x15)
NDEF_URIPREFIX_SIPS                 = (0x16)
NDEF_URIPREFIX_TFTP                 = (0x17)
NDEF_URIPREFIX_BTSPP                = (0x18)
NDEF_URIPREFIX_BTL2CAP              = (0x19)
NDEF_URIPREFIX_BTGOEP               = (0x1A)
NDEF_URIPREFIX_TCPOBEX              = (0x1B)
NDEF_URIPREFIX_IRDAOBEX             = (0x1C)
NDEF_URIPREFIX_FILE                 = (0x1D)
NDEF_URIPREFIX_URN_EPC_ID           = (0x1E)
NDEF_URIPREFIX_URN_EPC_TAG          = (0x1F)
NDEF_URIPREFIX_URN_EPC_PAT          = (0x20)
NDEF_URIPREFIX_URN_EPC_RAW          = (0x21)
NDEF_URIPREFIX_URN_EPC              = (0x22)
NDEF_URIPREFIX_URN_NFC              = (0x23)

PN532_GPIO_VALIDATIONBIT            = (0x80)
PN532_GPIO_P30                      = (0)
PN532_GPIO_P31                      = (1)
PN532_GPIO_P32                      = (2)
PN532_GPIO_P33                      = (3)
PN532_GPIO_P34                      = (4)
PN532_GPIO_P35                      = (5)

# FeliCa consts
FELICA_READ_MAX_SERVICE_NUM         = 16
FELICA_READ_MAX_BLOCK_NUM           = 12 # for typical FeliCa card
FELICA_WRITE_MAX_SERVICE_NUM        = 16
FELICA_WRITE_MAX_BLOCK_NUM          = 10 # for typical FeliCa card
FELICA_REQ_SERVICE_MAX_NODE_NUM     = 32

DEBUG = False
_felicaIDm = bytearray() # FeliCa IDm (NFCID2)
_felicaPMm = bytearray() # FeliCa PMm (PAD)

BitReverseTable = bytearray([
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
])

def spi_write(data: bytearray):
    for i in range( len(data) ):
        data[i] = BitReverseTable[ data[i] ]
    time.sleep(.0005)    # sleep 0.5 ms
    data = spi.transfer( data )
    for i in range( len(data) ):
        data[i] = BitReverseTable[ data[i] ]
    time.sleep(.0005)    # sleep 0.5 ms
    return data

def DMSG(msg):
    if DEBUG:
        print(msg)

def DMSG_HEX(char):
    if DEBUG:
        print('%x' % char)

def PrintHex(data: bytearray):
    """
        Prints a hexadecimal value in plain characters

        :param data:      data to print
    """
    print(" ".join('{:02X}'.format(x) for x in data))

def PrintHexChar(data: bytearray, numBytes: int):
    """
    Prints a hexadecimal value in plain characters, along with
        the char equivalents in the following format

        00 00 00 00 00 00  ......

    :param data: data to print
    """
    print(" ".join('{:02X}'.format(x) for x in data), end =" ")

    print("    ", end =" ")
    for i in range(numBytes):
        c = data[i]
        if (c <= 0x1f or c > 0x7f):
            print(".", end =" ")
        else:
            print("%c" % c, end =" ")

    print("")

def begin():
    global _command
    global inListedTag
    _command = 0
    inListedTag = 0 # Tg number of inlisted tag.
    msg = bytearray( [0] * 256 )
    spi_write(msg)

def _isReady():
    time.sleep(.001)    # sleep 1 ms
    msg = bytearray([STATUS_READ, 0])
    data = spi_write(msg)
    return data[1] & 0x1

def _readAckFrame():
    time.sleep(.001)    # sleep 1 ms
    PN532_ACK = [0, 0, 0xFF, 0, 0xFF, 0]

    t = 0
    while 1:
        if _isReady():
            break
        t+=1
        if (t > PN532_ACK_WAIT_TIME):
            DMSG("Time out when waiting for ACK\n")
            return PN532_TIMEOUT

    DMSG("ready at : ")
    DMSG(time.time())
    DMSG('\n')

    msg = bytearray([DATA_READ,0,0,0,0,0,0])
    data = spi_write(msg)

    ackBuf = list(data[1:])

    if ackBuf != PN532_ACK:
        DMSG("Invalid ACK {}\n".format(ackBuf))
        return PN532_INVALID_ACK

    return 0

def writeCommand( header: bytearray, body: bytearray = bytearray()):
    global _command 
    _command = header[0]
    # data_out = bytearray( len(header) + len(body) + 9 ) 
    data_out = [ DATA_WRITE, PN532_PREAMBLE, PN532_STARTCODE1, PN532_STARTCODE2 ]

    length = len(header) + len(body) + 1 # length of data field: TFI + DATA
    data_out += [ length ]
    data_out += [ (~length & 0xFF) + 1 ]     # checksum of length

    data_out += [ PN532_HOSTTOPN532 ]
    dsum = PN532_HOSTTOPN532 + sum(header) + sum(body)  # sum of TFI + DATA

    for i in range( len(header) ):
        data_out += [ header[ i ] ]
    for i in range( len(body) ):
        data_out += [ body[ i ] ]
    checksum = ((~dsum & 0xFF) + 1) & 0xFF # checksum of TFI + DATA

    data_out += [ checksum ]
    data_out += [ PN532_POSTAMBLE ]

    DMSG("writeCommand: {}    {}    {}".format(header, body, data_out))

    # send data
    msg = bytearray(data_out)
    spi_write(msg)

    return _readAckFrame()

def readResponse( timeout: int = 1000): # -> (int, bytearray):
    global _command 
    t = 0
    buf = bytearray()

    # if length < 0:
    #     return length, buf

    t = 0
    while 1:
        if _isReady():
            break
        time.sleep(.001)    # sleep 1 ms
        t+=1
        if (t > timeout):
            DMSG("Time out when waiting for ACK\n")
            return PN532_TIMEOUT, buf

    DMSG("ready at : ")
    DMSG(time.time())
    DMSG('\n')

    # [RDY] 00 00 FF LEN LCS (TFI PD0 ... PDn) DCS 00
    msg = bytearray([DATA_READ, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
    data = spi_write(msg)

    if (PN532_PREAMBLE != data[1] or # PREAMBLE
        PN532_STARTCODE1 != data[2] or # STARTCODE1
        PN532_STARTCODE2 != data[3]    # STARTCODE2
    ):
        DMSG('Invalid Response frame: {}'.format(data))
        return PN532_INVALID_FRAME, buf

    length = data[4]

    if (0 != (length + data[5] & 0xFF)):
     # checksum of length
        DMSG('Invalid Length Checksum: len {:d} checksum {:d}'.format(length, data[5]))
        return PN532_INVALID_FRAME, buf

    cmd = _command + 1 # response command
    if (PN532_PN532TOHOST != data[6] or cmd != data[7]):
        return PN532_INVALID_FRAME, buf

    DMSG("readResponse read command:  {:x}".format(cmd))

    dsum = PN532_PN532TOHOST + cmd
    buf = data[8:length+6]
    DMSG('readResponse response: {!r}\n'.format(buf))
    dsum += sum(buf)

    checksum = data[length+6]
    if (0 != (dsum + checksum) & 0xFF):
        DMSG("checksum is not ok: sum {:d} checksum {:d}\n".format(dsum, checksum))
        return PN532_INVALID_FRAME, buf
    # POSTAMBLE data [length+7]

    return length, buf

def getFirmwareVersion():
    """
    Checks the firmware version of the PN5xx chip

    :returns:  The chip's firmware version and ID
    """
    if (writeCommand(bytearray([PN532_COMMAND_GETFIRMWAREVERSION]))):
        return 0

    # read data packet
    status, response = readResponse()
    if (status < 0):
        return 0

    # response = pn532_packetbuffer[0]
    # response <<= 8
    # response |= pn532_packetbuffer[1]
    # response <<= 8
    # response |= pn532_packetbuffer[2]
    # response <<= 8
    # response |= pn532_packetbuffer[3]

    return int.from_bytes(response, byteorder='big')

def readRegister( reg: int):
    """
    Read a PN532 register.

    :param reg:  the 16-bit register address.

    :returns:  The register value.
    """
    header = bytearray([PN532_COMMAND_READREGISTER, ((reg >> 8) & 0xFF), reg & 0xFF])

    if (writeCommand(header)):
        return 0

    # read data packet
    status, response = readResponse( 1000 )
    if (0 > status):
        return 0

    return response[0]

def writeRegister( reg: int, val: int):
    """
    Write to a PN532 register.

    :param  reg:  the 16-bit register address.
    :param  val:  the 8-bit value to write.

    :returns:  0 for failure, 1 for success.
    """
    header = bytearray([PN532_COMMAND_WRITEREGISTER, ((reg >> 8) & 0xFF), (reg & 0xFF), (val & 0xFf)])

    if (writeCommand(header)):
        return 0
    

    # read data packet
    status, response = readResponse()
    if (0 > status):
        return 0

    return 1

def writeGPIO(  pinstate: int):
    """
    Writes an 8-bit value that sets the state of the PN532's GPIO  (P3)

    :warning: This function is provided exclusively for board testing and
             is dangerous since it will throw an error if any pin other
             than the ones marked "Can be used as GPIO" are modified!  All
             pins that can not be used as GPIO should ALWAYS be left high
             (value = 1) or the system will become unstable and a HW reset
             will be required to recover the PN532.

             pinState[0]  = P30     Can be used as GPIO
             pinState[1]  = P31     Can be used as GPIO
             pinState[2]  = P32     *** RESERVED (Must be 1!) ***
             pinState[3]  = P33     Can be used as GPIO
             pinState[4]  = P34     *** RESERVED (Must be 1!) ***
             pinState[5]  = P35     Can be used as GPIO

    :returns 1 if everything executed properly, 0 for an error
    """

    # Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34)

    # Fill command buffer
    header = bytearray([PN532_COMMAND_WRITEGPIO,
                        (PN532_GPIO_VALIDATIONBIT | pinstate),  # P3 Pins
                        0x00])  # P7 GPIO Pins (not used ... taken by I2C)

    DMSG("Writing P3 GPIO: ")
    DMSG_HEX(header)
    DMSG("\n")

    # Send the WRITEGPIO command (0x0E)
    if (writeCommand(header)):
        return False

    status, response = readResponse()
    return status >= 0

def readGPIO():
    """
        Reads the state of the PN532's GPIO pins (P3)

        :returns: An 8-bit value containing the pin state where:

             pinState[0]  = P30
             pinState[1]  = P31
             pinState[2]  = P32
             pinState[3]  = P33
             pinState[4]  = P34
             pinState[5]  = P35 
    """

    header = bytearray([PN532_COMMAND_READGPIO])

    # Send the READGPIO command (0x0C)
    if (writeCommand(header)):
        return 0x0

    status, response = readResponse()
    # READGPIO response without prefix and suffix should be in the following format:
    # 
    #   byte            Description
    #   -------------   ------------------------------------------
    #   b0              P3 GPIO Pins
    #   b1              P7 GPIO Pins (not used ... taken by I2C)
    #   b2              Interface Mode Pins (not used ... bus select pins)

    DMSG("P3 GPIO: ") 
    DMSG_HEX(response[0])
    DMSG("P7 GPIO: ") 
    DMSG_HEX(response[1])
    DMSG("I0I1 GPIO: ") 
    DMSG_HEX(response[2])
    DMSG("\n")

    return response[0]

def SAMConfig():
    """
    Configures the SAM (Secure Access Module)
    :returns: True if success, False if error
    """
    header = bytearray([PN532_COMMAND_SAMCONFIGURATION,
                        0x01,   # normal mode
                        0x14,   # timeout 50ms * 20 = 1 second
                        0x01])  # use IRQ pin!

    DMSG("SAMConfig\n")
    if (writeCommand(header)):
        return False

    status, response = readResponse()
    return status >= 0


def setPassiveActivationRetries( maxRetries: int):
    """
    Sets the MxRtyPassiveActivation uint8_t of the RFConfiguration register

    :param  maxRetries:    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries

    :returns: True if everything executed properly, False for an error
    """
    header = bytearray([PN532_COMMAND_RFCONFIGURATION,
                        5,  # Config item 5 (MaxRetries)
                        0xFF,  # MxRtyATR (default = 0xFF)
                        0x01,  # MxRtyPSL (default = 0x01)
                        maxRetries & 0xFF,
                        ])

    if (writeCommand(header)):
        return False  # no ACK

    status, response = readResponse()
    return (status >=  0)

def setRFField( autoRFCA: bool, RFOn: bool):
    """
    Sets the RFon/off uint8_t of the RFConfiguration register

    :param  autoRFCA:    False: No check of the external field before
                        activation

                        True: Check the external field before
                        activation

    :param  RFOn:    False Switch the RF field off, True: switch the RF
                    field on

    :returns:    True if everything executed properly, False for an error
    """
    header = bytearray([
        PN532_COMMAND_RFCONFIGURATION,
        1,
        (0x2 if autoRFCA else 0) | (0x1 if RFOn else 0)
    ])
    if(writeCommand(header)):
        return False  # no ACK

    status, response = readResponse()
    return (status >= 0)

# **** ISO14443A Commands *****

def readPassiveTargetID( cardbaudrate: int, timeout: int = 1000, inlist: bool = False): #  -> (bool, bytearray):
    global inListedTag
    """
    Waits for an ISO14443A target to enter the field

    :param  cardBaudRate:  Baud rate of the card
    :param  timeout:       The number of tries before timing out
    :param  inlist:        If set to True, the card will be inlisted

    :returns: (True if successful, uid of the card)
    """
    header = bytearray([
        PN532_COMMAND_INLISTPASSIVETARGET,
        1,  # max 1 cards at once (we can set this to 2 later)
        cardbaudrate & 0xFF,
    ])
    if (writeCommand(header)) :
        return False, bytearray()  # command failed


    # read data packet
    status, response = readResponse( timeout )
    if (status < 0):
        return False, bytearray()
    
    # check some basic stuff
    # ISO14443A card response should be in the following format:

      # byte            Description
      # -------------   ------------------------------------------
      # b0              Tags Found
      # b1              Tag Number (only one used in this example)
      # b2..3           SENS_RES
      # b4              SEL_RES
      # b5              NFCID Length
      # b6..NFCIDLen    NFCID

    if (response[0] != 1):
        return False, bytearray()

    sens_res = response[2]
    sens_res <<= 8
    sens_res |= response[3]

    DMSG("ATQA: 0x")
    DMSG_HEX(sens_res)
    DMSG("SAK: 0x")
    DMSG_HEX(response[4])
    DMSG("\n")

    # Card appears to be Mifare Classic 
    uidLength = response[5]
    uid = bytearray(response[6:6 + uidLength])

    if (inlist) :
        inListedTag = response[1]
    
    return True, uid

# **** Mifare Classic Functions *****

def mifareclassic_IsFirstBlock ( uiBlock: int):
    """
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector) 
    """

    # Test if we are in the small or big sectors
    if (uiBlock < 128):
        return ((uiBlock) % 4 == 0)
    else:
        return ((uiBlock) % 16 == 0)

def mifareclassic_IsTrailerBlock( uiBlock: int):
    """
    Indicates whether the specified block number is the sector trailer
    """

    # Test if we are in the small or big sectors
    if (uiBlock < 128):
        return ((uiBlock + 1) % 4 == 0)
    else:
        return ((uiBlock + 1) % 16 == 0)

def mifareclassic_AuthenticateBlock( uid: bytearray, blockNumber: int, keyNumber: int, keyData: bytearray):
    """
            Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    :param  uid:           Pointer to a byte array containing the card UID
    :param  blockNumber:   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    :param  keyNumber:     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    :param  keyData:       Pointer to a byte array containing the 6 bytes
                          key value

    :returns: True if everything executed properly, False for an error
    """

    # Hang on to the key and uid data
    _key = keyData
    _uid = uid

    # Prepare the authentication command #
    header = bytearray([PN532_COMMAND_INDATAEXCHANGE,
              1,
              MIFARE_CMD_AUTH_B if keyNumber else MIFARE_CMD_AUTH_A,
              blockNumber])
    header += _key[:6] + _uid

    if (writeCommand(header)):
        return False

    # Read the response packet
    status, response = readResponse()

    # Check if the response is valid and we are authenticated???
    # for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    # Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
    if (status < 0 or response[0] != 0x00):
        DMSG("Authentication failed\n")
        return False

    return True

def mifareclassic_ReadDataBlock ( blockNumber: int): # -> (bool, bytearray):
    """
    Tries to read an entire 16-bytes data block at the specified block
    address.

    :param  blockNumber:   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    :param  data:          Pointer to the byte array that will hold the
                          retrieved data (if any)

    :returns: tuple (result, data)
        result: bool True if operation was successful, False if error
        data: bytearray data read
    """

    DMSG("Trying to read 16 bytes from block ")
    DMSG(blockNumber)

    #  Prepare the command
    header = bytearray([
        PN532_COMMAND_INDATAEXCHANGE,
        1,                  # Card number
        MIFARE_CMD_READ,    # Mifare Read command = 0x30
        blockNumber,        # Block Number (0..63 for 1K, 0..255 for 4K)
    ])
    #  Send the command 
    if (writeCommand(header)):
        return False, bytearray()
    

    #  Read the response packet 
    status, response = readResponse()

    #  If byte 8 isn't 0x00 we probably have an error 
    if (status < 0 or response[0] != 0x00):
        DMSG("Authentication failed\n")
        return False, bytearray()

    #  Copy the 16 data bytes to the output buffer        
    #  Block content starts at byte 9 of a valid response
    return True, response[1:17]

def mifareclassic_WriteDataBlock ( blockNumber: int, data: bytearray):
    """
            Tries to write an entire 16-bytes data block at the specified block
    address.

    :param  blockNumber:   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    :param  data:          The byte array that contains the data to write.

    :returns: True if everything executed properly, False for an error
    """

    #  Prepare the first command
    header = bytearray([PN532_COMMAND_INDATAEXCHANGE, 1, MIFARE_CMD_WRITE, blockNumber]) + data[:16]

    #  Send the command 
    if (writeCommand(header)):
        return False
    
    #  Read the response packet
    status, response = readResponse()

    return (status >= 0)

def mifareclassic_FormatNDEF ():
    """
            Formats a Mifare Classic card to store NDEF Records

    :returns: True if everything executed properly, False for an error
    """

    sectorbuffer1 = bytearray([0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1])
    sectorbuffer2 = bytearray([0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1])
    sectorbuffer3 = bytearray([0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])

    # Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
    # for the MAD sector in NDEF records (sector 0)

    # Write block 1 and 2 to the card
    if (not (mifareclassic_WriteDataBlock(1, sectorbuffer1))):
        return False
    if (not (mifareclassic_WriteDataBlock(2, sectorbuffer2))):
        return False
    # Write key A and access rights card
    if (not (mifareclassic_WriteDataBlock(3, sectorbuffer3))):
        return False

    # Seems that everything was OK (?!)
    return True

def mifareclassic_WriteNDEFURI ( sectorNumber: int, uriIdentifier: int, url: str):
    """
    Writes an NDEF URI Record to the specified sector (1..15)

    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    :param  sectorNumber:  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    :param  uriIdentifier: The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    :param  url:           The uri text to write (max 38 characters).

    :return: True if everything executed properly, False for an error
    """

    # Figure out how long the string is
    url_bytes = bytearray(url, 'ascii')
    length = len(url_bytes)

    # Make sure we're within a 1K limit for the sector number
    if ((sectorNumber < 1) or (sectorNumber > 15)):
        return False

    # Make sure the URI payload is between 1 and 38 chars
    if ((length < 1) or (length > 38)):
        return False

    # Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
    # in NDEF records

    # Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    sectorbuffer1 = bytearray([0x00, 0x00, 0x03, length + 5, 0xD1, 0x01, length + 1, 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    sectorbuffer2 = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    sectorbuffer3 = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    sectorbuffer4 = bytearray([0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
    # TODO: This can probably be simplified...
    if (length <= 6) :
        # Unlikely we'll get a url this short, but why not ...
        sectorbuffer1 = sectorbuffer1[:9] + url_bytes[:length] + sectorbuffer1[9 + length:]
        sectorbuffer1[length + 9] = 0xFE
    elif (length == 7):
        # 0xFE needs to be wrapped around to next block
        sectorbuffer1 = sectorbuffer1[:9] + url_bytes[:length] + sectorbuffer1[9 + length:]
        sectorbuffer2[0] = 0xFE
    elif ((length > 7) and (length <= 22)):
        # Url fits in two blocks
        sectorbuffer1 = sectorbuffer1[:9] + url_bytes[:7] + sectorbuffer1[9 + 7:]
        sectorbuffer2 = url_bytes[7:] + sectorbuffer2[length - 7:]
        sectorbuffer2[length - 7] = 0xFE
    elif (length == 23):
        # 0xFE needs to be wrapped around to final block
        sectorbuffer1 = sectorbuffer1[:9] + url_bytes[:7] + sectorbuffer1[9 + 7:]
        sectorbuffer2 = url_bytes[7:]
        sectorbuffer3[0] = 0xFE
    else:
        # Url fits in three blocks
        sectorbuffer1 = sectorbuffer1[:9] + url_bytes[:7] + sectorbuffer1[9 + 7:]
        sectorbuffer2 = url_bytes[7:23]
        sectorbuffer3 = url_bytes[23:] + sectorbuffer3[length - 23:]
        sectorbuffer3[length - 23] = 0xFE

    # Now write all three blocks back to the card
    if (not (mifareclassic_WriteDataBlock(sectorNumber * 4, sectorbuffer1))):
        return False
    if (not (mifareclassic_WriteDataBlock((sectorNumber * 4) + 1, sectorbuffer2))):
        return False
    if (not (mifareclassic_WriteDataBlock((sectorNumber * 4) + 2, sectorbuffer3))):
        return False
    if (not (mifareclassic_WriteDataBlock((sectorNumber * 4) + 3, sectorbuffer4))):
        return False

    # Seems that everything was OK (?!)
    return True

# **** Mifare Ultralight Functions *****

def mifareultralight_ReadPage( page: int): # -> (bool, bytearray):
    """
            Tries to read an entire 4-bytes page at the specified address.

    :param  page:        The page number (0..63 in most cases)
    :returns: (result, data)
            result: bool True if successful, False if error
            data: bytearray received page data
    """

    #  Prepare the command
    header = bytearray([
        PN532_COMMAND_INDATAEXCHANGE,
        1,                   #  Card number
        MIFARE_CMD_READ,     #  Mifare Read command = 0x30
        page,                #  Page Number (0..63 in most cases)
    ])
    #  Send the command 
    if (writeCommand(header)):
        return False, bytearray()
    

    #  Read the response packet 
    status, response = readResponse()

    #  If byte 8 isn't 0x00 we probably have an error
    if (status < 0 or response[0] != 0x00):
        DMSG("Authentication failed\n")
        return False, bytearray()

    #  Copy the 4 data bytes to the output buffer
    #  Block content starts at byte 9 of a valid response
    #  Note that the command actually reads 16 bytes or 4
    #  pages at a time ... we simply discard the last 12
    #  bytes
    data = response[1:5]
    return True, data

def mifareultralight_WritePage( page: int, buffer: bytearray):
    """
    Tries to write an entire 4-bytes data buffer at the specified page
    address.

    :param  page:     The page number to write into.  (0..63).
    :param  buffer:   The byte array that contains the data to write.

    :returns: True if everything executed properly, False for an error
    """

    #  Prepare the first command 
    header = bytearray([PN532_COMMAND_INDATAEXCHANGE, 1, MIFARE_CMD_WRITE_ULTRALIGHT, page])
    header += buffer[:4]

    #  Send the command 
    if (writeCommand(header)):
        return False

    #  Read the response packet
    status, response = readResponse()
    return status >= 0

def inDataExchange( send: bytearray): # -> (bool, bytearray):
    global inListedTag
    """
            Exchanges an APDU with the currently inlisted peer

    :param  send:            Pointer to data to send
    :param  response:        Pointer to response data
    :param  responseLength:  Pointer to the response data length
    """

    header = bytearray([
        0x40,  # PN532_COMMAND_INDATAEXCHANGE
        inListedTag
    ])

    if (writeCommand(header, send)):
        return False, bytearray()
    

    status, response = readResponse()
    if (status < 0):
        return False, bytearray()
    

    if ((response[0] & 0x3f) != 0):
        DMSG("Status code indicates an error\n")
        return False, bytearray()

    response = response[1:]
    return True, response

def inListPassiveTarget():
    global inListedTag
    """
        'InLists' a passive target. PN532 acting as reader/initiator,
        peer acting as card/responder.
        :returns: True if command succeeded, False otherwise
    """
    header = bytearray([
        PN532_COMMAND_INLISTPASSIVETARGET,
        1,
        0,
    ])
    DMSG("inList passive target\n")

    if (writeCommand(header)):
        return False

    status, response = readResponse()
    if (status < 0 or response[0] != 1):
        return False
    
    inListedTag = response[1]

    return True

def tgInitAsTarget( command: bytearray, timeout: int):

    status = writeCommand(command)
    if (status < 0):
        return -1

    status, response = readResponse(timeout)
    if (status > 0):
        return 1
    elif(PN532_TIMEOUT == status):
        return 0
    else:
        return -2

def tgInitAsTargetP2P( timeout: int):
    """
     * Peer to Peer

     """

    command = bytearray([
        PN532_COMMAND_TGINITASTARGET,
        0,
        0x00, 0x00,  # SENS_RES
        0x00, 0x00, 0x00,  # NFCID1
        0x40,  # SEL_RES

        0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89,  # POL_RES
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF,

        0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, 0x00, 0x00,  # NFCID3t: Change this to desired value

        0x0a, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x10, 0x02, 0x02, 0x00, 0x80,
        # LLCP magic number, version parameter and MIUX
        0x00])

    return tgInitAsTarget(command, timeout)

def tgGetData(): # -> (int, bytearray):

    header = bytearray([PN532_COMMAND_TGGETDATA])

    if (writeCommand(header)):
        return -1, bytearray()
    

    status, response = readResponse()
    if (0 >= status):
        return status, bytearray()

    length = status - 1

    if (response[0] != 0):
        DMSG("status is not ok\n")
        return -5, bytearray()

    response = response[1:]
    return length, response


def tgSetData( header: bytearray, body: bytearray = bytearray()):
    header = bytearray([PN532_COMMAND_TGSETDATA]) + header

    if (writeCommand(header, body)):
        return False

    status, response = readResponse()
    if (0 > status):
        return False

    if (0 != response[0]):
        return False

    return True

def inRelease( relevantTarget: int = 0):
    header = bytearray([
        PN532_COMMAND_INRELEASE,
        relevantTarget,
    ])
    if (writeCommand(header)):
        return False

    # read data packet
    status, response = readResponse()
    return status >= 0

def felica_Polling( systemCode: int, requestCode: int, timeout: int = 1000): # -> (int, bytearray, bytearray, int):
    global inListedTag
    """
        Poll FeliCa card. PN532 acting as reader/initiator,
        peer acting as card/responder.
        :param timeout:
        :param  systemCode:             Designation of System Code. When sending FFFFh as System Code,
                                           all FeliCa cards can return response.
        :param  requestCode:            Designation of Request Data as follows:
                                             00h: No Request
                                             01h: System Code request (to acquire System Code of the card)
                                             02h: Communication performance request
        :returns: (status, idm, pwm, systemCodeResponse)
                    status                 0 = no card, 1 = FeliCa card detected, <0 = error
                    idm                    IDm of the card (8 bytes)
                    pmm                    PMm of the card (8 bytes)
                    systemCodeResponse     System Code of the card (Optional, 2bytes)
    """
    header = bytearray([
    PN532_COMMAND_INLISTPASSIVETARGET,
    1,
    1,
    FELICA_CMD_POLLING,
    (systemCode >> 8) & 0xFF,
    systemCode & 0xFF,
    requestCode & 0xFF,
    0,
    ])
    no_data = bytearray()

    if (writeCommand(header)):
        DMSG("Could not send Polling command\n")
        return -1, no_data, no_data, 0

    status, response = readResponse(timeout)
    if (status < 0):
        DMSG("Could not receive response\n")
        return -2, no_data, no_data, 0

    # Check NbTg (response[7])
    if (response[0] == 0):
        DMSG("No card had detected\n")
        return 0, no_data, no_data, 0
    elif (response[0] != 1):
        DMSG("Unhandled number of targets inlisted. NbTg: ")
        DMSG_HEX(response[7])
        DMSG("\n")
        return -3, no_data, no_data, 0

    inListedTag = response[1]
    DMSG("Tag number: ")
    DMSG_HEX(response[1])
    DMSG("\n")

    # length check
    responseLength = response[2]
    if (responseLength != 18 and responseLength != 20):
        DMSG("Wrong response length\n")
        return -4, no_data, no_data, 0

    idm = response[4:12]
    pwm = response[12:24]
    _felicaIDm = idm
    _felicaPMm = pwm

    if (responseLength == 20):
        systemCodeResponse = (response[20] << 8) + response[21]
    else:
        systemCodeResponse = 0

    return 1, idm, pwm, systemCodeResponse

def felica_SendCommand( command: bytearray): # -> (int, bytearray):
    """
        Sends FeliCa command to the currently inlisted peer

        :param  command:         FeliCa command packet. (e.g. 00 FF FF 00 00  for Polling command)
        :returns:  (status, response)
                    status 1: Success, < 0: error
                    response: FeliCa response packet. (e.g. 01 NFCID2(8 bytes) PAD(8 bytes)  for Polling response)
    """
    commandlength = len(command)
    no_data = bytearray()

    if (commandlength > 0xFE):
        DMSG("Command length too long\n")
        return -1, no_data

    header = bytearray([
        PN532_COMMAND_INDATAEXCHANGE,
        inListedTag,
        commandlength + 1,
    ])
    if (writeCommand(header, command)):
        DMSG("Could not send FeliCa command\n")
        return -2, no_data

    # Wait card response
    status, response = readResponse()
    if (status < 0):
        DMSG("Could not receive response\n")
        return -3, no_data

    # Check status (response[0])
    if ((response[0] & 0x3F) != 0):
        DMSG("Status code indicates an error: ")
        DMSG_HEX(response[0])
        DMSG("\n")
        return -4, no_data

    # length check
    responseLength = response[1] - 1
    if ((status - 2) !=  responseLength):
        DMSG("Wrong response length\n")
        return -5, no_data

    response_data = response[2: 2 + responseLength]
    

    return 1, response_data
"""
def felica_RequestService( nodeCodeList: List[int]): # -> (int, List[int]):
        Sends FeliCa Request Service command

        :param  nodeCodeList:      Node codes(Big Endian)
        :returns:   (status, keyVersions)
                    status      1: Success, < 0: error
                    keyVersions Key Version of each Node (Big Endian)
    no_data = []
    numNode = len(nodeCodeList)
    if (numNode > FELICA_REQ_SERVICE_MAX_NODE_NUM):
        DMSG("numNode is too large\n")
        return -1, no_data

    cmd = bytearray([FELICA_CMD_REQUEST_SERVICE]) + _felicaIDm[:8] + bytearray([numNode])
    for i in range(numNode):
        cmd.append(nodeCodeList[i] & 0xFF)
        cmd.append((nodeCodeList[i] >> 8) & 0xff)

    status, response = felica_SendCommand(cmd)
    if (status != 1):
        DMSG("Request Service command failed\n")
        return -2, no_data

    # length check
    responseLength = len(response)
    if (responseLength != 10 + 2 * numNode):
        DMSG("Request Service command failed (wrong response length)\n")
        return -3, no_data

    keyVersions = []
    for i in range(numNode):
        keyVersions.append(response[10 + i * 2] + (response[10 + i * 2 + 1] << 8))

    return 1, keyVersions
"""

def felica_RequestResponse(): # -> (int, int):
    """
    Sends FeliCa Request Response command

    :returns:     (status, mode)
                status  1: Success, < 0: error
                mode    Current Mode of the card
    """

    cmd = bytearray([FELICA_CMD_REQUEST_RESPONSE]) + _felicaIDm[:8]

    status, response = felica_SendCommand(cmd)
    responseLength = len(response)
    if (status != 1):
        DMSG("Request Response command failed\n")
        return -1, -1

    # length check
    if (responseLength != 10):
        DMSG("Request Response command failed (wrong response length)\n")
        return -2, -1

    mode = response[9]
    return 1, mode
"""
def felica_ReadWithoutEncryption( serviceCodeList: List[int], blockList: List[int]): # -> (int, List[bytearray]):

        Sends FeliCa Read Without Encryption command

        :param  serviceCodeList:    Service Code List (Big Endian)
        :param  blockList:          Block List (Big Endian, This API only accepts 2-byte block list element)
        :returns:       (status, blockData)
                          status    1: Success, < 0: error
                          blockData Block Data
    no_data = []
    numService =  len(serviceCodeList)
    if (numService > FELICA_READ_MAX_SERVICE_NUM):
        DMSG("numService is too large\n")
        return -1, no_data

    numBlock = len(blockList)
    if (numBlock > FELICA_READ_MAX_BLOCK_NUM):
        DMSG("numBlock is too large\n")
        return -2, no_data

    cmd = bytearray([FELICA_CMD_READ_WITHOUT_ENCRYPTION]) + _felicaIDm[:8] + bytearray([numService])
    for i in range(numService):
        cmd.append(serviceCodeList[i] & 0xFF)
        cmd.append((serviceCodeList[i] >> 8) & 0xff)

    cmd.append(numBlock)
    for i in range(numBlock):
        cmd.append((blockList[i] >> 8) & 0xFF)
        cmd.append(blockList[i] & 0xff)

    status, response = felica_SendCommand(cmd)
    if (status != 1):
        DMSG("Read Without Encryption command failed\n")
        return -3, no_data


    # length check
    responseLength = len(response)
    if (responseLength != 12 + 16 * numBlock):
        DMSG("Read Without Encryption command failed (wrong response length)\n")
        return -4, no_data

    # status flag check
    if (response[9] != 0 or response[10] != 0):
        DMSG("Read Without Encryption command failed (Status Flag: ")
        DMSG_HEX(response[9])
        DMSG_HEX(response[10])
        DMSG(")\n")
        return -5, no_data

    k = 12
    blockData = []
    for i in range(numBlock):
        start = 12+ i * 16
        blockData.append(response[start: start + 16])

    return 1, blockData

def felica_WriteWithoutEncryption( serviceCodeList: List[int], blockList: List[int], blockData: List[bytearray]):

        Sends FeliCa Write Without Encryption command

        :param  serviceCodeList:    Service Code List (Big Endian)
        :param  blockList:          Block List (Big Endian, This API only accepts 2-byte block list element)
        :returns:       status    1: Success, < 0: error
    numService, numBlock = len(serviceCodeList), len(blockList)
    if (numService > FELICA_WRITE_MAX_SERVICE_NUM):
        DMSG("numService is too large\n")
        return -1

    if (numBlock > FELICA_WRITE_MAX_BLOCK_NUM):
        DMSG("numBlock is too large\n")
        return -2

    cmd = bytearray([FELICA_CMD_WRITE_WITHOUT_ENCRYPTION]) + _felicaIDm[:8] + bytearray([numService])
    for i in range(numService):
        cmd.append(serviceCodeList[i] & 0xFF)
        cmd.append((serviceCodeList[i] >> 8) & 0xff)

    cmd.append(numBlock)
    for i in range(numBlock):
        cmd.append((blockList[i] >> 8) & 0xFF)
        cmd.append(blockList[i] & 0xff)

    for i in range(numBlock):
        for k in range(16):
            cmd.append(blockData[i][k])

    status, response = felica_SendCommand(cmd)
    responseLength = len(response)
    if (status != 1):
        DMSG("Write Without Encryption command failed\n")
        return -3

    # length check
    if (responseLength != 11):
        DMSG("Write Without Encryption command failed (wrong response length)\n")
        return -4

    # status flag check
    if (response[9] != 0 or response[10] != 0):
        DMSG("Write Without Encryption command failed (Status Flag: ")
        DMSG_HEX(response[9])
        DMSG_HEX(response[10])
        DMSG(")\n")
        return -5

    return 1
"""
def felica_RequestSystemCode(): # -> (int, List[int]):
    """
    Sends FeliCa Request System Code command

    :returns:   (status, systemCodeList)
                status          1: Success, < 0: error
                systemCodeList  System Code list (Array length should longer than 16)

     """

    cmd = bytearray([FELICA_CMD_REQUEST_SYSTEM_CODE]) + _felicaIDm[:8]

    status, response = felica_SendCommand(cmd)
    responseLength = len(response)
    if (status != 1):
        DMSG("Request System Code command failed\n")
        return -1, []

    numSystemCode = response[9]

    # length check
    if (responseLength < 10 + 2 * numSystemCode):
        DMSG("Request System Code command failed (wrong response length)\n")
        return -2, []

    systemCodeList = []
    for i in range(numSystemCode):
        systemCodeList.append((response[10 + i * 2] << 8) + response[10 + i * 2 + 1])

    return 1, systemCodeList

# ************************************************************************
# !

# ************************************************************************
def felica_Release():
    """
    Release FeliCa card
    :returns:   1: Success, < 0: error
    """

    # InRelease
    header = bytearray([
        PN532_COMMAND_INRELEASE,
        0x00,  # All target
    ])

    DMSG("Release all FeliCa target\n")

    if (writeCommand(header)):
        DMSG("No ACK\n")
        return -1  # no ACK

    # Wait card response
    frameLength, response = readResponse()
    if (frameLength < 0):
        DMSG("Could not receive response\n")
        return -2


    # Check status (response[0])
    if ((response[0] & 0x3F)!=0):
        DMSG("Status code indicates an error: ")
        DMSG_HEX(response[7])
        DMSG("\n")
        return -3

    return 1
# -----------------------------------------------------------------
