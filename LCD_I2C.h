
#define _XTAL_FREQ  20000000
#define LCD_Clear           0x01  //Clear display
#define LCD_ReturnHome      0x02  //Return cursor and LCD to home position
#define LCD_MoveCursor      0x06  //Move cursor to the end of the last character
#define LCD_Off             0x08  //Turn off display and cursor
#define LCD_DisplayOn       0x0C  //Turn on display and turn off cursor
#define LCD_CursorOn        0x0E  //Turn on display and cursor
#define LCD_CursorLine1     0x80  //Move cursor to the first position of line 1
#define LCD_CursorLine2     0xC0  //Move cursor to the first position of line 2
#define LCD_CursorLine3     0x90  //Move cursor to the first position of line 3
#define LCD_CursorLine4     0xD0  //Move cursor to the first position of line 4
#define LCD_MDRight         0x1C  //Move display to the right
#define LCD_MDLeft          0x18  //Move display to the left
#define LCD_BACKLIGHT       0x08  //Turn on background led
#define LCD_NOBACKLIGHT     0x00  //Turn off background led
/*Funtion set*/
#define LCD_TYPE            2     //| 0 -> 5x7 (1 line) | 1 -> 5x10 (1 line) | 2 -> 2 lines |
/*LCD 4bit or LCD 8bit?*/
#define LCD_4bit            0x28
#define LCD_8bit            0x38

#define clock       100000 //100kbps

/*---------------[ I2C Routines ]-------------------*/
void I2C_init();
void i2c_wait();
void i2c_start();
void i2c_stop();
void i2c_write(unsigned char a);

/******************************************************************************/

/*---------------[ LCD and I2C complex ]----------------*/
void i2c_LCD_write(unsigned char data);

/******************************************************************************/

/*---------------[ LCD Routines ]----------------*/
void LCD_command(unsigned char command);
void LCD_putc(unsigned char data);
void LCD_puts(char *str);
void LCD_gotoxy(unsigned char x, unsigned char y);
void LCD_tym(void);
void LCD_init(unsigned char I2C_Addr);
void TurnOn_Led(void);
void TurnOff_Led(void);
void LCD_int(unsigned int value);
void LCD_double(double value, int n);
