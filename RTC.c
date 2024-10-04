/*
 * File:   RTC.c
 * Author: ASUS
 *
 * Created on March 5, 2022, 5:01 PM
 */

// PIC16F877A Configuration Bit Settings
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>
#include <stdio.h>
#include "I2C_Master.h"
#include "LCD_I2C.h"
#define _XTAL_FREQ  20000000

#define buttonMenu      RB0
#define buttonMode      RB1
#define buttonAlarm     RB2
#define buttonUp        RB3
#define buttonDown      RB4

#define Ds1307ReadMode   0xD1  // DS1307 ID in read mode
#define Ds1307WriteMode  0xD0  // DS1307 ID in write mode

#define Ds1307SecondRegAddress   0x00   // Address to access Ds1307 SEC register
#define Ds1307ControlRegAddress  0x07   // Address to access Ds1307 CONTROL register

/*-----------------------------------------------------------[  Declare The Global Variables  ]----------------------------------------------------------*/
unsigned int MODE = 24;//MODE=12 or MODE=24
unsigned char Mode=1;   //Mode is bit6 in Hour register, Mode=1: 12h, Mode=0: 24h
unsigned char AP=1;     //AP is bit5 in Hour register, AP=1: PM, AP=0: AM
typedef struct
{
    unsigned char sec;
    unsigned char min;
    unsigned char hour;
    unsigned char week;
    unsigned char date;
    unsigned char month;
    unsigned char year;  
}RTC;

RTC RTCbits;
char str1[16], str2[16];
unsigned int demmenu = 0;
/*The SetTime Variables*/
signed int thu=1, ngay=1, thang=1, namdv=0, namch=0, gio=0, phut=0, giay=0;
/*The Alarm Variables*/
int giobt=0, phutbt=0, giaybt=0;

/*------------------------------------------------------------------[  Declare The functions  ]-----------------------------------------------------------*/
void RTC_init(void);
void RTC_Set_DateTime(RTC *rtc);
void RTC_Receive_DateTime(RTC *rtc);

unsigned char DecToBCD(unsigned char a);
unsigned char BCDtoDec(unsigned char a);

void SetTime(unsigned char gi, unsigned char ph, unsigned char gia);
void SetDate(unsigned char th, unsigned char ng, unsigned char tha, unsigned char nm);
void Display_Time();

void SetThu();
void SetNgay();
void SetThang();
void SetNamchuc();
void SetNamdv();
void SetGio();
void SetPhut();
void SetGiay();
void Select_Menu(void);

void SetMode(void);

void Display_Alarm(int Vitri);
void Setup_Alarm();

/*---------------------------------------------------------------------[  MAIN FUNCTION  ]---------------------------------------------------------------*/        
void main(void) 
{
    /*Initiate Status of Alarm is Off*/
    unsigned int DangBaoThuc=0;
    
    /*Set Port B is Input*/
    TRISB = 0xFF;
    /*Pull-up Port B*/
    OPTION_REGbits.nRBPU=0;
    /*Set Pin RD0 is Output*/
    TRISD0 = 0;
    /*Initiate Pin RD0=0*/
    RD0 = 0;   
    
    RTC_init();
    LCD_init(0x4E);
    TurnOn_Led();
    
    /*Initiate Time and Date*/
//    SetTime(15, 59,45);
//    SetDate(4, 9, 3, 22);
        
    while(1)
    {
        Select_Menu();
        SetMode(); 
        
        /*Neu Thoi gian thuc = Thoi gian cai dat Bao thuc && Dong ho phai o che do Hien thi -> Bat coi*/
        if((giobt==BCDtoDec(RTCbits.hour/*Mode 24h*/) || giobt==BCDtoDec(RTCbits.hour & ~(3<<5))+12  ||giobt==BCDtoDec(RTCbits.hour & ~(3<<5))/*Mode 12h*/)&& phutbt==BCDtoDec(RTCbits.min) && giaybt==BCDtoDec(RTCbits.sec) && demmenu==0)
        {
            /*Bat coi Bao thuc*/
            RD0=1;
            /*Bat trang thai Bao thuc*/
            DangBaoThuc=1;
        }
        
        /*Khi nhan nut Bao thuc*/       
        if(buttonAlarm==0)     
        {                   
            while(buttonAlarm==0);
            /*Neu Bao thuc dang keu -> Tat coi Bao thuc*/
            if(DangBaoThuc==1)
            {
                /*Tat coi Bao thuc*/
                RD0=0;  
                /*Tat trang thai Bao thuc*/
                DangBaoThuc=0; 
            }
            
            /*Neu Bao thuc dang khong hoat dong -> Cai Bao Thuc*/
            else
               Setup_Alarm();  
        }      
    }      
}

