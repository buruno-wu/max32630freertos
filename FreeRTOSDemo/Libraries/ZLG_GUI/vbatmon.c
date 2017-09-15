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
 * $Date: 2016-03-11 11:46:02 -0600 (Fri, 11 Mar 2016) $
 * $Revision: 21838 $
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   ADC demo application
 * @details Continuously monitors the ADC channels
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "led.h"
#include "adc.h"

/***** Definitions *****/

/* Change to #undef USE_INTERRUPTS for polling mode */
//#define USE_INTERRUPTS 0

/***** Globals *****/
#ifdef USE_INTERRUPTS
volatile unsigned int adc_done = 0;
#endif

/***** Functions *****/

#ifdef USE_INTERRUPTS
void AFE_IRQHandler(void)
{
    ADC_ClearFlags(MXC_F_ADC_INTR_ADC_DONE_IF);
    
    /* Signal bottom half that data is ready */
    adc_done = 1;
    
    return;
}
#endif
void vbatain_init(void)
{ 
  /* Initialize ADC */
    ADC_Init();
    /* Set up LIMIT0 to monitor high and low trip points */
    ADC_SetLimit(ADC_LIMIT_0, ADC_CH_0, 1, 0x25, 1, 0x300);    
#ifdef USE_INTERRUPTS
    NVIC_EnableIRQ(AFE_IRQn);
#endif

}

int vbat_get(void)
{
    uint16_t adc_val;
    unsigned int overflow;
    uint8_t fmtstr[40];    
    /* Convert channel 0 */
#ifdef USE_INTERRUPTS
	adc_done = 0;
	ADC_StartConvert(ADC_CH_0, 0, 1);
	while (!adc_done);
#else
	ADC_StartConvert(ADC_CH_0, 0, 1);
#endif
	overflow = (ADC_GetData(&adc_val) == E_OVERFLOW ? 1 : 0);
	/* Display results on OLED display, display asterisk if overflow */
	printf("0: 0x%04x",adc_val);
        return adc_val;
}
