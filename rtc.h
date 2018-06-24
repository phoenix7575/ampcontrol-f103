#ifndef RTC_H
#define RTC_H

#include <inttypes.h>

typedef enum {
    RTC_HOUR,
    RTC_MIN,
    RTC_SEC,
    RTC_DATE,
    RTC_MONTH,
    RTC_YEAR,

    RTC_NOEDIT,
} RtcMode;

typedef struct {
    int8_t hour;
    int8_t min;
    int8_t sec;
    int8_t date;
    int8_t month;
    int8_t year;
    int8_t wday;
    int8_t etm;
} RTC_type;

void rtcInit();

uint32_t rtcToSec(RTC_type *rtc);
void secToRtc(uint32_t time, RTC_type *rtc);

void rtcGetTime(RTC_type *rtc);

void rtcReadTime(void);
void rtcWriteTime(uint32_t time);

#endif // RTC_H
