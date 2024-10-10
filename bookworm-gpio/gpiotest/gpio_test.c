#include <stdio.h>
#include <stdlib.h>
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

#define TRUE    (1==1)
#define FALSE   (!TRUE)

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

//----- LED and BUTTON ----------------------------

void initpin( int pin, char *mode )
{
    FILE *fp;
    bool isdir;
    char path[100] = "";
    char direction[100] = "";
    sprintf( path, "/sys/class/gpio/gpio%d", pin );
    sprintf( direction, "/sys/class/gpio/gpio%d/direction", pin );
    struct stat st = {0};
    if ( !stat(path, &st) ) {
        isdir = S_ISDIR( st.st_mode );
    }
    if ( !isdir ) {
        fp = fopen( "/sys/class/gpio/export", "w" );
        fprintf( fp, "%d", pin );
        fclose( fp );
        fp = fopen( direction, "w" );
        fprintf( fp, "%s", mode );
        fclose( fp );
    }
}

void setpin( int pin, int value )
{
    FILE *fp;
    char pinvalue[100] = "";
    sprintf( pinvalue, "/sys/class/gpio/gpio%d/value", pin );
    fp = fopen( pinvalue, "w" );
    fprintf( fp, "%d", value );
    fclose( fp );
}

int getpin( int pin )
{
    FILE *fp;
    int value;
    char pinvalue[100] = "";
    sprintf( pinvalue, "/sys/class/gpio/gpio%d/value", pin );
    fp = fopen( pinvalue, "r" );
    fscanf( fp, "%d", &value );
    fclose( fp );
    return ( value );
}

void closepin( int pin )
{
    FILE *fp;
    fp = fopen( "/sys/class/gpio/unexport", "w" );
    fprintf( fp, "%d", pin );
    fclose( fp );
}

int led_test(void) {
    int i;

    printf("led test, blink led 5 times.\n");
    initpin( 32, "out" );

    for ( i = 0; i < 5; i++ ) {
        setpin( 32, 1 );
        sleep( 1 );
        setpin( 32, 0 );
        sleep( 1 );
    }

    closepin( 32 );
    return 0;
}

