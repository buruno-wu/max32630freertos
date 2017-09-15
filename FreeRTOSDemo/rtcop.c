#include "rtc.h"
#include <time.h>
/***** Definitions *****/
#define LED_Alarm0		  0
#define LED_Alarm1		  1
#define LED_tick		  2

#define ALARM0_SEC        3
#define ALARM1_SEC        5
#define SNOOZE_SEC        7

int bsp_RTC_init(void)
{
    rtc_cfg_t RTCconfig;
    //set RTC configuration
	  RTCconfig.compareCount[0] = ALARM0_SEC; //alarm0 time in seconds
    RTCconfig.compareCount[1] = ALARM1_SEC; //alarm1 time in seconds
    RTCconfig.prescaler = RTC_PRESCALE_DIV_2_12; //1Hz clock
    RTCconfig.prescalerMask = RTC_PRESCALE_DIV_2_11;//0.5s prescaler compare
    RTCconfig.snoozeCount = SNOOZE_SEC;//snooze time in seconds
    RTCconfig.snoozeMode = RTC_SNOOZE_DISABLE;
    RTC_Init(&RTCconfig);
    //start RTC
    RTC_Start();
}
/*******************************************************************************
* Function Name  : Time_ConvUnixToCalendar(time_t t)
* Description    : ??UNIX????????
* Input    : u32 t  ?????UNIX???
* Output   : None
* Return   : struct tm
*******************************************************************************/
struct tm Time_ConvUnixToCalendar(time_t t)
{
 struct tm *t_tm;
 t_tm = localtime(&t);
 t_tm->tm_year += 1900; //localtime?????tm_year????,???????
 return *t_tm;
}
/*******************************************************************************
* Function Name  : Time_ConvCalendarToUnix(struct tm t)
* Description    : ??RTC??????
* Input    : struct tm t
* Output   : None
* Return   : time_t
*******************************************************************************/
time_t Time_ConvCalendarToUnix(struct tm t)
{
 t.tm_year -= 1900;  //??tm?????????2008??
      //?time.h?????????1900??????
      //??,???????????????
 return mktime(&t);
}

/*******************************************************************************
* Function Name  : Time_GetUnixTime()
* Description    : ?RTC??????Unix????
* Input    : None
* Output   : None
* Return   : time_t t
*******************************************************************************/
time_t Time_GetUnixTime(void)
{
 return (time_t)RTC_GetCount();
}

/*******************************************************************************
* Function Name  : Time_GetCalendarTime()
* Description    : ?RTC??????????(struct tm)
* Input    : None
* Output   : None
* Return   : time_t t
*******************************************************************************/
struct tm Time_GetCalendarTime(void)
{
 time_t t_t;
 struct tm t_tm;

 t_t = (time_t)RTC_GetCount();
 t_tm = Time_ConvUnixToCalendar(t_t);
 return t_tm;
}

/*******************************************************************************
* Function Name  : Time_SetUnixTime()
* Description    : ????Unix?????RTC
* Input    : time_t t
* Output   : None
* Return   : None
*******************************************************************************/
void Time_SetUnixTime(time_t t)
{

 RTC_SetCount((uint32_t)t);
 return;
}
/*******************************************************************************
* Function Name  : Time_SetCalendarTime()
* Description    : ????Calendar???????UNIX?????RTC
* Input    : struct tm t
* Output   : None
* Return   : None
*******************************************************************************/
void Time_SetCalendarTime(struct tm t)
{
 Time_SetUnixTime(Time_ConvCalendarToUnix(t));
 return;
}

