/* gpio_test.c
 *
 * Compile:
 *   gcc -o gpio_test gpio_test.c -lgpiod
 *
 * Run:
 *   sudo ./gpio_test
 *
 * Note: keep SPI device path as appropriate (/dev/spidev2.0 in your setup).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <gpiod.h>

#define TRUE    (1==1)
#define FALSE   (!TRUE)

#define CHIP_NAME   "gpiochip1"
#define LED_OFFSET  0   /* global 32 */
#define SW_OFFSET   1   /* global 33 */

//----- UART ---------------------------------------------

char *readLine(int fd) {
    static unsigned char line[256];
    int size;
    int i;
    char *ptr = line;
    while(1) {
        size = read(fd, (unsigned char *)ptr, 255);
        while (size > 0) {
            if (*ptr == '\n') {
                ptr++;
                *ptr = '\0';
                return line;
            }
            ptr++;
            size--;
        }
    }
}

int uart_test( void ) {
    struct termios tty;
    int k;
    int fd;
    speed_t speed;

    fd = open("/dev/ttyS3", O_RDWR | O_NOCTTY );
    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &tty);

    if(cfsetispeed(&tty, B115200) < 0)
    {
        printf("invalid baud rate\n");
        return EXIT_FAILURE;
    }
    tty.c_cflag &= ~(CSIZE | PARENB);
    tty.c_cflag |= (CLOCAL | CREAD | CS8);
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        fprintf (stderr, "error %d from tcsetattr", errno);
        return -1;
    }

    speed = cfgetispeed(&tty);
    printf("input speed: %lu\n", (unsigned long) speed);

    tcflush(fd, TCIFLUSH);

    for ( k = '0'; k <= '~'; k++ ) {
        char i[3] = "9\r\n";
        i[0] = k;
        write(fd, &i[0], sizeof i);
        char *j = readLine(fd);
        printf("%s", j);
    }
    printf("\n");
    close(fd);
    return 0;
}

//------- PWM -------------------------------------------------

void pwm_open( void )
{
    FILE *fp;
    bool isdir;
    char path[100] = "";
    strcpy( path, "/sys/class/pwm/pwmchip2/pwm0" );
    struct stat st = {0};
    if ( !stat(path, &st) ) {
        isdir = S_ISDIR( st.st_mode );
    }
    if ( !isdir ) {
        fp = fopen( "/sys/class/pwm/pwmchip2/export", "w" );
        fprintf( fp, "%d", 0 );
        fclose( fp );
    }
}

void pwm_polarity( void )
{
    FILE *fp;
    fp = fopen( "/sys/class/pwm/pwmchip2/pwm0/polarity", "w" );
    fputs( "normal", fp );
    fclose( fp );
}

void pwm_enable( void )
{
    FILE *fp;
    fp = fopen( "/sys/class/pwm/pwmchip2/pwm0/enable", "w" );
    fprintf( fp, "%d", 1 );
    fclose( fp );
}

void pwm_stop( void )
{
    FILE *fp;
    fp = fopen( "/sys/class/pwm/pwmchip2/pwm0/enable", "w" );
    fprintf( fp, "%d", 0 );
    fclose( fp );
}

void pwm_close( void )
{
    FILE *fp;
    fp = fopen( "/sys/class/pwm/pwmchip2/unexport", "w" );
    fprintf( fp, "%d", 0 );
    fclose( fp );
}

int pwm_freq( int freq )
{
    FILE *fp;
    int pwm_period;
    pwm_period = 1000000000.0 / freq;
    fp = fopen( "/sys/class/pwm/pwmchip2/pwm0/period", "w" );
    fprintf( fp, "%d", pwm_period );
    fclose( fp );
    return ( pwm_period );
}

void pwm_duty( float duty, int pwm_period )
{
    FILE *fp;
    int dutycycle;
    dutycycle = duty * pwm_period;
    fp = fopen( "/sys/class/pwm/pwmchip2/pwm0/duty_cycle", "w" );
    fprintf( fp, "%d", dutycycle );
    fclose( fp );
}