int button_test( void )
{
    int i;
    int old_state;
    int current_state;
    printf( "Push button 10 times.\n" );
    initpin( 32, "out" );
    initpin( 33, "in" );

    old_state = 0;
    current_state = 0;
    for ( i = 0; i < 10; ) {
        current_state = getpin( 33 );
        if ( old_state == 0 && current_state == 1 ) {
            setpin( 32, 1 );
            old_state = current_state;
        } else if ( old_state == 1 && current_state == 0 ) {
            setpin( 32, 0 );
            old_state = current_state;
            i++;
        }
        usleep( 100000 );
    }

    closepin( 32 );
    closepin( 33 );
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

void spi_write( int spi, unsigned char *dout, int len )
{  
    u_int8_t buf[len];
    int status;
    int i;
    struct spi_ioc_transfer xfer;

    memset(&xfer, 0, sizeof(xfer));

    for ( i = 0; i < len; i++ )
        buf[i] = dout[i];
    xfer.cs_change = 0;
    xfer.delay_usecs = 0;          //delay in us
    xfer.speed_hz = 5000000;       //speed
    xfer.bits_per_word = 8;        // bites per word 8
    xfer.tx_buf = (unsigned long )buf;
    xfer.rx_buf = (unsigned long )NULL;
    xfer.len = sizeof buf;
    status = ioctl( spi, SPI_IOC_MESSAGE(1), &xfer );
    if ( status < 0 ) {
        printf("SPI_IOC_MESSAGE");
    }
}

void ssd1306_init( int spi )
{
    unsigned char myData[] = {0xa8, 0x3f, 0xd3, 0x0, 0x40, 0xa0, 0xc0, 0xda, 0x2, 0x81, 0x7f, 0xa4, 0xa6, 0xd5, 0x80, 0x8d, 0x14, 0xaf};

    setpin( 32, 0);
    spi_write( spi, myData, 18 );
}

void set_col_addr( int spi, int col_start, int col_end )
{
    unsigned char myData[3];

    setpin( 32, 0) ;

    myData[0] = 0x21;
    myData[1] = col_start & 0x7f;
    myData[2] = col_end & 0x7f;
    spi_write( spi, myData, 3 );
}

void set_page_addr( int spi, int page_start, int page_end )
{
    unsigned char myData[3];

    setpin( 32, 0) ;

    myData[0] = 0x22;
    myData[1] = page_start & 0x3;
    myData[2] = page_end & 0x3;
    spi_write( spi, myData, 3 );
}

void set_horizontal_mode( int spi )
{
    unsigned char myData[2];

    setpin( 32, 0) ;

    myData[0] = 0x20;
    myData[1] = 0x00;
    spi_write( spi, myData, 2 );
}

void set_start_page( int spi, int page )
{
    unsigned char myData[1];

    setpin( 32, 0) ;

    myData[0] = 0xB0 | (page & 0x3);
    spi_write( spi, myData, 1 );
}

void set_start_col( int spi, int col )
{
    unsigned char myData[2];

    setpin( 32, 0) ;

    myData[0] = 0xf & col;
    myData[1] = (0xf & (col >> 4)) | 0x10;
    spi_write( spi, myData, 2 );
}

void clearDisplay( int spi )
{
    int i;
    int j;
    int k;

    set_col_addr( spi, 0, 127 );
    set_page_addr( spi, 0, 3 );
    setpin( 32, 1 );
    for (j=0; j<4; j++) {
        for (k=0; k<8; k++) {
            unsigned char myData[16];
            for (i=0; i < 16; i++) {
                myData[i] = 0;
            }
            spi_write( spi, myData, 16 );
        }
    }
}

void oledprintf( int spi, unsigned char *ch )
{
  int i;
  int j;
  int start_col;
  int end_col;
  int tmp;
  int tmp1;
  int page2_3;

  start_col = 0;
  page2_3 = 0;

  for (j=0; ch[j] != '\0'; j++) {
      if (ch[j] != '\n' && ch[j] != '\r') {
          unsigned char mychar[12];
          tmp = ch[j] - ' ';
          tmp1 = (tmp << 3) + (tmp << 2);  // tmp x 12
          for (i=0; i< 12; i++) {
              mychar[i] = font7x14[tmp1];
              tmp1++;
          }
          end_col = start_col + 5;
          set_col_addr( spi, start_col, end_col );
          if (page2_3 == 0) {
              set_page_addr( spi, 0, 1 );
          } else {
              set_page_addr( spi, 2, 3 );
          }
          setpin( 32, 1 );
          spi_write( spi, mychar, 12 );
          start_col += 7;
      } else {
          page2_3 = 1;
          start_col = 0;
      }
  }
}

void oledascii( int spi )
{
    int i;
    int j;
    int k;
    int start_col;
    int end_col;
    int tmp;
    int tmp1;
  
    for (k=0; k < 3; k++) {
        start_col = 0;
        for (j=0; j < 32; j++) {             // 16 characters per two pages
            unsigned char mychar[12];
            tmp = j + (k << 5);
            tmp1 = (tmp << 3) + (tmp << 2);  // tmp x 12
            for (i=0; i< 12; i++) {
                mychar[i] = font7x14[tmp1];
                tmp1++;
            }
            end_col = start_col + 5;
            set_col_addr( spi, start_col, end_col );
            if (j < 16) {
                set_page_addr( spi, 0, 1 );
            } else {
                set_page_addr( spi, 2, 3 );
            }
            setpin( 32, 1 );
            spi_write( spi, mychar, 12 );
            start_col += 7;
            if ( start_col >= 112 ) {
                start_col = 0;
            }
        }
        sleep( 2 );
        clearDisplay( spi );
    }
}

int ssd1306_test( void )
{
    int spi;
    int i;
    int j;
    u_int8_t mode = 0;
    u_int32_t speed = 5000000;
    u_int8_t bits = 8;

    initpin( 32, "out" ); 
    setpin( 32, 1 );
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

    ssd1306_init( spi );
  
    //set_page_mode();
    set_horizontal_mode( spi );
    set_col_addr( spi, 0, 127 );
    set_page_addr( spi, 0, 3 );
  
    setpin( 32, 1 );
    for (j=0; j < 4; j++) {
        for (i=0; i < 128; i=i+8) {
	    unsigned char myData[] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
            spi_write( spi, myData, 8 );
        }
    }
  
    sleep( 2 );
    clearDisplay( spi );
    oledascii( spi );
    sleep( 2 );
    clearDisplay( spi );
    oledprintf( spi, "This is a test !\nIt works !\n" );

    closepin( 32 );
    close( spi );
    return 0;
}


//------ GPIO_TEST ----------------------------------

char test_menu( void )
{
    char ch;

    printf( "--- select a test ---\n" );
    printf( "1. uart test\n2. led test\n3. button test\n4. pwm led test\n5. i2c lcd test\n6. tongsong\n7. servo\n8. spi oled test\nq. quit\n");
    ch = getchar();
    getchar();
    if ( ch == 'q' )
        printf( "Goodbye !\n" );
    return ch;
}

int main(void) {
    char ch;
    char test_item;

    test_item = '\0';
    while( test_item != 'q' ) {
        test_item = test_menu();
        if ( test_item == '1' ) {
            uart_test();
        } else if ( test_item == '2' ) {
            led_test();
        } else if ( test_item == '3' ) {
            button_test();
        } else if ( test_item == '4' ) {
            pwm_led_test();
        } else if ( test_item == '5' ) {
            i2c_lcd_test();
            printf( "press x to exit i2c lcd test\n" );
            ch = getchar();
            getchar();
        } else if ( test_item == '6' ) {
            tongsong();
        } else if ( test_item == '7' ) {
            servo();
        } else if ( test_item == '8' ) {
            ssd1306_test();
        }
    }

    return 0;
}
