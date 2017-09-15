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

/* config.h is the required application configuration; RAM layout, stack, chip type etc. */
#include "mxc_config.h"
#include "board.h"
#include "imu.h"
#include "hz.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//#include <unistd.h>
#include <stdlib.h>
//#include <sys/stat.h>
#include <lcd12864.h>
/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "arm_math.h"
/* FreeRTOS+ */
//#include "FreeRTOS_CLI.h"
#include "config.h"
/* Maxim CMSIS SDK */
#include "rtc.h"
#include "uart.h"
#include "lp.h"
#include "max30102.h"
#include "vbatmon.h"
#include "SHTxx.h"
#include  "rtcop.h"
#include  "bsp_uart.h"
/* Mutual exclusion (mutex) semaphores */
SemaphoreHandle_t xGPIOmutex;
void t3aix_data(void);
/* Task IDs */
TaskHandle_t cmd_task_id;

/* Enables/disables LP1 tick-less mode */
unsigned int disable_lp1 = 1;

/* Stringification macros */
#define STRING(x) STRING_(x)
#define STRING_(x) #x

/* Array sizes */
#define CMD_LINE_BUF_SIZE  80
#define OUTPUT_BUF_SIZE  512

/* =| vTask0 |============================================
 *
 * This task blinks LED0 at a 0.5Hz rate, and does not
 *  drift due to the use of vTaskDelayUntil(). It may have
 *  jitter, however, due to any higher-priority task or
 *  interrupt causing delays in scheduling.
 *
 * =======================================================
 */
void vTask0(void *pvParameters)
{
    TickType_t xLastWakeTime;
    unsigned int x = LED_OFF;
    /* Get task start time */
    xLastWakeTime = xTaskGetTickCount();
    SHT_init();
    while (1) {
        if (x == LED_OFF) {
            LED_On(0);
            x = LED_ON;
        } else {
            LED_Off(0);
            x = LED_OFF;
        }
        ESP_UART_init();
        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ);
        // t3aix_data();
    }
}

/* =| vTask1 |============================================
 *
 * This task blinks LED1 at a 0.5Hz rate, and does not
 *  drift due to the use of vTaskDelayUntil(). It may have
 *  jitter, however, due to any higher-priority task or
 *  interrupt causing delays in scheduling.
 *
 * =======================================================
 */
struct tm gtime;
struct tm gcurtime;
uint8_t timeok;
void vTask1(void *pvParameters)
{
    TickType_t xLastWakeTime;
    unsigned int x = LED_ON;
    /* Get task start time */
    xLastWakeTime = xTaskGetTickCount();
    vbatain_init();
    bsp_RTC_init();
    gcurtime.tm_hour=1;
    gcurtime.tm_sec=15;
    gcurtime.tm_min=30;
    gcurtime.tm_year=2017;
    gcurtime.tm_mon=8;
    gcurtime.tm_mday=16;
    gtime=Time_GetCalendarTime();
    if(gtime.tm_year==1970)
        Time_SetCalendarTime(gcurtime);
    timeok=0;
    while (1) {
        if (x == LED_OFF) {
            LED_On(1);
            x = LED_ON;
        } else {
            LED_Off(1);
            x = LED_OFF;
        }
        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ);
//      gsencond++;
//      if(gsencond>59)
//      {
//        gsencond=0;
//        gminite=+1;
//        if(gminite>59)
//        {
//          ghours+=1;
//          if(ghours>11)ghours=0;
//        }
//      }
        gtime=Time_GetCalendarTime();
        timeok=1;
        vbat_get();
        //TH_display1();
    }
    /* Wait 1 second until next run */
}
void vCmdLineTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
     /* Get task start time */
    xLastWakeTime = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ);
        TH_display1();
    }
    /* Wait 1 second until next run */
}
/* =| vTickTockTask |============================================
 *
 * This task writes the current RTOS tick time to the console
 *
 * =======================================================
 */