int pwm_led_test( void )
{
    int i;
    int x;
    int pwm_period;
    pwm_open();
    pwm_period = pwm_freq( 60 );
    pwm_duty( 0, pwm_period );
    pwm_polarity();
    pwm_enable();

    for ( i = 0; i < 10; i++ ) {
        for ( x = 0; x <= 100; x = x + 5 ) {
            pwm_duty( x/100.0, pwm_period );
            usleep( 60000 );
        }
        for ( x = 100; x >= 0; x = x - 5 ) {
            pwm_duty( x/100.0, pwm_period );
            usleep( 60000 );
        }
    }

    pwm_stop();
    pwm_close();
    return 0;
}

void tong( int note, int duration )
{
    int pwm_period;
    int extra;
    extra = duration / 5;
    if ( note == 0 ) {
        usleep( duration );
        usleep( extra );
    } else {
        pwm_period = pwm_freq( note );
        pwm_duty( 0.5, pwm_period );
        pwm_polarity();
        pwm_enable();
        usleep( duration );
        pwm_stop();
        usleep( extra );
    }
}

int tongsong( void )
{
    int i;
    static int melody[8] = {262, 196, 196, 220, 196, 0, 247, 262};
    static int noteDurations[8] = {4, 8, 8, 4, 4, 4, 4, 4};
    int noteDuration;

    pwm_open();

    for ( i = 0; i < 8; i++ ) {
        noteDuration = 1500000 / noteDurations[i];
        tong( melody[i], noteDuration );
    }

    pwm_close();
    return 0;
}

int servo( void )
{
    int i;
    int pwm_period;

    pwm_open();
    pwm_period = pwm_freq( 50 );
    pwm_duty( 0.05, pwm_period );
    pwm_polarity();
    pwm_enable();

    for ( i = 0; i < 3; i++ ) {
        pwm_duty( 0.05, pwm_period );
        printf( "0 degree\n" );
        sleep( 1 );
        pwm_duty( 0.075, pwm_period );
        printf( "45 degree\n" );
        sleep( 1 );
        pwm_duty( 0.1, pwm_period );
        printf( "90 degree\n" );
        sleep( 1 );
        pwm_duty( 0.075, pwm_period );
        printf( "45 degree\n" );
        sleep( 1 );
    }

    pwm_stop();
    pwm_close();
    return 0;
}

//------- I2C --------------------------------------

int LCDAddr = 0x27;
int BLEN = 1;

void i2c_write_word( int fd, int data )
{
    int temp = data;
        if ( BLEN == 1 )
                temp |= 0x08;
        else
                temp &= 0xF7;
    unsigned char buf[1] = { temp };

    ioctl( fd, I2C_SLAVE, LCDAddr );
    write( fd, buf, 1 );
}

void i2c_send_command(int fd, int comm){
        int buf;
        // Send bit7-4 firstly
        buf = comm & 0xF0;
        buf |= 0x04;                    // RS = 0, RW = 0, EN = 1
        i2c_write_word(fd, buf);
        usleep(2000);
        buf &= 0xFB;                    // Make EN = 0
        i2c_write_word(fd, buf);

        // Send bit3-0 secondly
        buf = (comm & 0x0F) << 4;
        buf |= 0x04;                    // RS = 0, RW = 0, EN = 1
        i2c_write_word(fd, buf);
        usleep(2000);
        buf &= 0xFB;                    // Make EN = 0
        i2c_write_word(fd, buf);
}

void i2c_send_data(int fd, int data){
        int buf;
        // Send bit7-4 firstly
        buf = data & 0xF0;
        buf |= 0x05;                    // RS = 1, RW = 0, EN = 1
        i2c_write_word(fd, buf);
        usleep(2000);
        buf &= 0xFB;                    // Make EN = 0
        i2c_write_word(fd, buf);

        // Send bit3-0 secondly
        buf = (data & 0x0F) << 4;
        buf |= 0x05;                    // RS = 1, RW = 0, EN = 1
        i2c_write_word(fd, buf);
        usleep(2000);
        buf &= 0xFB;                    // Make EN = 0
        i2c_write_word(fd, buf);
}

