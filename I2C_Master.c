/*
 * File:   I2C_Master.c
 * Author: ASUS
 *
 * Created on January 6, 2022, 10:13 AM
 */

// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "I2C_Master.h"
#define _XTAL_FREQ  20000000
#define I2C_speed       100000  //100 kbps

void I2C_init()
{
    /*Set pin RC3, RC4 as input as given in datasheet*/
    TRISC3=1;
    TRISC4=1;
    
    /*Enabled for high-speed mode*/
    SSPSTATbits.SMP=0;
    /*Disable SMBus specific inputs*/
    SSPSTATbits.CKE=0;
    /*Note: or SSPSTAT=0x00;*/
    
    /*Synchronous Serial Port Enable (SCL & SDA)*/
    SSPCONbits.SSPEN=1;
    /*No use*/
    SSPCONbits.CKP=0;
    /*SSPM3:SSPM0: Synchronous Serial Port Mode Select bits*/
    /*1000: I2C Master mode, clock = FOSC/(4 * (SSPADD + 1))*/
    SSPCONbits.SSPM=0b1000;
    /*Note: or SSPCON=0x28;*/
    
    SSPCON2=0x00;
    /*clock = FOSC/(4 * (SSPADD + 1))*/
    SSPADD=(_XTAL_FREQ/(4*I2C_speed))-1;
}

void I2C_wait()
{
    /*While R/W bit is still not ==0 (while ==0, transmit is not in progress)*/
    /*Or while SEN, RSEN, PEN, RCEN or ACKEN still not ==0 (while ==0, transmit is not in progress)*/
    while((SSPSTATbits.R_W!=0) || ((SSPCON2 & 0x1F)!=0));
    __delay_ms(1);
}

void I2C_start()
{    
    /*Enable Start bit*/
    SSPCON2bits.SEN=1;  
    /*Wait for start bit send complete*/
    I2C_wait();
}

void I2C_restart()
{   
    /*Enable ReStart bit*/
    SSPCON2bits.RSEN=1;
    /*Wait for restart bit send complete*/
    I2C_wait();
}

void I2C_stop()
{    
    /*Enable Stop bit*/
    SSPCON2bits.PEN=1;
    /*Wait for stop bit send complete*/
    I2C_wait();    
}

void I2C_write(unsigned char a)
{    
    /*Write a byte to buffer register*/
    SSPBUF=a;
    /*Wait for write complete*/
    I2C_wait();
}

void I2C_Send_ACK(void)
{
    ACKDT = 0;      // 0 means ACK
    ACKEN = 1;      // Send ACKDT value
    /*Wait for write complete*/
    I2C_wait();
}

void I2C_Send_NACK(void)
{
    ACKDT = 1;      // 1 means NACK
    ACKEN = 1;      // Send ACKDT value
    /*Wait for write complete*/
    I2C_wait();
}
char I2C_read(unsigned char ACK)
{
    unsigned char read=0;
    
    /*Receive Enable*/
    SSPCON2bits.RCEN=1;
    I2C_wait();
    
    /*Read data from SSPBUF*/
    read=SSPBUF;
    I2C_wait();
    
    if(ACK == 1)
    {
        /*Acknowledge Data bit*/
        I2C_Send_ACK();
    }
    else
    {
        /*Acknowledge Data bit*/
        I2C_Send_NACK();
    }
    
    return read;
}

//void main(void) 
//{
//    /*turn on all the internal pull-ups*/
//    /*Enable input port B*/
//    OPTION_REGbits.nRBPU=0;
//    
//    TRISB=0xFF;
//    TRISD=0;    PORTD=0;
//    
//    I2C_init();
//    
//    while(1)
//    {
//        I2C_start();
//        /*Send 7 bit address + Write*/
//        I2C_write(0x70);
//        /*Write data from Port B to SSPBUF register*/
//        I2C_write(PORTB);
//        I2C_stop();
//        /*Wait for write complete*/
//        __delay_ms(1);
//        
//        I2C_start();
//        /*Send 7 bit address + Read*/
//        I2C_write(0x71);
//        /*Read data from SSPBUF register to port D + Acknowledge*/
//        PORTD=I2C_read();
//        I2C_stop();
//        __delay_ms(1);
//    }
//    return;
//}