/*--------------------------------------------------------------------[  RTC Initiation  ]---------------------------------------------------------------*/
void RTC_init(void)
{
    I2C_init();                             
    I2C_start();                            
    /*Send the DS1307 address and select write operation*/
    I2C_write(Ds1307WriteMode);
    /*Select the DS1307 ControlRegister to configure DS1307*/
    I2C_write(Ds1307ControlRegAddress);
    /*Write 0x00 to Control register to disable SQW-Out*/
    I2C_write(0x00);                        
    /*Stop I2C communication after initializing DS1307*/
    I2C_stop();
}
/*---------------------------------------------------------[  Access to Address form PIC to DS1307  ]----------------------------------------------------*/
void RTC_Set_DateTime(RTC *rtc)
{
    I2C_start();
    /*Send the DS1307 address and select write operation*/
    I2C_write(Ds1307WriteMode);
    /*Request address of second register at Address: 0x00*/
    I2C_write(Ds1307SecondRegAddress);
    
    /*Write second from RAM address 0x00*/
    I2C_write(rtc->sec);  
    /*Write minute from RAM address 0x01*/
    I2C_write(rtc->min);
    /*Write hour from RAM address 0x02*/
    I2C_write(rtc->hour);
    /*Write week from RAM address 0x03*/
    I2C_write(rtc->week);
    /*Write date from RAM address 0x04*/
    I2C_write(rtc->date); 
    /*Write month from RAM address 0x05*/
    I2C_write(rtc->month);
    /*Write year from RAM address 0x06*/
    I2C_write(rtc->year);
    
    /*Stop I2C communication after write date time DS1307*/
    I2C_stop();
}
/*---------------------------------------------------------------[  Read Data form PIC to DS1307  ]------------------------------------------------------*/
void RTC_Read_DateTime(RTC *rtc)
{
    I2C_start();       
    /*Send the DS1307 address and select write operation*/
    I2C_write(Ds1307WriteMode); 
    /*Request address of second register at Address: 0x00*/
    I2C_write(Ds1307SecondRegAddress);
    /*Stop I2C communication after initializing DS1307*/
    I2C_stop();
    
    /*Start I2C communication again*/
    I2C_start();    
    /*Send the DS1307 address and select read operation*/
    I2C_write(Ds1307ReadMode);            
    
    /*Read second and return ACK*/
    rtc->sec = I2C_read(1);
    /*Read minute and return ACK*/
    rtc->min = I2C_read(1); 
    /*Read hour and return ACK*/
    rtc->hour = I2C_read(1); 
    /*Read week and return ACK*/
    rtc->week = I2C_read(1); 
    /*Read date and return ACK*/
    rtc->date = I2C_read(1);
    /*Read month and return ACK*/
    rtc->month = I2C_read(1); 
    /*Read year and return NACK*/
    rtc->year = I2C_read(0);
    
    /*Stop I2C communication after read date time DS1307*/
    I2C_stop();
}

/*-------------------------------------------------------------[  Convert from Decimal to BCD  ]---------------------------------------------------------*/
unsigned char DecToBCD(unsigned char a)
{
    unsigned char chuc, dv;
    chuc = (a/10)<<4;
    dv = a%10;
    return (chuc+dv);
}
/*----------------------------------------------------------------[  Convert from BCD to Decimal  ]------------------------------------------------------*/
unsigned char BCDtoDec(unsigned char a)
{
    unsigned char chuc, dv;
    chuc = (a>>4)*10;
    dv = a&0x0F;
    return (chuc+dv);
}

/*----------------------------------------------------------------[  Set Date from DS1307 to PIC  ]------------------------------------------------------*/
void SetTime(unsigned char gi, unsigned char ph, unsigned char gia)
{
    if(MODE == 12)
        if(gi <= 12)    {RTCbits.hour = DecToBCD(gi)|(Mode<<6);}
        else            {RTCbits.hour = DecToBCD(gi-12)|(Mode<<6)|(AP<<5);}
    else if(MODE == 24)
        RTCbits.hour = DecToBCD(gi);

    RTCbits.min = DecToBCD(ph);
    RTCbits.sec = DecToBCD(gia);
    
    RTC_Set_DateTime(&RTCbits);
}

