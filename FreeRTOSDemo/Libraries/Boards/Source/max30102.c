/** \file main.cpp ******************************************************
*
* Project: MAXREFDES117#
* Filename: main.cpp
* Description: This module contains the Main application for the MAXREFDES117 example program.
*
*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */
/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
/*!\mainpage Main Page
*
* \section intro_sec Introduction
*
* This is the code documentation for the MAXREFDES117# subsystem reference design.
* 
*  The Files page contains the File List page and the Globals page.
* 
*  The Globals page contains the Functions, Variables, and Macros sub-pages.
*
* \image html MAXREFDES117_Block_Diagram.png "MAXREFDES117# System Block Diagram"
* 
* \image html MAXREFDES117_firmware_Flowchart.png "MAXREFDES117# Firmware Flowchart"
*
*/

#include "MAX30102.h"
// Standard includes
#include <stdio.h>
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
#include <lcd12864.h>
/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "hz.h"
#define MAX_BRIGHTNESS 255

uint32_t aun_ir_buffer[500]; //IR LED sensor data
int32_t n_ir_buffer_length;    //data length
uint32_t aun_red_buffer[500];    //Red LED sensor data
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;
int32_t n_sp02_tmp=0; //SPO2 value
int32_t n_heart_rate_tmp=0;  
int  max30102_io_init(void)
{
 /* Configure the initial state of LDO2 */
 /* Configure GPIO for interrupt pin from PMIC */
 /* Setup the I2CM Peripheral to talk to the MAX14690 */
    I2CM_Init(MAX30102_ICM, &max30102_sys_cfg, I2CM_SPEED_400KHZ);
    if (GPIO_Config(&max30102_int) != E_NO_ERROR) {
        return E_UNKNOWN;
    }
    else
      return E_NO_ERROR;
    
}
uint8_t red_io(void)
{
    /* Configure and enable interrupt */
  uint32_t ret;
  ret=GPIO_InGet(&max30102_int);
  //printf("pin %d-----------\n\r",ret);
   if(ret!=0)return 1;
   else return 0;
}

// the setup routine runs once when you press reset:
int test(void) 
 { 
    uint32_t un_min, un_max, un_prev_data;  //variables to calculate the on-board LED brightness that reflects the heartbeats
    int i;
    int32_t n_brightness;
    float f_temp;
    int j,sendcount;
    uint8_t strbuf[32];
    int hight;
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    maxim_max30102_reset(); //resets the MAX30102    
    while(state==0)vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/50); 
    
    //read and clear status register
    Read_One_Byte(0x00,&uch_dummy);    
    if(maxim_max30102_init()==false)
    {
      printf("max30102 init err\n\r");  //initializes the MAX30102 
      maxim_max30102_reset(); //resets the MAX30102
      //read and clear status register
      Read_One_Byte(0x00,&uch_dummy);  
      maxim_max30102_init();
    }        
    n_brightness=0;
    un_min=0x3FFFF;
    un_max=0;
    n_ir_buffer_length=500; //buffer length of 100 stores 5 seconds of samples running at 100sps     
    printf("testing......");
    //read the first 500 samples, and determine the signal range
    for(i=0;i<n_ir_buffer_length;i++)
    {
       while(red_io()==1);//vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/500);   //wait until the interrupt pin asserts
       maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO            
   /*     if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];    //update signal min
        if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];    //update signal max
       */  
        if(i%100==0) printf("testing......%d\n\r",i/100);
    }
    //un_prev_data=aun_red_buffer[i];   
    printf("max30102 500sps get  ok\n\r");  //initializes the MAX30102 
    //calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
    GUI_ClearSCR();
    //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
    while(state==4)
    {
        i=0;
        un_min=0x3FFFF;
        un_max=0;
        
        //dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
        for(i=100;i<500;i++)
        {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
      /*      //update the signal min and max
            if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];
            if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];
            */
        }
        
        //take 100 sets of samples before calculating the heart rate.
        for(i=400;i<500;i++)
        {
            //un_prev_data=aun_red_buffer[i-1];
            while(red_io()==1);//vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/500);
            maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));   
            
       /*     if(aun_red_buffer[i]>un_prev_data)
            {
                f_temp=aun_red_buffer[i]-un_prev_data;
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness-=(int)f_temp;
                if(n_brightness<0)
                    n_brightness=0;
            }
            else
            {
                f_temp=un_prev_data-aun_red_buffer[i];
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness+=(int)f_temp;
                if(n_brightness>MAX_BRIGHTNESS)
                    n_brightness=MAX_BRIGHTNESS;
            }*/
#if defined(TARGET_KL25Z) || defined(TARGET_MAX32600MBED)
            led.write(1-(float)n_brightness/256);
#endif
            //send samples and calculation result to terminal program through UART
        if(1)
        {    if((n_heart_rate_tmp!=n_heart_rate)|(n_sp02_tmp!=n_sp02))
            {
            printf("red=");
            printf("%i", aun_red_buffer[i]);
            printf(", ir=");
            printf("%i", aun_ir_buffer[i]);
            printf(", HR=%i, ", n_heart_rate); 
            printf("HRvalid=%i, ", ch_hr_valid);
            printf("SpO2=%i, ", n_sp02);
            printf("SPO2Valid=%i\n\r", ch_spo2_valid);
            n_heart_rate_tmp=n_heart_rate;
            n_sp02_tmp=n_sp02;
            if((ch_hr_valid)&(n_heart_rate<150))
            {  
            sprintf(strbuf,"%d",n_heart_rate); 
            }
            else
            {  
            sprintf(strbuf,"..."); 
            }
          //  GUI_PutHZ(2,8,code001,17,19);
          //  GUI_PutHZ(20,8,code002,18,19); 
            GUI_LoadPic(0, 0, hart, 35, 30);
            GUI_PutChar24_32_str(48,0,"   ");
            GUI_PutChar24_32_str(48,0,strbuf);
      
            if(ch_spo2_valid)
            {
              sprintf(strbuf,"%d%%",n_sp02);
            }
            else
            {  
            sprintf(strbuf,"..."); 
            }
           // GUI_PutHZ(2,40,code003,17,18);
            //GUI_PutHZ(20,40,code004,17,17);
            GUI_LoadPic(0, 33, spo2, 35, 30);
            GUI_PutChar24_32_str(48,32,"   ");
            GUI_PutChar24_32_str(48,32,strbuf);         
            GUI_Exec();
            }
        }
        }
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);   
      if(ch_hr_valid)
			 board_sensor.hartbeat=n_heart_rate;
			if(ch_spo2_valid)
			 board_sensor.sop2=n_sp02;	
				vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/500);      
    }
     maxim_max30102_reset(); //resets the MAX30102
}
                                         
 