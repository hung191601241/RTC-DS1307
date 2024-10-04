/*
 * File:   LCD_I2C_Master.c
 * Author: ASUS
 *
 * Created on January 21, 2022, 10:36 AM
 */

#include <xc.h>
//#include <math.h>
#include "LCD_I2C.h"
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

unsigned char BackLight_State=0, i2c_addr=0;
//unsigned char TymBlack[8]={0x00, 0x0A, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00};
//unsigned char TymWhite[8]={0x00, 0x0A, 0x15, 0x11, 0x11, 0x0A, 0x04, 0x00};
//unsigned char yen[8]={0x11, 0x0A, 0x1F, 0x04, 0x1F, 0x04, 0x04, 0x00};

/*----------------------------[ I2C Routines ]--------------------------------*/
void I2C_Init(void)
{
    TRISC3=1;
    TRISC4=1;
    
    SSPSTATbits.SMP=1;
    SSPSTATbits.CKE=0;
    
    SSPCONbits.SSPEN=1;
    SSPCONbits.SSPM=0b1000;
    
    SSPADD=(_XTAL_FREQ/(4*clock))-1;
    
    SSPCON2=0x00;
}

void i2c_wait(void)
{
    while(SSPSTATbits.R_W!=0 || (SSPCON2 & 0x1F)!=0);    
}

void i2c_start(void)
{
    SSPCON2bits.SEN=1;
    i2c_wait();
}

void i2c_stop(void)
{
    SSPCON2bits.PEN=1;
    i2c_wait();
}

void i2c_write(unsigned char a)
{
    SSPBUF=a;
    i2c_wait();
}

/******************************************************************************/

/*---------------[ LCD and I2C complex ]----------------*/
void i2c_LCD_write(unsigned char data)
{
    i2c_start();
    /*write Address of module I2C*/
    i2c_write(i2c_addr);
    /*write data*/
    i2c_write(data | BackLight_State);
    i2c_stop();
}

/******************************************************************************/

/*-----------------------------[ LCD Routines ]-------------------------------*/
void LCD_command(unsigned char command)
{
    unsigned char n=0x00;
    i2c_LCD_write(n);
    
    n=(n & 0x0F)|(command & 0xF0);
    i2c_LCD_write(n);
    i2c_LCD_write(n |= 0x04);
    __delay_ms(1);
    i2c_LCD_write(n &= 0xFB);
    __delay_ms(1);
    
    n=(n & 0x0F)|(command << 4);
    i2c_LCD_write(n);
    i2c_LCD_write(n |= 0x04);
    __delay_ms(1);
    i2c_LCD_write(n &= 0xFB);
    __delay_ms(1);
}

void LCD_putc(unsigned char data)
{
    unsigned char n=0x01;
    i2c_LCD_write(n);
    
    n=(n & 0x0F)|(data & 0xF0);
    i2c_LCD_write(n);
    i2c_LCD_write(n |= 0x04);
    __delay_ms(1);
    i2c_LCD_write(n &= 0xFB);
    __delay_ms(1);
    
    n=(n & 0x0F)|(data << 4);
    i2c_LCD_write(n);
    i2c_LCD_write(n |= 0x04);
    __delay_ms(1);
    i2c_LCD_write(n &= 0xFB);
    __delay_ms(1);
}

void LCD_puts(char *str)
{
    for(int i=0; str[i]!='\0'; i++)
    {
        LCD_putc(str[i]);
    }
}

/*Send the integer to LCD*/
/*In ASCII, 48=0*/
void LCD_int(unsigned int value)
{
    int dem=0, a1=0;
    /*If it is a negative number*/
    if(value<0)
    {
        LCD_putc('-');
        value*=-1;
    }
    if(value>=0 && value<10)
    {
        LCD_putc(0+48);
        LCD_putc((value+48));
    }
    if(value>=10 && value <100)
        dem=2;
    if(value>=100 && value<1000)
        dem=3;
    if(value>=1000 && value<10000)
        dem=4;
    if(value>=10000 && value<32767)
        dem=5;
    
    if(dem>=2)
    {
        do
        {
            int a2=1;
            for(int i=0; i<(dem-1); i++)
            {
                a2*=10;
            }
            a1=value/a2;
            LCD_putc((a1+48));
            value %= a2;
            dem--;
        }while(dem!=0);
    }
}

