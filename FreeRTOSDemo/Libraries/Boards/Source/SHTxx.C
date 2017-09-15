#include <math.h>
#include "SHTxx.h"
#include "hz.h"
void delay_us(unsigned long count)
{  
   int i;
   for(;count>0;count--)//delay count*1us
   {
    for(i=0;i<100;i++)
    {  
     __asm("nop");
     __asm("nop");
     __asm("nop");
    }
   }
}
void delay_ms(unsigned long count)
{
    delay_us(count*1000);
}
int SHT_init(void)
{
    if (GPIO_Config(&shtdata_pin_out)!= E_NO_ERROR) {
        return E_UNKNOWN;
    }
    if (GPIO_Config(&shtsck_pin) != E_NO_ERROR)  {
        return E_UNKNOWN;
    }
      return E_NO_ERROR;
}
static int read_sda(void)
{
    int proc_result = -1;
    int ret ;
     ret =GPIO_InGet(&shtdata_pin_in);
    if(ret==shtdata_pin_in.mask)
    {
     proc_result= 1;
    // printf("sda value %x ------mask %x",ret,shtdata_pin_in.mask);
    }
    else proc_result=0;
    return (proc_result);
}

/*
函数名称：s_write_byte（）
返回值：error0
*/
char s_write_byte(unsigned char value)
{
    unsigned char i,error0=0;
    DATA_OUT;
    for (i=0x80; i>0; i/=2)           //shift bit for masking
    {
        if(i & value) {
            DATA_H;   //masking value with i , write to SENSI-BUS
        }
        else  {
            DATA_L;
        }
        SCK_H;                          //clk for SENSI-BUS
        delay_us(5);
        SCK_L;
        delay_us(5);

    }
    //DATA_H;							//	释放数据线
    DATA_IN;
    SCK_H;
    delay_us(20);
    error0=read_sda();   //check ack (DATA will be pulled down by SHT11)
    delay_us(5);
    DATA_OUT;
    DATA_H;
    SCK_L;
    return error0;                     //error0=1 in case of no acknowledge
}

//----------------------------------------------------------------------------------
unsigned char s_read_byte(unsigned char ack)
//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
{
    unsigned char i,val=0;
    //DATA_OUT;SCK_OUT;
    //DATA_H;
    DATA_IN;
    for(i=0x80; i>0; i/=2)
    {
        SCK_H;
        if(read_sda()==1) val=(val | i);
        else   val=(val | 0x00);
        SCK_L;
        delay_us(5);
    }
    DATA_OUT;
    DATA_H;
    delay_us(3);
    if(ack==1)
    {
        DATA_L;
    }
    else
    {
        DATA_H;
    }
    SCK_H;
    delay_us(5);
    SCK_L;
    DATA_H;	   									//	读第二字节数据之前，DATA应为高
    return val;
}

//----------------------------------------------------------------------------------
void s_transstart(void)
//----------------------------------------------------------------------------------
// generates a transmission start
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
{
    DATA_OUT;

    DATA_H;
    SCK_L; 	//	此处必须要保留
    delay_us(2);

    SCK_H;
    delay_us(2);

    DATA_L;
    delay_us(2);

    SCK_L;
    delay_us(2);

    SCK_H;
    delay_us(2);

    DATA_H;
    delay_us(2);

    SCK_L;
    delay_us(2);
}

//----------------------------------------------------------------------------------
void s_connectionreset(void)
//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{
    unsigned char i;
    DATA_OUT;
    DATA_H;
    SCK_L;                    //Initial state
    for(i=0; i<9; i++)                //9 SCK cycles
    {
        SCK_H;
        delay_us(2);
        SCK_L;
        delay_us(2);
    }

    s_transstart();                   //transmission start
}