void i2c_init(int fd){
        i2c_send_command(fd, 0x33);     // Must initialize to 8-line mode at first
        usleep(5000);
        i2c_send_command(fd, 0x32);     // Then initialize to 4-line mode
        usleep(5000);
        i2c_send_command(fd, 0x28);     // 2 Lines & 5*7 dots
        usleep(5000);
        i2c_send_command(fd, 0x0C);     // Enable display without cursor
        usleep(5000);
        i2c_send_command(fd, 0x01);     // Clear Screen
        usleep(5000);
        i2c_write_word(fd, 0x08);
}

void i2c_clear(int fd){
        i2c_send_command(fd, 0x01);     //clear Screen
}

void i2c_write(int fd, int x, int y, char data[]){
        int addr, i;
        int tmp;
        if (x < 0)  x = 0;
        if (x > 15) x = 15;
        if (y < 0)  y = 0;
        if (y > 3)  y = 3;

        // Move cursor
        if ( y < 2 ) {
            addr = 0x80 + 0x40 * y + x;
        } else {
            addr = 0x80 + 0x40 * (y - 2) + 0x14 + x;
        }
        i2c_send_command(fd, addr);

        tmp = strlen(data);
        for (i = 0; i < tmp; i++){
                i2c_send_data(fd, data[i]);
        }
}

int i2c_lcd_test ( void )
{
    int i;
    int fd;
    printf ("i2c test\n") ;
  
    // 0x62 is rgb, 0x3e is text
    fd = open( "/dev/i2c-2", O_RDWR );
 
    i2c_init(fd);
    i2c_clear(fd);
    sleep( 1 );
    i2c_write(fd, 4, 0, "Hello world !");
    i2c_write(fd, 7, 1, "It works !"); 
    i2c_write(fd, 3, 2, "This is a test.");
    i2c_write(fd, 10, 3, "Bye !");

    close( fd );
    return 0;
}

//----- SPI ----------------------------------------------

