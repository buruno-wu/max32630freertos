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
 *
 *******************************************************************************
 */

/**
 * @file    main.c
 * @brief   Main for UART example.
 * @details This example loops back the TX to the RX on UART1. For this example
 *          you must connect a jumper across P2.0 to P2.1 and connect P2.2 and P2.3. UART_BAUD and the BUFF_SIZE
 *          can be changed in this example.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "clkman.h"
#include "ioman.h"
#include "uart.h"
#include "board.h"
#include "tmr_utils.h"
#include "lp.h"
#include "tmr.h"
#include "FreeRTOS.h"
#include "jansson.h"
/***** Definitions *****/
#define UART_BAUD           115200
#define BUFF_SIZE           512
#define STARTUP_DELAY           (USEC(20))
#define SLEEP_DELAY             (MSEC(100000))

/***** Globals *****/

/***** Functions *****/
volatile int read_error;
volatile int tmr_flag;

void add_2array_to_json( json_t* obj, const char* name, const int*
                         marr, size_t dim1, size_t dim2 )
{
    size_t i, j;
    json_t* jarr1 = json_array();

    for( i=0; i<dim1; ++i ) {
        json_t* jarr2 = json_array();

        for( j=0; j<dim2; ++j ) {
            int val = marr[ i*dim2 + j ];
            json_t* jval = json_integer( val );
            json_array_append_new( jarr2, jval );
        }
        json_array_append_new( jarr1, jarr2 );
    }
    json_object_set_new( obj, name, jarr1 );
    return;
}

typedef struct datapoint {
    char *unit;
    char *name;
    char *time;
    char *id;
    int value;
} led;

#define URL "POST /devices/13532667/datapoints?type=3 HTTP/1.1\n"
#define APIKEY "api-key:xanXzCHg7av98MUmxRHeUhVjKd8=\n"
#define HOST "Host:api.heclouds.com\n"
#define CONTLENTH "Content-Length:"
#define URL_GET "/devices/13532667/datastreams/num HTTP/1.1\n"

void add_value_to_json( json_t* obj, const char* name,float *marr)
{
    double temp;
    temp=*marr;
    json_t* jarr1 = json_real(temp);
    json_object_set_new( obj, name, jarr1 );
}
void add_int_value_to_json( json_t* obj, const char* name,void *marr)
{
    json_t* jarr1 = json_integer(*(int*)marr);
    json_object_set_new( obj, name, jarr1 );
}
int sw_value;
led taideng;

/******************************************************************************/
void TMR1_IRQHandler(void)
{
    tmr_flag = 0;
    TMR32_ClearFlag(MXC_TMR1);
}
tmr32_cfg_t tmr_cfg;
void tmer_init(int utime)
{
    uint32_t ticks;
    NVIC_EnableIRQ(MXC_TMR_GET_IRQ_32(1));
    TMR_Init(MXC_TMR1, TMR_PRESCALE_DIV_2_2, NULL);
    TMR32_EnableINT(MXC_TMR1);
    TMR32_TimeToTicks(MXC_TMR1, utime, TMR_UNIT_MILLISEC, &ticks);
    tmr_cfg.mode = TMR32_MODE_ONE_SHOT;
    tmr_cfg.compareCount = ticks;
    tmr_flag = 1;
    TMR32_Config(MXC_TMR1, &tmr_cfg);
    TMR32_Start(MXC_TMR1);
    //	TMR32_Config(MXC_TMR1, &tmr_cfg);
}
/***** Globals *****/
volatile int read_flag;
volatile int write_flag;

/***** Functions *****/

/******************************************************************************/
void read_cb(uart_req_t* req, int error)
{
    read_flag = error;
}

/******************************************************************************/
void write_cb(uart_req_t* req, int error)
{
    write_flag = error;
}

