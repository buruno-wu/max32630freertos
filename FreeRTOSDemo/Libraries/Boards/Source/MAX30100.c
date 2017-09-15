/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include <Wire.h>

#include "MAX30100.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mxc_config.h"
#include "board.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
//#include <unistd.h>
#include <stdlib.h>
//#include <sys/stat.h>

/* FreeRTOS */
#include <stddef.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "max14690.h"
#include "board.h"
#include "i2cm.h"
#include "lp.h"

uint32_t rawIRValue;
uint32_t rawRedValue;
void delay_nms(int i)
{
  int j;
for(;i>0;i--)
     for(j=0;j<5000;j++);
}

uint8_t maxim_max30102_init()
/**
* \brief        Initialize the MAX30102
* \par          Details
*               This function initializes the MAX30102
*
* \param        None
*
* \retval       true on success
*/
{
  if(!Write_One_Byte(REG_INTR_ENABLE_1,0xc0)) // INTR setting
    return false;
  if(!Write_One_Byte(REG_INTR_ENABLE_2,0x00))
    return false;
  if(!Write_One_Byte(REG_FIFO_WR_PTR,0x00))  //FIFO_WR_PTR[4:0]
    return false;
  if(!Write_One_Byte(REG_OVF_COUNTER,0x00))  //OVF_COUNTER[4:0]
    return false;
  if(!Write_One_Byte(REG_FIFO_RD_PTR,0x00))  //FIFO_RD_PTR[4:0]
    return false;
  if(!Write_One_Byte(REG_FIFO_CONFIG,0x0f))  //sample avg = 1, fifo rollover=false, fifo almost full = 17
    return false;
  if(!Write_One_Byte(REG_MODE_CONFIG,0x03))   //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
    return false;
  if(!Write_One_Byte(REG_SPO2_CONFIG,0x27))  // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
    return false;
  
  if(!Write_One_Byte(REG_LED1_PA,0x24))   //Choose value for ~ 7mA for LED1
    return false;
  if(!Write_One_Byte(REG_LED2_PA,0x24))   // Choose value for ~ 7mA for LED2
    return false;
  if(!Write_One_Byte(REG_PILOT_PA,0x7f))   // Choose value for ~ 25mA for Pilot LED
    return false;
  return true;  
}

bool maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led);

///////////////////////////////////////
uint8_t Write_One_Byte_stop(uint8_t address)
{
 //注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)
 uint8_t tx_data[1] = {address};
if( I2CM_Write(MAX30102_ICM, MAX30102_ADDR, NULL,0, tx_data, 1)==1)
return 0;
else
 return 1;
}
bool maxim_max30102_reset()
/**
* \brief        Reset the MAX30102
* \par          Details
*               This function resets the MAX30102
*
* \param        None
*
* \retval       true on success
*/
{
    if(!Write_One_Byte(REG_MODE_CONFIG,0x40))
        return false;
    else
        return true;    
}


bool maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)
/**
* \brief        Read a set of samples from the MAX30102 FIFO register
* \par          Details
*               This function reads a set of samples from the MAX30102 FIFO register
*
* \param[out]   *pun_red_led   - pointer that stores the red LED reading data
* \param[out]   *pun_ir_led    - pointer that stores the IR LED reading data
*
* \retval       true on success
*/
{
  uint32_t un_temp;
  unsigned char uch_temp;
  *pun_red_led=0;
  *pun_ir_led=0;
  uint8_t ach_i2c_data[6];  

  //read and clear status register
  if(Read_One_Byte(REG_INTR_STATUS_1,&uch_temp)==0)return false ;
  if(Read_One_Byte(REG_INTR_STATUS_2,&uch_temp)==0)return false ;  
  ach_i2c_data[0]=REG_FIFO_DATA;
  Write_One_Byte_stop(ach_i2c_data[0]);
  if(Buff_Read(REG_FIFO_DATA, ach_i2c_data, 6)==0)return false ;
  un_temp=(unsigned char) ach_i2c_data[0];
  un_temp<<=16;
  *pun_red_led+=un_temp;
  un_temp=(unsigned char) ach_i2c_data[1];
  un_temp<<=8;
  *pun_red_led+=un_temp;
  un_temp=(unsigned char) ach_i2c_data[2];
  *pun_red_led+=un_temp;  
  un_temp=(unsigned char) ach_i2c_data[3];
  un_temp<<=16;
  *pun_ir_led+=un_temp;
  un_temp=(unsigned char) ach_i2c_data[4];
  un_temp<<=8;
  *pun_ir_led+=un_temp;
  un_temp=(unsigned char) ach_i2c_data[5];
  *pun_ir_led+=un_temp;
  *pun_red_led&=0x03FFFF;  //Mask MSB [23:18]
  *pun_ir_led&=0x03FFFF;  //Mask MSB [23:18]    
  return true;
}





uint8_t Write_One_Byte(uint8_t address,uint8_t val)
{
 //注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)
 uint8_t tx_data[2] = {address, val};
if( I2CM_Write(MAX30102_ICM, MAX30102_ADDR, NULL,0, tx_data, 2)==2)
 {
 return 1;
 }
 else return 0;
}

uint8_t Read_One_Byte(uint8_t address,uint8_t *result)//读字节
{
  uint8_t ret;
  ret=I2CM_Read(MAX30102_ICM, MAX30102_ADDR, &address,1,result,1);
 if (ret==1)
{
//Report("read value is 0x%x\n\r",result);
 
return 1;
}
else
{
printf("read err %d\n\r",ret);
return 0;
}
}

uint8_t Buff_write (uint8_t *buff,uint8_t address,uint8_t length)
{
  uint8_t tx_data[64];
  tx_data[0]=address;
  memcpy(&tx_data[1],buff,length);  
  if(I2CM_Write(MAX30102_ICM, MAX30102_ADDR, NULL,0, tx_data, length+1)==length+1){
   
    return 1;
  }
  else
  {printf("write err\n\r");
    return 0;
  }
}
uint8_t Buff_Read2(uint8_t address,uint8_t *buff,uint8_t length)//读字符串

{
  int i;
  for(i=0;i<length;i++)
  {
    if(I2CM_Read(MAX30102_ICM, MAX30102_ADDR, &address,1,&buff[i],1)==1)
  { 
   // address++;
   // printf("%d read value is 0x%x\n\r",i,buff[i]);
     return 1;
  }   
  else return 0; 
  }
  
}
uint8_t Buff_Read(uint8_t address,uint8_t *buff,uint8_t length)//读字符串

{
  int i;
 if (I2CM_Read(MAX30102_ICM, MAX30102_ADDR, &address,1,buff,length)==length)
  { 
   // for(i=0;i<length;i++)
   // Report("%d read value is 0x%x\n\r",i,buff[i]);
   //  MAP_UtilsDelay(800000);
    return 1;

  }   
  else return 0; 
  }
  