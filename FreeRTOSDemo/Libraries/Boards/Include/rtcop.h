#ifndef _RTC_OP_H_
#define _RTC_OP_H_
#include <time.h>
int bsp_RTC_init(void);
struct tm Time_ConvUnixToCalendar(time_t t);
time_t Time_ConvCalendarToUnix(struct tm t);
time_t Time_GetUnixTime(void);
struct tm Time_GetCalendarTime(void);
void Time_SetUnixTime(time_t t);
void Time_SetCalendarTime(struct tm t);

#endif
