
#include "pn532_i2c.h"

uint8_t command;
int fd;

#define STATUS_READ 2
#define DATA_WRITE 1
#define DATA_READ 3

void delay(int numseconds)
{
    int milliseconds = 1000 * numseconds;
    clock_t start_time = clock();
    while (clock() < start_time + milliseconds);
}

void i2c_write( unsigned char *dout, int len )
{
    usleep( 100 );
    ioctl( fd, I2C_SLAVE, 0x24 );
    write( fd, dout, len );
    usleep( 100 );
}

void i2c_read( unsigned char *dout, int len )
{
    usleep( 100 );
    ioctl( fd, I2C_SLAVE, 0x24 );
    read( fd, dout, len );
    usleep( 100 );
}

void begin()
{
    command = 0;
    fd = open( "/dev/i2c-2", O_RDWR );
}

int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    command = header[0];
    writeFrame(header, hlen, body, blen);

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
    uint8_t status = 0;
    uint16_t time = 0;
    uint8_t packet[ 64 ];
    int16_t result;

    while ( status == 0 )
    {
        ioctl( fd, I2C_SLAVE, 0x24 );
        read( fd, packet, 64 );
        status = packet[0] & 0x1;
        if ( status == 0x1 ) {
            uint8_t length = packet[4];
            if (0x00 != packet[1] || // PREAMBLE
                0x00 != packet[2] || // STARTCODE1
                0xFF != packet[3]     // STARTCODE2
            )
            {
                result = PN532_INVALID_FRAME;
                break;
            }

            //printf("cmd = %x, length = %d\n", command, length);
            if (0 != (uint8_t)(packet[4] + packet[5]))
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
        }
        delay(1);
        time++;
        if (time > timeout)
        {
            return PN532_TIMEOUT;
        }
    }

    return result;
}

void writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    uint8_t packet[ hlen + blen + 8];
    uint8_t *p = packet;

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

    i2c_write( packet, hlen + blen + 8 );

    //printf("\n");
}

int8_t readAckFrame()
{
    int i;
    uint8_t status = 0;
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    uint8_t packet[ sizeof(PN532_ACK) + 1 ];
    uint8_t packet2[ sizeof(PN532_ACK) ];

    int timeout = PN532_ACK_WAIT_TIME;
    while ( status == 0 )
    {
        i2c_read( packet, sizeof(PN532_ACK) + 1 );
        status = packet[0] & 0x1;
        delay( 1 );
        timeout--;
        if (0 == timeout)
        {
            printf("Time out when waiting for ACK\n");
            return -2;
        }
    }

    for ( i = 0; i < sizeof(PN532_ACK); i++ )
    {
        packet2[i] = packet[i+1];
        //printf("%x ", packet2[i]);
    }
    //printf("\n");
    return memcmp(packet2, PN532_ACK, sizeof(PN532_ACK));
}