static const unsigned char font7x14[] = {
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,  // space
  0X00,0X00,0X00,0XFC,0X00,0X00,0X00,0X00,0X00,0X0D,0X00,0X00,  // exclam
  0X00,0X00,0X1E,0X00,0X1E,0X00,0X00,0X00,0X00,0X00,0X00,0X00,  // quotedbl
  0X00,0X20,0XFC,0X20,0XFC,0X20,0X00,0X01,0X0F,0X01,0X0F,0X01,  // numbersign
  0X30,0X48,0X88,0XFC,0X88,0X30,0X06,0X08,0X08,0X1F,0X08,0X07,  // dollar
  0X18,0X24,0XA4,0X78,0X10,0X0C,0X0C,0X02,0X07,0X09,0X09,0X06,  // percent
  0X00,0XB8,0XC4,0X44,0X38,0X80,0X07,0X08,0X08,0X05,0X06,0X09,  // ampersand
  0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,  // quotesingle
  0X00,0X00,0XE0,0X18,0X04,0X02,0X00,0X00,0X03,0X0C,0X10,0X20,  // parenleft
  0X00,0X02,0X04,0X18,0XE0,0X00,0X00,0X20,0X10,0X0C,0X03,0X00,  // parenright
  0X00,0X20,0X40,0XF0,0X40,0X20,0X00,0X02,0X01,0X07,0X01,0X02,  // asterisk
  0X00,0X80,0X80,0XF0,0X80,0X80,0X00,0X00,0X00,0X07,0X00,0X00,  // plus
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X24,0X1C,0X00,0X00,  // comma
  0X00,0X80,0X80,0X80,0X80,0X80,0X00,0X00,0X00,0X00,0X00,0X00,  // hyphen
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X08,0X1C,0X08,0X00,  // period
  0X00,0X00,0X00,0XE0,0X18,0X06,0X30,0X0C,0X03,0X00,0X00,0X00,  // slash
  0XF0,0X08,0X04,0X04,0X08,0XF0,0X03,0X04,0X08,0X08,0X04,0X03,  // zero
  0X00,0X10,0X08,0XFC,0X00,0X00,0X00,0X08,0X08,0X0F,0X08,0X08,  // one
  0X18,0X04,0X04,0X04,0XC4,0X38,0X08,0X0C,0X0A,0X09,0X08,0X08,  // two
  0X04,0X04,0X44,0X64,0X54,0X8C,0X06,0X08,0X08,0X08,0X08,0X07,  // three
  0X00,0XC0,0X30,0X08,0XFC,0X00,0X03,0X02,0X02,0X02,0X0F,0X02,  // four
  0X7C,0X24,0X24,0X24,0X24,0XC4,0X06,0X08,0X08,0X08,0X08,0X07,  // five
  0XF0,0X88,0X44,0X44,0X44,0X80,0X07,0X08,0X08,0X08,0X08,0X07,  // six
  0X04,0X04,0X04,0XC4,0X34,0X0C,0X00,0X0C,0X03,0X00,0X00,0X00,  // seven
  0X18,0XA4,0X44,0X44,0XA4,0X18,0X07,0X08,0X08,0X08,0X08,0X07,  // eight
  0X78,0X84,0X84,0X84,0X44,0XF8,0X06,0X08,0X08,0X08,0X04,0X03,  // nine
  0X00,0X00,0X20,0X70,0X20,0X00,0X00,0X00,0X04,0X0E,0X04,0X00,  // colon
  0X00,0X00,0X60,0X60,0X00,0X00,0X00,0X00,0X12,0X0E,0X00,0X00,  // semicolon
  0X00,0X80,0X40,0X20,0X10,0X08,0X00,0X00,0X01,0X02,0X04,0X08,  // less
  0X20,0X20,0X20,0X20,0X20,0X20,0X01,0X01,0X01,0X01,0X01,0X01,  // equal
  0X00,0X08,0X10,0X20,0X40,0X80,0X00,0X08,0X04,0X02,0X01,0X00,  // greater
  0X18,0X04,0X04,0XC4,0X24,0X18,0X00,0X00,0X00,0X0D,0X00,0X00,  // question
  0XF0,0X08,0XE4,0X14,0X14,0XF8,0X03,0X04,0X09,0X0A,0X0A,0X0B,  // at
  0XF0,0X88,0X84,0X84,0X88,0XF0,0X0F,0X00,0X00,0X00,0X00,0X0F,  // A
  0XFC,0X44,0X44,0X44,0XA8,0X10,0X0F,0X08,0X08,0X08,0X04,0X03,  // B
  0XF8,0X04,0X04,0X04,0X04,0X18,0X07,0X08,0X08,0X08,0X08,0X06,  // C
  0XFC,0X04,0X04,0X04,0X08,0XF0,0X0F,0X08,0X08,0X08,0X04,0X03,  // D
  0XFC,0X44,0X44,0X44,0X04,0X04,0X0F,0X08,0X08,0X08,0X08,0X08,  // E
  0XFC,0X44,0X44,0X44,0X04,0X04,0X0F,0X00,0X00,0X00,0X00,0X00,  // F
  0XF8,0X04,0X04,0X84,0X84,0X98,0X07,0X08,0X08,0X08,0X04,0X0F,  // G
  0XFC,0X40,0X40,0X40,0X40,0XFC,0X0F,0X00,0X00,0X00,0X00,0X0F,  // H
  0X00,0X04,0X04,0XFC,0X04,0X04,0X00,0X08,0X08,0X0F,0X08,0X08,  // I
  0X00,0X00,0X00,0X04,0XFC,0X04,0X06,0X08,0X08,0X08,0X07,0X00,  // J
  0XFC,0X40,0XA0,0X10,0X08,0X04,0X0F,0X00,0X00,0X01,0X02,0X0C,  // K
  0XFC,0X00,0X00,0X00,0X00,0X00,0X0F,0X08,0X08,0X08,0X08,0X08,  // L
  0XFC,0X18,0X60,0X60,0X18,0XFC,0X0F,0X00,0X00,0X00,0X00,0X0F,  // M
  0XFC,0X30,0X40,0X80,0X00,0XFC,0X0F,0X00,0X00,0X00,0X03,0X0F,  // N
  0XF8,0X04,0X04,0X04,0X04,0XF8,0X07,0X08,0X08,0X08,0X08,0X07,  // O
  0XFC,0X84,0X84,0X84,0X84,0X78,0X0F,0X00,0X00,0X00,0X00,0X00,  // P
  0XF8,0X04,0X04,0X04,0X04,0XF8,0X07,0X09,0X09,0X0A,0X1C,0X27,  // Q
  0XFC,0X84,0X84,0X84,0X84,0X78,0X0F,0X00,0X00,0X01,0X02,0X0C,  // R
  0X38,0X44,0X44,0X84,0X84,0X18,0X06,0X08,0X08,0X08,0X08,0X07,  // S
  0X04,0X04,0X04,0XFC,0X04,0X04,0X00,0X00,0X00,0X0F,0X00,0X00,  // T
  0XFC,0X00,0X00,0X00,0X00,0XFC,0X07,0X08,0X08,0X08,0X08,0X07,  // U
  0X3C,0XC0,0X00,0X00,0XC0,0X3C,0X00,0X01,0X0E,0X0E,0X01,0X00,  // V
  0X00,0XFC,0X00,0X00,0X00,0XFC,0X00,0X07,0X08,0X07,0X08,0X07,  // W
  0X0C,0X30,0XC0,0XC0,0X30,0X0C,0X0C,0X03,0X00,0X00,0X03,0X0C,  // X
  0X00,0X1C,0X60,0X80,0X60,0X1C,0X00,0X00,0X00,0X0F,0X00,0X00,  // Y
  0X04,0X04,0X84,0X64,0X14,0X0C,0X0C,0X0B,0X08,0X08,0X08,0X08,  // Z
  0X00,0X00,0XFE,0X02,0X02,0X02,0X00,0X00,0X3F,0X20,0X20,0X20,  // bracketleft
  0X06,0X18,0XE0,0X00,0X00,0X00,0X00,0X00,0X00,0X03,0X0C,0X30,  // backslash
  0X00,0X02,0X02,0X02,0XFE,0X00,0X00,0X20,0X20,0X20,0X3F,0X00,  // bracketright
  0X08,0X04,0X02,0X02,0X04,0X08,0X00,0X00,0X00,0X00,0X00,0X00,  // asciicircum
  0X00,0X00,0X00,0X00,0X00,0X00,0X20,0X20,0X20,0X20,0X20,0X20,  // underscore
  0X00,0X00,0X02,0X04,0X08,0X00,0X00,0X00,0X00,0X00,0X00,0X00,  // grave
  0X40,0X20,0X20,0X20,0X20,0XC0,0X06,0X09,0X09,0X09,0X09,0X0F,  // a
  0XFC,0X40,0X20,0X20,0X20,0XC0,0X0F,0X04,0X08,0X08,0X08,0X07,  // b
  0XC0,0X20,0X20,0X20,0X20,0X40,0X07,0X08,0X08,0X08,0X08,0X04,  // c
  0XC0,0X20,0X20,0X20,0X40,0XFC,0X07,0X08,0X08,0X08,0X04,0X0F,  // d
  0XC0,0X20,0X20,0X20,0X20,0XC0,0X07,0X09,0X09,0X09,0X09,0X05,  // e
  0X40,0X40,0XF8,0X44,0X44,0X08,0X00,0X00,0X0F,0X00,0X00,0X00,  // f
  0XC0,0X20,0X20,0X20,0XC0,0X20,0X19,0X26,0X2A,0X2A,0X29,0X10,  // g
  0XFC,0X40,0X20,0X20,0X20,0XC0,0X0F,0X00,0X00,0X00,0X00,0X0F,  // h
  0X00,0X00,0X20,0XEC,0X00,0X00,0X00,0X08,0X08,0X0F,0X08,0X08,  // i
  0X00,0X00,0X00,0X00,0X20,0XEC,0X00,0X18,0X20,0X20,0X20,0X1F,  // j
  0XFC,0X00,0X80,0X40,0X20,0X00,0X0F,0X01,0X01,0X02,0X04,0X08,  // k
  0X00,0X00,0X04,0XFC,0X00,0X00,0X00,0X08,0X08,0X0F,0X08,0X08,  // l
  0X00,0XE0,0X20,0XC0,0X20,0XC0,0X00,0X0F,0X00,0X07,0X00,0X0F,  // m
  0XE0,0X40,0X20,0X20,0X20,0XC0,0X0F,0X00,0X00,0X00,0X00,0X0F,  // n
  0XC0,0X20,0X20,0X20,0X20,0XC0,0X07,0X08,0X08,0X08,0X08,0X07,  // o
  0XE0,0X40,0X20,0X20,0X20,0XC0,0X3F,0X04,0X08,0X08,0X08,0X07,  // p
  0XC0,0X20,0X20,0X20,0X40,0XE0,0X07,0X08,0X08,0X08,0X04,0X3F,  // q
  0XE0,0X40,0X20,0X20,0X20,0XC0,0X0F,0X00,0X00,0X00,0X00,0X00,  // r
  0X40,0XA0,0X20,0X20,0X20,0X40,0X04,0X08,0X09,0X09,0X0A,0X04,  // s
  0X20,0X20,0XFC,0X20,0X20,0X00,0X00,0X00,0X07,0X08,0X08,0X04,  // t
  0XE0,0X00,0X00,0X00,0X00,0XE0,0X07,0X08,0X08,0X08,0X04,0X0F,  // u
  0X00,0XE0,0X00,0X00,0X00,0XE0,0X00,0X00,0X03,0X0C,0X03,0X00,  // v
  0X00,0XE0,0X00,0X80,0X00,0XE0,0X00,0X07,0X08,0X07,0X08,0X07,  // w
  0X60,0X80,0X00,0X00,0X80,0X60,0X0C,0X02,0X01,0X01,0X02,0X0C,  // x
  0XE0,0X00,0X00,0X00,0X00,0XE0,0X13,0X24,0X24,0X24,0X22,0X1F,  // y
  0X20,0X20,0X20,0XA0,0X60,0X20,0X08,0X0C,0X0B,0X08,0X08,0X08,  // z
  0X00,0X00,0X80,0X7C,0X02,0X02,0X00,0X00,0X00,0X1F,0X20,0X20,  // braceleft
  0X00,0X00,0X00,0XFE,0X00,0X00,0X00,0X00,0X00,0X3F,0X00,0X00,  // bar
  0X00,0X02,0X02,0X7C,0X80,0X00,0X00,0X20,0X20,0X1F,0X00,0X00,  // braceright
  0X0C,0X02,0X04,0X08,0X10,0X0C,0X00,0X00,0X00,0X00,0X00,0X00,  // asciitilde
  0XFE,0XFE,0XFE,0XFE,0XFE,0XFE,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F   // del
};

