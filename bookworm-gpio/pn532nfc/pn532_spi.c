
#include "pn532_spi.h"

#define STATUS_READ 2
#define DATA_WRITE 1
#define DATA_READ 3

uint8_t command;
static int spi;

struct spi_ioc_transfer xfer;

uint8_t BitReverseTable[256] =
{
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
};

int spiSetup( void )
{
    uint8_t mode = 0;
    uint32_t speed = 1000000;
    uint8_t bits = 8;

    if ((spi = open( "/dev/spidev0.0", O_RDWR )) < 0) {
        printf("Failed to open the bus.");
        exit( 1 );
    }
    if (ioctl( spi, SPI_IOC_WR_MODE, &mode )<0) {
        printf("can't set spi mode");
        return 1;
    }
    if (ioctl( spi, SPI_IOC_WR_BITS_PER_WORD, &bits )<0) {
        printf("can't set bits per word");
        return 1;
    }
    if (ioctl( spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed )<0) {
        printf("can't set max speed hz");
        return 1;
    }

    xfer.cs_change = 0;
    xfer.delay_usecs = 0;          //delay in us
    xfer.speed_hz = 1000000;       //speed
    xfer.bits_per_word = 8;        // bites per word 8
    return 0;
}

void spi_write( unsigned char *dout, int len )
{
    uint8_t buf[len];
    int status;
    int i;

    for ( i = 0; i < len; i++ )
        buf[i] = BitReverseTable[ dout[i] ];
    xfer.tx_buf = (unsigned long )buf;
    xfer.rx_buf = (unsigned long )buf;
    xfer.len = sizeof buf;
    usleep(100);
    status = ioctl( spi, SPI_IOC_MESSAGE(1), &xfer );
    if ( status < 0 ) {
        printf("SPI_IOC_MESSAGE");
    }
    for ( i = 0; i < len; i++ ) {
        dout[i] = BitReverseTable[ buf[i] ];
    }
    usleep(100);
}

void begin()
{
    int i;
    uint8_t packet[256];

    command = 0;
    spiSetup();
    for ( i = 0; i < 256; i++ ) {
        packet[ i ] = 0x0;
    }
    spi_write( packet, 256 );
}

int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    command = header[0];
    writeFrame(header, hlen, body, blen);

    int timeout = PN532_ACK_WAIT_TIME;
    while (!isReady())
    {
        usleep( 1000 );
        timeout--;
        if (0 == timeout)
        {
            printf("Time out when waiting for ACK\n");
            return -2;
        }
    }
    if (readAckFrame())
    {
        printf("Invalid ACK\n");
        return PN532_INVALID_ACK;
    }

    return 0;
}

int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t i;
    uint16_t time = 0;
    uint8_t packet[ 64 ];

    for ( i = 0; i < 64; i++ ) {
        packet[ i ] = 0x0;
    }
    packet[0] = DATA_READ;

    while (!isReady())
    {
        usleep( 1000 );
        time++;
        if (time > timeout)
        {
            return PN532_TIMEOUT;
        }
    }

    spi_write( packet, 64 );

    int16_t result;
    do
    {
        if (0x00 != packet[1] || // PREAMBLE
            0x00 != packet[2] || // STARTCODE1
            0xFF != packet[3]     // STARTCODE2
        )
        {

            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t length = packet[4];
        //printf("cmd = %x, length = %d\n", command, length);
        if (0 != (uint8_t)(length + packet[5]))
        { // checksum of length
            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t cmd = command + 1; // response command
        if (PN532_PN532TOHOST != packet[6] || (cmd) != packet[7])
        {
            result = PN532_INVALID_FRAME;
            break;
        }

        //printf("read: cmd = %x, length = %d, len = %d\n", cmd, length, len);

        uint8_t sum = PN532_PN532TOHOST + cmd;
        for ( i = 0; i < (length - 2); i++ )
        {
            buf[i] = packet[ i+8 ];
            sum += buf[i];

            //printf("%02x ", buf[i]);
        }
        //printf("\n");

        uint8_t checksum = packet[ i+8 ];
        if (0 != (uint8_t)(sum + checksum))
        {
            printf("checksum is not ok, %x, %x\n", sum, checksum);
            result = PN532_INVALID_FRAME;
            break;
        }
        //read(); // POSTAMBLE

        result = length;
    } while (0);

    return result;
}

bool isReady()
{
    uint8_t packet[ 2 ];
    uint8_t *p = packet;

    *p = STATUS_READ;
    p++;
    *p = 0x0;
    p++;

    spi_write( packet, 2 );

    uint8_t status = packet[1] & 0x1;

    return status;
}

void writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    uint8_t packet[ hlen + blen + 9];
    uint8_t *p = packet;

    *p = DATA_WRITE;
    p++;
    *p = PN532_PREAMBLE;
    p++;
    *p = PN532_STARTCODE1;
    p++;
    *p = PN532_STARTCODE2;
    p++;

    uint8_t length = hlen + blen + 1; // length of data field: TFI + DATA
    *p = length;
    p++;
    *p = ~length + 1; // checksum of length
    p++;

    *p = PN532_HOSTTOPN532;
    p++;
    uint8_t sum = PN532_HOSTTOPN532; // sum of TFI + DATA

    //printf("write: ");

    for (uint8_t i = 0; i < hlen; i++)
    {
        *p = header[i];
        p++;
        sum += header[i];
    }
    for (uint8_t i = 0; i < blen; i++)
    {
        *p = body[i];
        p++;
        sum += body[i];
    }

    uint8_t checksum = ~sum + 1; // checksum of TFI + DATA
    *p = checksum;
    p++;
    *p = PN532_POSTAMBLE;
    p++;

    spi_write( packet, hlen + blen + 9 );

    //printf("\n");
}

int8_t readAckFrame()
{
    int i;
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    uint8_t packet[ sizeof(PN532_ACK) + 1 ];
    uint8_t packet2[ sizeof(PN532_ACK) ];
    uint8_t *p = packet;

    *p = DATA_READ;
    p++;

    for ( i = 0; i < sizeof(PN532_ACK); i++ )
    {
        *p = 0x0;
        p++;
    }

    spi_write( packet, sizeof(PN532_ACK) + 1 );
  
    for ( i = 0; i < sizeof(PN532_ACK); i++ )
    {
        packet2[i] = packet[i+1];
        //printf("%x ", packet2[i]);
    }
    //printf("\n");
    return memcmp(packet2, PN532_ACK, sizeof(PN532_ACK));
}