/*--------------------------------------------------------------[  Set Date from PIC to DS1307  ]--------------------------------------------------------*/
void SetDate(unsigned char th, unsigned char ng, unsigned char tha, unsigned char nm)
{
    RTCbits.week = DecToBCD(th);
    RTCbits.date = DecToBCD(ng);
    RTCbits.month = DecToBCD(tha);
    RTCbits.year = DecToBCD(nm);
    
    RTC_Set_DateTime(&RTCbits);
}

/*------------------------------------------------------------------[  Display time to LCD  ]------------------------------------------------------------*/
void Display_Time()
{
    RTC_Read_DateTime(&RTCbits);
    /*Display week to LCD*/
    switch(RTCbits.week)
    {
        case 1:     sprintf(&str2[0], " SUN");      break;
        case 2:     sprintf(&str2[0], " MON");      break;
        case 3:     sprintf(&str2[0], " TUE");      break;           
        case 4:     sprintf(&str2[0], " WED");      break;
        case 5:     sprintf(&str2[0], "THUR");      break;
        case 6:     sprintf(&str2[0], " FRI");      break;
        default:    sprintf(&str2[0], " SAT");      break;
    }

    AP = (RTCbits.hour & (1<<5));
    if(MODE == 12)
        /*Check AP(bit 5)*/
        if(AP == 0)
        {
            sprintf(&str1[0], " %02x:%02x:%02x AM    ", (RTCbits.hour & 0x1F), RTCbits.min, RTCbits.sec);
        }
        else
        {
            sprintf(&str1[0], " %02x:%02x:%02x PM    ", (RTCbits.hour & 0x1F), RTCbits.min, RTCbits.sec);
        }
    else if(MODE == 24)
    {
        sprintf(&str1[0], " %02x:%02x:%02x       ", (RTCbits.hour & 0x3F), RTCbits.min, RTCbits.sec);
    }

    sprintf(&str2[4], ",%02x/%02x/20%02x", RTCbits.date, RTCbits.month, RTCbits.year);
    LCD_gotoxy(1,1);
    LCD_puts(&str2[0]);
    LCD_gotoxy(2,1);
    LCD_puts(&str1[0]);
}