/* SPI helper: write `len` bytes from dout to spi_fd using SPI_IOC_MESSAGE */
void spi_write(int spi_fd, const uint8_t *dout, size_t len)
{
    struct spi_ioc_transfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.tx_buf = (unsigned long)dout;
    xfer.rx_buf = (unsigned long)NULL;
    xfer.len = len;
    xfer.delay_usecs = 0;
    xfer.speed_hz = 5000000;
    xfer.bits_per_word = 8;

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &xfer) < 0) {
        fprintf(stderr, "spi_write: SPI_IOC_MESSAGE failed: %s\n", strerror(errno));
    }
}

/* Simplified SSD1306 routines that toggle the LED (led_line) while writing */
void ssd1306_init(int spi_fd, struct gpiod_line *led_line)
{
    const uint8_t init_cmds[] = {
        0xa8, 0x3f, 0xd3, 0x00, 0x40, 0xa0, 0xc0,
        0xda, 0x02, 0x81, 0x7f, 0xa4, 0xa6, 0xd5,
        0x80, 0x8d, 0x14, 0xaf
    };

    /* If led_line is valid, ensure LED off during SPI control commands */
    if (led_line) gpiod_line_set_value(led_line, 0);
    spi_write(spi_fd, init_cmds, sizeof(init_cmds));
}