void t3aix_data(void)
{   uint8_t strbuf[64];
    uint8_t str[16];
    Prepare_Data();
    Get_Attitude();
    // printf("angle x:%f,y:%f,z:%f\n",Q_ANGLE.X,Q_ANGLE.Y,Q_ANGLE.Z);
    //  uart_report_imu();
    /*   sprintf(str,"x:%f",Q_ANGLE.X);
       GUI_PutString(1,16,str);
       sprintf(str,"y:%f",Q_ANGLE.Y);
       GUI_PutString(1,24,str);
       sprintf(str,"z:%f",Q_ANGLE.Z);
       GUI_PutString(1,32,str);*/
    // sprintf(strbuf,"angle x:%f,y:%f,z:%f\n",Q_ANGLE.X,Q_ANGLE.Y,Q_ANGLE.Z);

}
void draw_dgree(void)
{
    int orgin_x,orgin_y;
    int r1,r2;
    int start_x, start_y, end_x,end_y;
    int i;
    r1=28;
    r2=32;
    orgin_x=64;
    orgin_y=32;
    for(i=0; i<12; i++)
    {
        start_x=orgin_x+r1*cos(i*PI/6);
        start_y=orgin_y+r1*sin(i*PI/6);
        end_x=orgin_x+r2*cos(i*PI/6);
        end_y=orgin_y+r2*sin(i*PI/6);
        GUI_LineWith(start_x, start_y, end_x,end_y,2,disp_color);
    }
}
void draw_arrow(int hours,int minites, int seconds,int flag)
{
    int orgin_x,orgin_y;
    int r1,r2,r3;
    int end_x,end_y;
    int i;
    r1=24;
    r2=22;
    r3=16;
    orgin_x=64;
    orgin_y=32;
    TCOLOR  color;
    if(flag)color=disp_color;
    else color=back_color;
//hours
    {
        end_x=orgin_x+r3*cos(hours*PI/6-PI/2);
        end_y=orgin_y+r3*sin(hours*PI/6-PI/2);
        GUI_LineWith(orgin_x, orgin_y, end_x,end_y,1,color);
    }
//minites
    {
        end_x=orgin_x+r2*cos(minites*PI/30-PI/2);
        end_y=orgin_y+r2*sin(minites*PI/30-PI/2);
        GUI_LineWith(orgin_x, orgin_y, end_x,end_y,1,color);
    }
//seconds
    {
        end_x=orgin_x+r1*cos(seconds*PI/30-PI/2);
        end_y=orgin_y+r1*sin(seconds*PI/30-PI/2);
        GUI_LineWith(orgin_x, orgin_y, end_x,end_y,1,color);
    }
}

void vTickTockTask(void *pvParameters)
{
    TickType_t ticks = 0;
    TickType_t xLastWakeTime;
    /* Get task start time */
    uint8_t str[32];
    int timesum;
    int oldh,oldm,olds;
    xLastWakeTime = xTaskGetTickCount();
    board_sensor.temp=0.0;
    board_sensor.humi=0.0;
    board_sensor.hartbeat=0;
    board_sensor.sop2=0;
    while (1) {
        if(state==4)
        {
            GUI_ClearSCR();
            GUI_LoadPic(0, 5, piaodai, 128, 53);
            GUI_LoadPic(1, 1, ant, 14, 9);
            GUI_LoadPic(115, 1, batt, 14, 9);
            GUI_LoadPic(32, 4, xinlvmenu, 64, 56);
            GUI_Exec();
            test();
            GUI_ClearSCR();
        }
        else  if(state==1)
        {
            //   GUI_ClearSCR();
            // t3aix_data();
            GUI_LoadPic(0, 5, piaodai, 128, 53);
            GUI_LoadPic(1, 1, ant, 14, 9);
            GUI_LoadPic(115, 1, batt, 14, 9);
            GUI_LoadPic(32, 4, jibumenu, 64, 56);
            GUI_Exec();
            vTaskDelayUntil(&xLastWakeTime, (configTICK_RATE_HZ/50));
        }
        else  if(state==0)
        {
            // t3aix_data();
            GUI_ClearSCR();
            GUI_LoadPic(1, 1, ant, 14, 9);
            GUI_LoadPic(115, 1, batt, 14, 9);
            GUI_Circle(64, 31, 30, disp_color);
            GUI_Circle(64, 31, 31, disp_color);
            draw_dgree();
            while(state==0)
            {
                if(timesum!=gtime.tm_hour+gtime.tm_min+gtime.tm_sec)
                {
                    draw_arrow(oldh,oldm,olds,0);
                    timesum=gtime.tm_hour+gtime.tm_min+gtime.tm_sec;
                    oldh=gtime.tm_hour;
                    oldm=gtime.tm_min;
                    olds=gtime.tm_sec;
                    draw_arrow(gtime.tm_hour,gtime.tm_min,gtime.tm_sec,1);
                    sprintf(str,"%4d",gtime.tm_year);
                    GUI_PutString8_8(0,24,str);
                    sprintf(str,"%2d-%2d",gtime.tm_mon,gtime.tm_mday);
                    GUI_PutString8_8(0,48,str);
                    sprintf(str,"%2d",gtime.tm_hour);
                    GUI_PutString8_8(112,24,str);
                    sprintf(str,"%2d:%2d",gtime.tm_min,gtime.tm_sec);
                    GUI_PutString8_8(96,48,str);
                    GUI_Exec();
                }    //
                vTaskDelayUntil(&xLastWakeTime, (configTICK_RATE_HZ/25));
            }
            GUI_ClearSCR();
        }
        else  if(state==3)
        {
            // t3aix_data();
            //
            GUI_LoadPic(0, 5, piaodai, 128, 53);
            GUI_LoadPic(1, 1, ant, 14, 9);
            GUI_LoadPic(115, 1, batt, 14, 9);
            GUI_LoadPic(32, 4, shezhimenu, 64, 56);
            GUI_Exec();
        }
        else
        {
            // t3aix_data();
            //GUI_ClearSCR();
            GUI_LoadPic(0, 5, piaodai, 128, 53);
            GUI_LoadPic(1, 1, ant, 14, 9);
            GUI_LoadPic(115, 1, batt, 14, 9);
            GUI_LoadPic(32, 4, tianqmenu, 64, 56);
            GUI_Exec();
            vTaskDelayUntil(&xLastWakeTime, (configTICK_RATE_HZ*2));
            GUI_ClearSCR();
            while(state==2)
            {
                TH_display();
                vTaskDelayUntil(&xLastWakeTime, (configTICK_RATE_HZ));
                GUI_Exec();
            }
            GUI_ClearSCR();
        }
        vTaskDelayUntil(&xLastWakeTime, (configTICK_RATE_HZ/50));
    }
}