/*Send the decimal to LCD*/
/*void lcd_double(vale, number of decimal places)*/
//void LCD_double(double value, int n)
//{
//    int a1, a2, i;
//    double a3;
//    /*If it is a negative number*/
//    if(value<0)
//    {
//        LCD_putc('-');
//        value*=-1;
//    }
//    /*floor function: rounded down*/
//    a1=floor(value);
//    LCD_int(a1);
//    LCD_putc('.');
//    /*Decimals*/
//    a3=value-a1;
//    for(i=0; i<n; i++)
//    {
//        a3*=10;
//        a2=floor(a3);
//        LCD_int(a2);
//        a3=a3-a2;        
//    }
//}

/*Code trai tym*/
//void LCD_tym(void)
//{
//    int i;
//    /*Go to the CGRAM address*/
//    LCD_command(0x40);
//    for(i=0; i<8; i++)
//    {
//        LCD_putc(TymWhite[i]);
//    }
//}

void LCD_gotoxy(unsigned char x, unsigned char y)
{
    switch(x)
    {
        case 1:
            LCD_command(LCD_CursorLine1 + (y-1));
            break;
        case 2:
            LCD_command(LCD_CursorLine2 + (y-1));
            break;
        case 3:
            LCD_command(LCD_CursorLine3 + (y-1));
            break;
        //Case 4
        default:
            LCD_command(LCD_CursorLine4 + (y-1));
    }
}

void LCD_init(unsigned char I2C_Addr)
{
    LCD_command(LCD_Clear);
    i2c_addr=I2C_Addr;
    i2c_LCD_write(0);   
    
    LCD_command(LCD_ReturnHome);
    /*Function set: 4bit Protocol and Display 2 lines*/
    LCD_command(0x20 | (LCD_TYPE << 2));
    LCD_command(LCD_DisplayOn);
    /*No use Entry mode set*/
}

void TurnOn_Led(void) 
{
  BackLight_State = LCD_BACKLIGHT;
  i2c_LCD_write(0);
}
//
//void TurnOff_Led(void) 
//{
//  BackLight_State = LCD_NOBACKLIGHT;
//  i2c_LCD_write(0);
//}

//void main(void) 
//{
//    I2C_init();
//    LCD_init(0x40);
//    TurnOn_Led();
//    LCD_tym();
//    LCD_gotoxy(1,1);
//    __delay_ms(500);
//    LCD_puts("LOVE ");
//    __delay_ms(200);
//    LCD_puts("YOU");
//    __delay_ms(500);
//    LCD_gotoxy(2,1);
//    LCD_puts("(=^.^=) ");
//    __delay_ms(500);
//    /*Ki tu trai tym*/
//    LCD_putc(0);
//    __delay_ms(200);
//    LCD_putc(0);
//    __delay_ms(200);
//    LCD_putc(0);
//    __delay_ms(200);
//    LCD_putc(0);
//    __delay_ms(200);
//    LCD_putc(0);
//    __delay_ms(1000);
//    
//    LCD_init(0x4E);
//    LCD_gotoxy(1,1);
//    LCD_puts("Chao Trang");
//    __delay_ms(300);
//    LCD_gotoxy(2,1);
//    LCD_puts("Ngay hom nay cua");
//    LCD_gotoxy(3,1);
//    LCD_puts("ban the nao?");
//    LCD_gotoxy(4,1);
//    LCD_puts("Happy New Year!!");
//    while(1)
//    {
//        
//    }
//    return;
//}