/******************************************************************************/
void UART2_IRQHandler(void)
{
    UART_Handler(MXC_UART_GET_UART(ESP8266_UART));
}
int recv_fram(unsigned char *buf)
{
    int error,num,i;
    num=0;
    i=0;
    tmr_flag = 1;
    tmer_init(800);
    while(tmr_flag==1)
    {
        //if(!UART_NumReadAvail(MXC_UART_GET_UART(ESP8266_UART)))break;
        num=UART_NumReadAvail(MXC_UART_GET_UART(ESP8266_UART));
        if(num>0)	{
            error = UART_Read(MXC_UART_GET_UART(ESP8266_UART), &buf[i],num, NULL);
            if(error<E_NO_ERROR) {
                printf("Error starting async read %d\n", error);
                return 	error;
            }
            else i+=error;
        }
    }
    printf("rxdata %s\n",buf);
    return i;
}
int write_ESP(char *str)
{
    int error,len;
    uint8_t txdata[BUFF_SIZE];
// Setup the asynchronous requests
    uart_req_t write_req;
    write_req.data = txdata;
    write_req.len = BUFF_SIZE;
    write_req.callback = write_cb;
    read_flag = 1;
    write_flag = 1;
    memset(txdata,0,sizeof(txdata));
    len=write_req.len = strlen(str);
    memcpy(txdata,str,len);
    error = UART_WriteAsync(MXC_UART_GET_UART(ESP8266_UART), &write_req);
    if(error != E_NO_ERROR) {
        printf("Error starting async write %d\n", error);
        return error;
    }
    return len;
}
int write_ESP_data(char *str,int num)
{
    int error,len;
    uint8_t txdata[BUFF_SIZE];
// Setup the asynchronous requests
    uart_req_t write_req;
    write_req.data = txdata;
    write_req.len = BUFF_SIZE;
    write_req.callback = write_cb;
    read_flag = 1;
    write_flag = 1;
    memset(txdata,0,sizeof(txdata));
    len=write_req.len =num;
    memcpy(txdata,str,len);
    error = UART_WriteAsync(MXC_UART_GET_UART(ESP8266_UART), &write_req);
    if(error != E_NO_ERROR) {
        printf("Error starting async write %d\n", error);
        return error;
    }
    return len;
}
int utest(char *buf)
{
    int len;
    json_t* jdata;
    char* s;
    double val;
    jdata = json_object();
    add_value_to_json(jdata,"temp",&board_sensor.temp);
    add_value_to_json(jdata,"humi",&board_sensor.humi);
    add_int_value_to_json(jdata,"hartbeat",&board_sensor.hartbeat);
    add_int_value_to_json(jdata,"SPO2",&board_sensor.sop2);
    s = json_dumps( jdata, JSON_PRESERVE_ORDER );
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s%s%s%s%d\n\n%s\n",URL,APIKEY,HOST,CONTLENTH,strlen(s),s);
    //memset(buf,0,sizeof(buf));
    // sprintf(buf,"GET %sapi-key:%sHost:%s\n",URL_GET,APIKEY,HOST);
    jsonp_free( s );
    json_decref( jdata );

}
/******************************************************************************/
int ESP_UART_init(void)
{
    int error, i,len;
    uint32_t ticks;
    TickType_t xLastWakeTime;
    uint8_t txdata[BUFF_SIZE];
    uint8_t rxdata[BUFF_SIZE];
    printf("\n\n***** UART Example *****\n");
    printf(" System freq \t: %d Hz\n", SystemCoreClock);
    printf(" UART freq \t: %d Hz\n", ESP8266_BAUD);
    printf(" Loop back \t: %d bytes\n\n", BUFF_SIZE);
    memset(rxdata, 0x0, BUFF_SIZE);
    xLastWakeTime = xTaskGetTickCount();
    // Initialize the data buffers
    // Setup the interrupt
    NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(ESP8266_UART));
    NVIC_DisableIRQ(MXC_UART_GET_IRQ(ESP8266_UART));
    NVIC_SetPriority(MXC_UART_GET_IRQ(ESP8266_UART), 1);
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(ESP8266_UART));
    // Initialize the UART
    error =UART_Init(MXC_UART_GET_UART(ESP8266_UART), &esp8266_uart_cfg, &esp8266_sys_cfg);
    if(error != E_NO_ERROR) {
        printf("Error initializing UART %d\n", error);
        while(1) {}
    } else {
        printf("UART Initialized\n");
    }
    if(0)
    {
        write_ESP("+++");
        //  len=recv_fram(&rxdata[0]);
        //  memset(rxdata, 0x0, BUFF_SIZE);
        //  write_ESP("AT+RST\r\n");
        // len=recv_fram(&rxdata[0]);
        //  vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ*3);
        memset(rxdata, 0x0, BUFF_SIZE);
        write_ESP("AT+GMR\r\n");
        len=recv_fram(&rxdata[0]);
        if(strstr(rxdata,"OK")) {
            printf("version get ok!\r\n");
        }
        memset(rxdata, 0x0, BUFF_SIZE);
        write_ESP("AT+CWMODE_DEF=1\r\n");
        recv_fram(&rxdata[0]);
        memset(rxdata, 0x0, BUFF_SIZE);
        write_ESP("AT+CWJAP_DEF=\"software\",\"software123\"\r\n");
        //len=recv_fram(&rxdata[0]);
        while(!strstr(rxdata,"OK"))
        {
            memset(rxdata, 0x0, BUFF_SIZE);
            recv_fram(&rxdata[0]);
            if(strstr(rxdata,"FAIL"))return -1 ;
        }
        if(strstr(rxdata,"OK")) {
            printf("wifi connected!\r\n");
            memset(rxdata, 0x0, BUFF_SIZE);
            vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ);
            write_ESP("AT+CWAUTOCONN=1\r\n");
            recv_fram(&rxdata[0]);
            memset(rxdata, 0x0, BUFF_SIZE);
            write_ESP("AT+CIPSTART=\"TCP\",\"183.230.40.34\",80\r\n");
            recv_fram(&rxdata[0]);
            vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/2);
            memset(rxdata, 0x0, BUFF_SIZE);
            write_ESP("AT+CIPMODE=1\r\n");
            recv_fram(&rxdata[0]);
            vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/2);
            write_ESP("AT+CIPSEND\r\n");
            recv_fram(&rxdata[0]);
            vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/2);
        }
    }

    /*	    write_ESP("AT+GMR\r\n");
        len=recv_fram(&rxdata[0]);
        if(strstr(rxdata,"OK")) {
            printf("version get ok!\r\n");
        }
        memset(rxdata, 0x0, BUFF_SIZE);*/
    write_ESP("AT+RST\r\n");
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ*5);
    //  write_ESP("AT+CIPSTART=\"TCP\",\"183.230.40.34\",80\r\n");
    // recv_fram(&rxdata[0]);
    memset(rxdata, 0x0, BUFF_SIZE);
    write_ESP("AT+CIPMODE=1\r\n");
    recv_fram(&rxdata[0]);
    write_ESP("AT+CIPSEND\r\n");
    recv_fram(&rxdata[0]);
    while(1)
    {
        memset(rxdata, 0x0, BUFF_SIZE);
        utest(txdata);
        write_ESP(txdata);
        recv_fram(&rxdata[0]);
        printf("%s",txdata);
        vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ*10);
    }

}