/* =| UART0_IRQHandler |======================================
 *
 * This function overrides the weakly-declared interrupt handler
 *  in system_max326xx.c and is needed for asynchronous UART
 *  calls to work properly
 *
 * ===========================================================
 */
void UART0_IRQHandler(void)
{
    UART_Handler(MXC_UART0);
}
/* =| UART0_IRQHandler |======================================
 *
 * This function overrides the weakly-declared interrupt handler
 *  in system_max326xx.c and is needed for asynchronous UART
 *  calls to work properly
 *
 * ===========================================================
 */
void UART1_IRQHandler(void)
{
    UART_Handler(MXC_UART1);
}



#ifdef configUSE_TICKLESS_IDLE
/* =| freertos_permit_lp1 |===============================
 *
 * Determine if any hardware activity should prevent
 *  low-power tickless operation.
 *
 * =======================================================
 */
int freertos_permit_lp1(void)
{
    static TickType_t last_uart_tick = 0;

    if (disable_lp1 == 1) {
        return E_BUSY;
    }

    /* Did the RX pin wake the processor? */
    if (LP_IsGPIOWakeUpSource(&console_uart_rx)) {
        last_uart_tick = xTaskGetTickCount();
        LP_ClearWakeUpFlags();
    }

    /* Delay entry into LP1 until the tick that RX woke us is not current tick */
    if (last_uart_tick == xTaskGetTickCount()) {
        return E_BUSY;
    }

    return Console_PrepForSleep();
}
#endif
void BSP_Init(void)
{
    GUI_Initialize();
    GUI_SetColor(1,0);//此时是正常显示，若(0,1)则反色显示
}

/* =| main |==============================================
 *
 * This program demonstrates FreeRTOS tasks, mutexes,
 *  and the FreeRTOS+CLI extension.
 *
 * =======================================================
 */
int main(void)
{
#if (configUSE_TICKLESS_IDLE != 0)
    rtc_cfg_t rtc_cfg = {RTC_PRESCALE_DIV_2_0, RTC_PRESCALE_DIV_2_0, {0, 0}, 0, RTC_SNOOZE_MODE_B};
    uart_cfg_t uart_cfg;

    /* RTC interrupt synchronization must be enabled for interrupts to work */
    MXC_CLKMAN->sys_clk_ctrl_1_sync = 1;

    /* If running tickless idle, must reduce baud rate to avoid loosing character */
    memcpy(&uart_cfg, &console_uart_cfg, sizeof(uart_cfg));
    uart_cfg.baud = 115200;
    if (UART_Init(MXC_UART_GET_UART(CONSOLE_UART), &uart_cfg, &console_sys_cfg) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
    }
    /* Clear all previous wake-up configuration */
    LP_ClearWakeUpConfig();
    /* Reconfigure for only RTC COMP1 */
    LP_ConfigRTCWakeUp(0, 1, 0, 0);

    /* The RTC must be enabled for tickless operation */
    RTC_Init(&rtc_cfg);
    RTC_SetCount(0);
    RTC_Start();
#endif
    /* Print banner (RTOS scheduler not running) */
    printf("\n-=- %s FreeRTOS (%s) Demo -=-\n", STRING(TARGET), tskKERNEL_VERSION_NUMBER);
#if (configUSE_TICKLESS_IDLE != 0)
    printf("Tickless LP1 idle is configured. Type 'tickless 1' to enable.\n");
#endif
    BSP_Init();
    //OLED_DrawPoint(0,0,1);
    /* Configure task */
    if ((xTaskCreate(vTask0, (const char *)"Task0",
                     25*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) ||
            (xTaskCreate(vTask1, (const char *)"Task1",
                         20*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL) != pdPASS) ||
            (xTaskCreate(vTickTockTask, (const char *)"TickTock",
                         20*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) ||
	(xTaskCreate(vCmdLineTask, (const char *)"CmdLineTask",
		     20*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS)
       )
    {
        printf("xTaskCreate() failed to create a task.\n");
    } else {
        /* Start scheduler */
        printf("Starting scheduler.\n");
        vTaskStartScheduler();
    }
    /* This code is only reached if the scheduler failed to start */
    printf("ERROR: FreeRTOS did not start due to above error!\n");
    while (1) {
        __NOP();
    }
    /* Quiet GCC warnings */
    return -1;
}
