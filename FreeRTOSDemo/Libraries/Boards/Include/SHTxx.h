#ifndef __SHTXX_H
#define __SHTXX_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"

void delay_ms(unsigned long count);

typedef union 
{ 
	unsigned short i;
  	float f;
} value;
extern value humi_val,temp_val;
enum {TEMP,HUMI};

#define noACK 0
#define ACK   1
                            //adr  command  r/w
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET        0x1e   //000   1111    0

//sbit SCK = PB2;
//sbit DATA = PB3;


#define SCK_H   GPIO_OutSet(&shtsck_pin)
#define SCK_L   GPIO_OutClr(&shtsck_pin)

#define DATA_H   GPIO_OutSet(&shtdata_pin_out)  
#define DATA_L   GPIO_OutClr(&shtdata_pin_out)

#define SCK_read        GPIOPinRead(GPIO_PORTB_BASE, SCK )
#define DATA_IN         GPIO_Config(&shtdata_pin_in)
#define DATA_OUT        GPIO_Config(&shtdata_pin_out)
//#define DATA_READ       (GPIO_InGet(&shtdata_pin_in)!=0)? 0 : 1

void s_connectionreset(void);
char s_measure(unsigned char *p_value, unsigned int *p_checksum, unsigned char mode);
void calc_sth11(float *p_humidity ,float *p_temperature);
float calc_dewpoint(float h,float t);
int SHT_init(void);
void ConverFloatToChar(float flo,char * ptr);
char s_softreset(void);
void TH_display(void);
void TH_display1(void);
#endif 

