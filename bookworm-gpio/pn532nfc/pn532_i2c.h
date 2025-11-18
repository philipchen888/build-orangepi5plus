
#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// #include <wiringPiSPI.h>

#define TRUE    (1==1)
#define FALSE   (!TRUE)

// #define SPI_CHAN    0
// #define _ss         5

#define PN532_PREAMBLE                (0x00)
#define PN532_STARTCODE1              (0x00)
#define PN532_STARTCODE2              (0xFF)
#define PN532_POSTAMBLE               (0x00)

#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)

#define PN532_ACK_WAIT_TIME           (100)  // ms, timeout of waiting for ACK

#define PN532_INVALID_ACK             (-1)
#define PN532_TIMEOUT                 (-2)
#define PN532_INVALID_FRAME           (-3)
#define PN532_NO_SPACE                (-4)

void begin( void );
void wakeup( void );
void i2c_write( unsigned char *dout, int len );
void i2c_read( unsigned char *dout, int len );
int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);
int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
void writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);
int8_t readAckFrame( void );

#endif