//----------------------------------------------------------------------------------
char s_softreset(void)
//----------------------------------------------------------------------------------
// resets the sensor by a softreset
{
    unsigned char error0=0;
    s_connectionreset();              //reset communication
    error0+=s_write_byte(RESET);       //send RESET-command to sensor
    return error0;                     //error0=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
char s_measure(unsigned char *p_value, unsigned int *p_checksum, unsigned char mode)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{
    unsigned int error0=0;
    unsigned int i,j;

    s_transstart();                   //transmission start
    switch(mode)
    {   //send command to sensor
    case TEMP :
        error0+=s_write_byte(MEASURE_TEMP);
        break;
    case HUMI :
        error0+=s_write_byte(MEASURE_HUMI);
        break;
    default     :
        break;
    }
    DATA_IN;
    for (i=0; i<1; i++)
    {
        for(j=0; j<65535; j++)
        {
            if(read_sda()==0) {
                break;   //wait until sensor has finished the measurement
            }
            delay_us(5);
        }
    }
    if(read_sda()) {
        error0+=1;   // or timeout (~2 sec.) is reached
    }

    *(p_value+1)    =   s_read_byte(ACK);    			//read the first byte (MSB)
    *(p_value)  =	  s_read_byte(ACK); 			//read the second byte (LSB)
    *p_checksum   =	  s_read_byte(noACK); 			//read checksum	 */
    return error0;
}

//----------------------------------------------------------------------------------------
void calc_sth11(float *p_humidity ,float *p_temperature)
//----------------------------------------------------------------------------------------
// calculates temperature and humidity [%RH]
// input :  humi [Ticks] (12 bit)
//          temp [Ticks] (14 bit)
// output:  humi [%RH]
//          temp
{
    const float C1=-4.0;              	// for 12 Bit
    const float C2=+0.0405;           	// for 12 Bit
    const float C3=-0.0000028;        	// for 12 Bit
    const float T1=+0.01;             	// for 14 Bit @ 5V
    const float T2=+0.00008;           	// for 14 Bit @ 5V

    float rh=*p_humidity;             	// rh:      Humidity [Ticks] 12 Bit
    float t=*p_temperature;           	// t:       Temperature [Ticks] 14 Bit
    float rh_lin;                     	// rh_lin:  Humidity linear
    float rh_true;                    	// rh_true: Temperature compensated humidity
    float t_C;                        	// t_C   :  Temperature [癈]

    t_C=t*0.01 - 39.6;                  	//calc. temperature from ticks to [癈]
    rh_lin=C3*rh*rh + C2*rh + C1;     	//calc. humidity from ticks to [%RH]
    rh_true=(t_C-25)*(T1+T2*rh)+rh_lin; //calc. temperature compensated humidity [%RH]
    if(rh_true>100)rh_true=100;       	//cut if the value is outside of
    if(rh_true<0.1)rh_true=0.1;       	//the physical possible range

    *p_temperature=t_C;               	//return temperature [癈]
    *p_humidity=rh_true;              	//return humidity[%RH]
}
//--------------------------------------------------------------------
//float calc_dewpoint(float h,float t)
//--------------------------------------------------------------------
// calculates dew point
// input:   humidity [%RH], temperature [癈]
// output:  dew point [癈]
float calc_dewpoint(float h,float t)
{
    float logEx,dew_point;
    logEx=0.66077+7.5*t/(237.3+t)+(log10(h)-2);
    dew_point = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);
    return dew_point;
}
//---------------------------------------------------------------------
void ConverFloatToChar(float flo,char * ptr)
{
    int i=0,intnum,tmp,tmp1;
    float data;
    data = flo;
    while(i++<8) *(ptr+i-1) = 0;
    i = 0;
    while(data >= 1)
    {
        data = data/10;
        i++;
    }
    intnum=i;
    if(!intnum)
    {
        *ptr = 0;
        *(ptr+1) = '.';
        data = flo;
        for(i=2; i<=3; i++)
        {
            data *= 10;
            tmp = data;
            *(ptr+i) = tmp+48;
            data = data- tmp;
        }
    }
    else
    {
        *(ptr+intnum) = '.';
        tmp = flo;
        for(i=1; i<=intnum; i++)
        {
            tmp1 = tmp % 10;
            *(ptr+intnum-i) = tmp1+48;
            tmp = tmp/10;
        }
        data = flo;
        tmp = data;
        data = data - tmp;
        for(i=intnum+1; i<6; i++)
        {
            data *= 10;
            tmp = data;
            *(ptr+i) = tmp+48;
            data = data- tmp;
        }
    }
}
void float2char1(float floatVariable,unsigned char *result)
{
    //unsigned char dest[2];
    //result=dest;
    result[0]=((int)(floatVariable*100)&0xff00)>>8;
    result[1]=((int)(floatVariable*100)&0xff);
}
void TH_display1(void)
{
    value humi_val,temp_val;
    
    float dew_point;
    unsigned char error0;
    unsigned int checksum;
    error0=0;
    s_connectionreset();
    error0+=s_measure((unsigned char*)&humi_val.i,&checksum,HUMI);  //measure humidity
    error0+=s_measure((unsigned char*)&temp_val.i,&checksum,TEMP);  //measure temperature
    if(error0!=0)
    {
        s_connectionreset();
    } //in case of an error0: connection reset
    else
    {
        humi_val.f=(float)humi_val.i;               //converts integer to float
        temp_val.f=(float)temp_val.i;               //converts integer to float
        calc_sth11(&humi_val.f,&temp_val.f);        //calculate humidity, temperature
        //		dew_point=calc_dewpoint(humi_val.f,temp_val.f);
        printf("wendu %f,shidu %f\n\r",temp_val.f,humi_val.f);
			  board_sensor.temp=(int)(temp_val.f*100)/100;
			  board_sensor.humi=(int)(humi_val.f*100)/100;
      }
    SCK_L;
    delay_ms(50);    //(be sure that the compiler doesn't eliminate this line!)
    s_softreset();
    delay_ms(50);

}
void TH_display(void)
{
        uint8_t strbuf[32];
        sprintf(strbuf,"%.1f", board_sensor.temp);
        GUI_LoadPic(0, 0, wendu, 18, 30);
        GUI_PutChar24_32_str(24,32,"   ");
        GUI_PutChar24_32_str(24,32,strbuf); 
        GUI_LoadPic(0, 33, shidu, 18, 28);
        sprintf(strbuf,"%.1f", board_sensor.humi);
        GUI_PutChar24_32_str(24,0,"   ");
        GUI_PutChar24_32_str(24,0,strbuf); 
}
//END OF FILE