void set_col_addr(int spi_fd, struct gpiod_line *led_line, int start, int end)
{
    uint8_t cmd[3];
    if (led_line) gpiod_line_set_value(led_line, 0);
    cmd[0] = 0x21;
    cmd[1] = start & 0x7f;
    cmd[2] = end & 0x7f;
    spi_write(spi_fd, cmd, 3);
}

void set_page_addr(int spi_fd, struct gpiod_line *led_line, int start, int end)
{
    uint8_t cmd[3];
    if (led_line) gpiod_line_set_value(led_line, 0);
    cmd[0] = 0x22;
    cmd[1] = start & 0x03;
    cmd[2] = end & 0x03;
    spi_write(spi_fd, cmd, 3);
}

void set_horizontal_mode(int spi_fd, struct gpiod_line *led_line)
{
    uint8_t cmd[2];
    if (led_line) gpiod_line_set_value(led_line, 0);
    cmd[0] = 0x20;
    cmd[1] = 0x00;
    spi_write(spi_fd, cmd, 2);
}

void clearDisplay(int spi_fd, struct gpiod_line *led_line)
{
    int j, k, i;
    set_col_addr(spi_fd, led_line, 0, 127);
    set_page_addr(spi_fd, led_line, 0, 3);

    for (j = 0; j < 4; j++) {
        for (k = 0; k < 8; k++) {
            uint8_t zeros[16];
            memset(zeros, 0x00, sizeof(zeros));
            if (led_line) gpiod_line_set_value(led_line, 1);
            spi_write(spi_fd, zeros, sizeof(zeros));
        }
    }
}