/*---------------------------------------------[  Set Thu, Ngay, Thang, Nam chuc, Nam dv, Gio, Phut, Giay  ]---------------------------------------------*/
void SetThu()
{
    if (buttonUp == 0)      {while(buttonUp==0);    thu = (thu==7?1:(thu+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  thu = (thu==1?7:(thu-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT THU   ");
    LCD_gotoxy(2,1);
    LCD_puts("  THU: ");
    switch(thu)
    {
        case 1:     LCD_puts("SUNDAY   ");      break;
        case 2:     LCD_puts("MONDAY   ");      break;
        case 3:     LCD_puts("TUESDAY  ");      break;           
        case 4:     LCD_puts("WEDNESDAY");      break;
        case 5:     LCD_puts("THURSDAY ");      break;
        case 6:     LCD_puts("FRIDAY   ");      break;
        default:    LCD_puts("SATUDAY  ");      break;
    }
    SetDate(thu, ngay, thang, namch*10+namdv);
}

void SetNgay()
{
    if (buttonUp == 0)      {while(buttonUp==0);    ngay = (ngay==31?1:(ngay+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  ngay = (ngay==1?31:(ngay-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT NGAY  ");
    LCD_gotoxy(2,1);
    LCD_puts("    NGAY: ");
    LCD_gotoxy(2,11);
    LCD_int(ngay);
    LCD_gotoxy(2,13);
    LCD_puts("     ");     
    SetDate(thu, ngay, thang, namch*10+namdv);
}

void SetThang()
{
    if (buttonUp == 0)      {while(buttonUp==0);    thang = (thang==12?1:(thang+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  thang = (thang==1?12:(thang-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT THANG  ");
    LCD_gotoxy(2,1);
    LCD_puts("   THANG: ");
    LCD_gotoxy(2,11);
    LCD_int(thang);
    LCD_gotoxy(2,13);
    LCD_puts("    ");  
    SetDate(thu, ngay, thang, namch*10+namdv);
}

void SetNamchuc()
{
    if (buttonUp == 0)      {while(buttonUp==0);    namch = (namch==9?0:(namch+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  namch = (namch==0?9:(namch-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT NAM  ");
    LCD_gotoxy(2,1);
    LCD_puts("   NAMCH: ");
    LCD_gotoxy(2,11);
    LCD_int(namch);
    LCD_gotoxy(2,13);
    LCD_puts("    "); 
    SetDate(thu, ngay, thang, namch*10+namdv);
}

void SetNamdv()
{
    if (buttonUp == 0)      {while(buttonUp==0);    namdv = (namdv==9?0:(namdv+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  namdv = (namdv==0?9:(namdv-1));}  
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT NAM  ");
    LCD_gotoxy(2,1);
    LCD_puts("   NAMDV: ");
    LCD_gotoxy(2,11);
    LCD_int(namdv);
    LCD_gotoxy(2,13);
    LCD_puts("    ");  
    SetDate(thu, ngay, thang, namch*10+namdv);
}

void SetGio()
{
    if (buttonUp == 0)      {while(buttonUp==0);    gio = (gio==23?0:(gio+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  gio = (gio==0?23:(gio-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT GIO  ");
    LCD_gotoxy(2,1);
    LCD_puts("     GIO: ");
    LCD_gotoxy(2,11);
    LCD_int(gio);
    LCD_gotoxy(2,13);
    LCD_puts("    ");   
    SetTime(gio, phut, giay);
}

void SetPhut()
{
    if (buttonUp == 0)      {while(buttonUp==0);    phut = (phut==59?0:(phut+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  phut = (phut==0?59:(phut-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT PHUT  ");
    LCD_gotoxy(2,1);
    LCD_puts("    PHUT: ");
    LCD_gotoxy(2,11);
    LCD_int(phut);
    LCD_gotoxy(2,13);
    LCD_puts("    ");    
    SetTime(gio, phut, giay);
}

void SetGiay()
{
    if (buttonUp == 0)      {while(buttonUp==0);    giay = (giay==59?0:(giay+1));}
    if (buttonDown == 0)    {while(buttonDown==0);  giay = (giay==0?59:(giay-1));}
    LCD_gotoxy(1,1);
    LCD_puts("  CAI DAT GIAY  ");
    LCD_gotoxy(2,1);
    LCD_puts("    GIAY: ");
    LCD_gotoxy(2,11);
    LCD_int(giay);
    LCD_gotoxy(2,13);
    LCD_puts("    ");    
    SetTime(gio, phut, giay);
}


/*----------------------------------------------------------------[  Configure Time and Date  ]----------------------------------------------------------*/
void Select_Menu(void)
{
    if(buttonMenu == 0)
    {
        while(buttonMenu==0);
        demmenu = demmenu + 1;
        if(demmenu > 8)
        {
            LCD_gotoxy(1,1);
            LCD_puts("  SAVE SETTING  ");
            LCD_gotoxy(2,1);
            LCD_puts("  PROGRESS....  ");
            __delay_ms(1000);
            demmenu=0;
        }                
    }
    switch(demmenu)      
    {
        case 0:     Display_Time();     break;
        case 1:     SetThu();           break;
        case 2:     SetNgay();          break;
        case 3:     SetThang();         break;
        case 4:     SetNamchuc();       break;
        case 5:     SetNamdv();         break;
        case 6:     SetGio();           break;
        case 7:     SetPhut();          break;
        case 8:     SetGiay();          break;
    }
}
unsigned int g, ph, gi, tt=0;

/*----------------------------------------------------------[  Convert between Mode 12H and Mode 24H  ]--------------------------------------------------*/
void SetMode(void)
{
    /*Mode 12H:*/
    if(buttonMode==0 && tt==0)
    {
        while(buttonMode==0);
        tt=1; 
        
        /*If hour>0h and hour<=12h(In Mode 24h)*/
        if(BCDtoDec(RTCbits.hour)>0 && BCDtoDec(RTCbits.hour)<12)
            /*Mode(bit6)=1*/
            g=BCDtoDec(RTCbits.hour|(Mode<<6));
        /*If hour>12h and hour<=23h(In Mode 24h)*/
        if(BCDtoDec(RTCbits.hour)>12 && BCDtoDec(RTCbits.hour)<=23)
            /*Mode(bit6)=1, AP(bit5)=1*/
            g=BCDtoDec(DecToBCD(BCDtoDec(RTCbits.hour)-12)|(3<<5));
        /*If hour=0h(In Mode 24h)*/
        if(BCDtoDec(RTCbits.hour)==0)
            g=BCDtoDec(DecToBCD(BCDtoDec(RTCbits.hour)+12)|(Mode<<6));
        /*If hour=12h(In Mode 24h)*/
        if(BCDtoDec(RTCbits.hour)==12)
            /*Mode(bit6)=1, AP(bit5)=1*/
            g=BCDtoDec(RTCbits.hour|(3<<5));

        ph=BCDtoDec(RTCbits.min);
        gi=BCDtoDec(RTCbits.sec);            
        SetTime(g, ph, gi);   
        MODE = 12;
    }
    
    /*Mode 24H:*/
    if(buttonMode==0 && tt==1)
    {
        while(buttonMode==0);
        tt=0;
        
        /*If hour is AM and hour<12h(In Mode 12h)*/
        if((RTCbits.hour & (1<<5))==0 && BCDtoDec(RTCbits.hour& ~(3<<5))<12) 
            /*Mode(bit6)=1*/
            g=BCDtoDec(RTCbits.hour&~(Mode<<6));
        /*If hour is AM and hour=12h(In Mode 12h)*/
        if((RTCbits.hour & (1<<5))==0 && BCDtoDec(RTCbits.hour& ~(3<<5))==12)
            g=BCDtoDec(RTCbits.hour&~(Mode<<6))-12;
        /*If hour is PM and hour<12h(In Mode 12h)*/
        if((RTCbits.hour & (1<<5))!=0 && BCDtoDec(RTCbits.hour& ~(3<<5))<12)
            /*Mode(bit6)=0, AP(bit5)=0*/
            g=BCDtoDec(RTCbits.hour&~(3<<5))+12;
        /*If hour is PM and hour=12h(In Mode 12h)*/
        if((RTCbits.hour & (1<<5))!=0 && BCDtoDec(RTCbits.hour& ~(3<<5))==12)
            /*Mode(bit6)=0, AP(bit5)=0*/
            g=BCDtoDec(RTCbits.hour&~(3<<5));
        ph=BCDtoDec(RTCbits.min);
        gi=BCDtoDec(RTCbits.sec);
        MODE = 24;
        SetTime(g, ph, gi);
    }
    /*Notice the position between MODE=...; and SetTime(g, ph, gi);*/
}


/*-------------------------------------------------------------[  Display Alarm Mode to LCD  ]-----------------------------------------------------------*/
void Display_Alarm(int Vitri)
{  
    LCD_gotoxy(2,1);
    LCD_puts("    "); 
    if(Vitri==1)    {LCD_puts("__");} 
    else            {LCD_gotoxy(2,5);       LCD_int(giobt);}
    LCD_putc(':');
    
    if(Vitri==2)    {LCD_puts("__");} 
    else            {LCD_gotoxy(2,8);       LCD_int(phutbt);}
    LCD_putc(':');
    
    if(Vitri==3)    {LCD_puts("__");} 
    else            {LCD_gotoxy(2,11);       LCD_int(giaybt);}             
    LCD_puts("    ");    
}

/*--------------------------------------------------------------------[  Setup Alarm  ]------------------------------------------------------------------*/
void Setup_Alarm()
{
    int BthucMode=1, dis=0;
    LCD_gotoxy(1,1);
    LCD_puts("CAI DAT BAO THUC");
    while(1)
    {
        if(dis<5)       Display_Alarm(0);       //dis=0->4: Hien thi Gia tr? thoi gian Bao thuc can chinh
        else            Display_Alarm(BthucMode);    //dis=5->9: Hien thi dau '__' tai gia tri duoc chon 
        dis++; 
        if(dis>9)  dis=0; 
        
        /*Khi nhan nut chinh bao thuc*/
        if(buttonAlarm==0)   
        {      
            while(buttonAlarm==0);                  
            BthucMode++;
            if(BthucMode>3)
            {
                LCD_gotoxy(1,1);
                LCD_puts("  SAVE SETTING  ");
                LCD_gotoxy(2,1);
                LCD_puts("  PROGRESS....  ");
                __delay_ms(1000);
                /*Thoat khoi ham chinh bao thuc sau khi chinh xong  3 gia tri bao thuc*/
                break;
            }
        }
        
        if(buttonUp==0)
        {                                  
            while(buttonUp==0);
            dis=0;                                                                                  
            switch(BthucMode)                                                          
            {
                case 1:     giobt = (giobt==23?0:(giobt+1));        break;                   
                case 2:     phutbt = (phutbt==59?0:(phutbt+1));     break;
                case 3:     giaybt = (giaybt==59?0:(giaybt+1));     break;
            }
        }   
 
        if(buttonDown==0)
        {  
            while(buttonDown==0);
            dis=0;                   
            switch(BthucMode)                                                          
            {
                case 1:     giobt = (giobt==0?23:(giobt-1));        break;                   
                case 2:     phutbt = (phutbt==0?59:(phutbt-1));     break;
                case 3:     giaybt = (giaybt==0?59:(giaybt-1));     break;
            }
        }
        
        /*Neu dang cai dat Bao thuc, nhan nut Chinh gio -> Ve man hinh Hien thi thoi gian chinh*/
        if(buttonMenu==0) 
        { 
            while(buttonMenu==0);
            break;
        }
    }    
}