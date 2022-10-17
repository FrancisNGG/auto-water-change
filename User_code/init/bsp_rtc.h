#pragma once
#ifndef __BSP_RTC_H__
#define __BSP_RTC_H__

#include "main.h"

extern void SetTime(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate);
extern void GetTime(RTC_TimeTypeDef* gTime, RTC_DateTypeDef* gDate);
extern uint8_t GetWeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay);
extern void AlarmCheck(void);

extern RTC_TimeTypeDef RTC_Time;
extern RTC_DateTypeDef RTC_Date;

#endif // !__BSP_RTC_H__