/* Small-ish oled write that uses your 7x14 font (kept compact) */
void oledprintf(int spi_fd, struct gpiod_line *led_line, const char *s)
{
    int start_col = 0;
    int page2_3 = 0;

    for (size_t j = 0; s[j] != '\0'; j++) {
        if (s[j] == '\n' || s[j] == '\r') {
            page2_3 = 1;
            start_col = 0;
            continue;
        }
        unsigned char mychar[12];
        int tmp = (int)s[j] - ' ';
        if (tmp < 0) tmp = 0;
        int idx = tmp * 12;
        for (int i = 0; i < 12; i++) mychar[i] = font7x14[idx + i];

        int end_col = start_col + 5;
        set_col_addr(spi_fd, led_line, start_col, end_col);
        if (!page2_3)
            set_page_addr(spi_fd, led_line, 0, 1);
        else
            set_page_addr(spi_fd, led_line, 2, 3);

        if (led_line) gpiod_line_set_value(led_line, 1);
        spi_write(spi_fd, mychar, sizeof(mychar));
        start_col += 7;
    }
}

/* Write some ASCII patterns - simplified from your original */
void oledascii(int spi_fd, struct gpiod_line *led_line)
{
    for (int k = 0; k < 3; k++) {
        int start_col = 0;
        for (int j = 0; j < 32; j++) {
            unsigned char mychar[12];
            int tmp = j + (k << 5);
            int idx = tmp * 12;
            for (int i = 0; i < 12; i++) mychar[i] = font7x14[idx + i];
            int end_col = start_col + 5;
            set_col_addr(spi_fd, led_line, start_col, end_col);
            if (j < 16)
                set_page_addr(spi_fd, led_line, 0, 1);
            else
                set_page_addr(spi_fd, led_line, 2, 3);
            if (led_line) gpiod_line_set_value(led_line, 1);
            spi_write(spi_fd, mychar, sizeof(mychar));
            start_col += 7;
            if (start_col >= 112) start_col = 0;
        }
        sleep(2);
        clearDisplay(spi_fd, led_line);
    }
}

/* LED blinking test using gpiod_line */
int led_test(struct gpiod_line *led_line)
{
    if (!led_line) return -1;
    puts("LED test: blink 5 times.");
    for (int i = 0; i < 5; ++i) {
        gpiod_line_set_value(led_line, 1);
        sleep(1);
        gpiod_line_set_value(led_line, 0);
        sleep(1);
    }
    return 0;
}

/* Button press test: expects transition 0->1->0 to count as a press */
int button_test(struct gpiod_line *led_line, struct gpiod_line *sw_line)
{
    if (!led_line || !sw_line) return -1;
    puts("Button test: push button 10 times.");
    int old_state = 0, current_state = 0;
    for (int count = 0; count < 10; ) {
        int v = gpiod_line_get_value(sw_line);
        if (v < 0) {
            fprintf(stderr, "button_test: read failed: %s\n", strerror(errno));
            return -1;
        }
        current_state = v;
        if (old_state == 0 && current_state == 1) {
            gpiod_line_set_value(led_line, 1);
            old_state = current_state;
        } else if (old_state == 1 && current_state == 0) {
            gpiod_line_set_value(led_line, 0);
            old_state = current_state;
            count++;
            printf("Detected press %d/10\n", count);
        }
        usleep(100000);
    }
    return 0;
}

int ssd1306_test(struct gpiod_line *led_line)
{
    const char *spi_dev = "/dev/spidev0.0"; /* adjust if needed */
    int spi_fd = open(spi_dev, O_RDWR);
    if (spi_fd < 0) {
        fprintf(stderr, "ssd1306_test: open(%s) failed: %s\n", spi_dev, strerror(errno));
        return -1;
    }

    uint8_t mode = 0;
    uint8_t bits = 8;
    uint32_t speed = 5000000;

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
    {
        fprintf(stderr, "ssd1306_test: failed to configure SPI: %s\n", strerror(errno));
        close(spi_fd);
        return -1;
    }

    if (led_line) gpiod_line_set_value(led_line, 1);
    ssd1306_init(spi_fd, led_line);
    set_horizontal_mode(spi_fd, led_line);
    set_col_addr(spi_fd, led_line, 0, 127);
    set_page_addr(spi_fd, led_line, 0, 3);

    /* Some pattern */
    for (int page = 0; page < 4; page++) {
        for (int i = 0; i < 128; i += 8) {
            uint8_t pattern[8] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
            if (led_line) gpiod_line_set_value(led_line, 1);
            spi_write(spi_fd, pattern, sizeof(pattern));
        }
    }

    sleep(2);
    clearDisplay(spi_fd, led_line);
    oledascii(spi_fd, led_line);
    sleep(2);
    clearDisplay(spi_fd, led_line);
    oledprintf(spi_fd, led_line, "This is a test !\nIt works !");
    close(spi_fd);
    return 0;
}

/* Simple menu routine */
char test_menu(void)
{
    char ch;
    puts("\n--- select a test ---");
    puts("1. uart test");
    puts("2. led test");
    puts("3. button test");
    puts("4. pwm led test");
    puts("5. i2c lcd test");
    puts("6. tongsong");
    puts("7. servo");
    puts("8. spi oled test");
    puts("q. quit");
    ch = getchar();
    /* consume newline */
    while (getchar() != '\n') ;
    if (ch == 'q') puts("Goodbye!");
    return ch;
}

int main(void)
{
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *led = NULL;
    struct gpiod_line *sw = NULL;

    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        perror("gpiod_chip_open_by_name");
        return EXIT_FAILURE;
    }

    led = gpiod_chip_get_line(chip, LED_OFFSET);
    if (!led) { perror("gpiod_chip_get_line (LED)"); goto cleanup; }
    if (gpiod_line_request_output(led, "gpio_test", 0) < 0) {
        perror("gpiod_line_request_output");
        goto cleanup;
    }

    sw = gpiod_chip_get_line(chip, SW_OFFSET);
    if (!sw) { perror("gpiod_chip_get_line (SW)"); goto cleanup; }
    if (gpiod_line_request_input(sw, "gpio_test") < 0) {
        perror("gpiod_line_request_input");
        goto cleanup;
    }

    char choice = '\0';
    while (choice != 'q') {
        choice = test_menu();
        switch (choice) {
            case '1': uart_test(); break;
            case '2': led_test(led); break;
            case '3': button_test(led, sw); break;
            case '4': pwm_led_test(); break;
            case '5': i2c_lcd_test(); break;
            case '6': tongsong(); break;
            case '7': servo(); break;
            case '8': ssd1306_test(led); break;
            case 'q': break;
            default: puts("Unknown selection"); break;
        }
    }

cleanup:
    if (led) gpiod_line_release(led);
    if (sw) gpiod_line_release(sw);
    if (chip) gpiod_chip_close(chip);
    return 0;
}

