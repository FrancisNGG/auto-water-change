#include "bsp_rtc.h"
#include "bsp_hmi.h"
#include "bsp_flash.h"
#include "stm32f1xx_hal.h"

RTC_TimeTypeDef RTC_Time;
RTC_DateTypeDef RTC_Date;

extern RTC_HandleTypeDef hrtc;
extern HAL_StatusTypeDef  RTC_WriteTimeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter);
extern uint32_t           RTC_ReadTimeCounter(RTC_HandleTypeDef* hrtc);

//平年的月份日期表
const uint8_t month_table[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/*
************************************************************
*	函数名称：	uint8_t Is_Leap_Year(uint16_t year)
*
*	函数功能：	判断是否是闰年
*
*	入口参数：	年
*
*	返回参数：	该年份是不是闰年。1表示是，0表示不是
*
*	说明：月份   1  2  3  4  5  6  7  8  9  10 11 12
*				闰年   31 29 31 30 31 30 31 31 30 31 30 31
*				非闰年 31 28 31 30 31 30 31 31 30 31 30 31
************************************************************
*/
uint8_t Is_Leap_Year(uint8_t Year)
{
	//设定为2000年1月1日开始
	//STM32的Year最小是0，最大为99
	if (Year % 4 == 0) //必须能被4整除
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
************************************************************
*	函数名称：	uint32_t Seconds2Date(uint32_t Seconds)
*
*	函数功能：	秒数转年月日
*
*	入口参数：	无
*
*	返回参数：	Date
*
*	说明：
************************************************************
*/
static void Seconds2Date(RTC_DateTypeDef* pDate, uint32_t Seconds)
{
	//基于 2000年1月1日 作为时间基点开始计算
	//STM32的Year最小是0，最大为99
	uint8_t Year = 0;
	uint8_t Month = 0;
	uint32_t Date = Seconds / 86400;

	while (Date >= 365)
	{
		if (Is_Leap_Year(Year))//是闰年
		{
			if (Date >= 366)
				Date -= 366;
			else break;
		}
		else
		{
			Date -= 365;
		}
		Year++;
	}

	while (Date >= 28)
	{
		if (Is_Leap_Year(pDate->Year) && Month == 1)//闰年2月
		{
			if (Date >= 29)
				Date -= 29;
			else
				break;
		}
		else
		{
			if (Date >= month_table[Month])
				Date -= month_table[Month];
			else
				break;
		}
		Month++;
	}

	pDate->Year = Year;
	pDate->Month = Month + 1;
	pDate->Date = Date + 1;
}


/*
************************************************************
*	函数名称：	void GetTime(void)
*
*	函数功能：	获取RTC时钟
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		获取时间戳并转换成日期格式保存在 RTC_Time RTC_Date
************************************************************
*/
void GetTime(RTC_TimeTypeDef* gTime, RTC_DateTypeDef* gDate)
{
	/* HAL库中使用了 HAL_RTC_GetTime 后会清空 CNTH ，所以不再使用HAL库 */

	/* 获取计时器中的总秒数 */
	uint32_t timecounter = RTC_ReadTimeCounter(&hrtc);
	RTC_Time.Hours = timecounter / 3600 % 24;
	RTC_Time.Minutes = timecounter / 60 % 60;
	RTC_Time.Seconds = timecounter % 60;

	Seconds2Date(gDate, timecounter);
}

/*
************************************************************
*	函数名称：	uint32_t Date2Seconds(RTC_DateTypeDef sDate)
*
*	函数功能：	给一个年月日获取总秒数
*
*	入口参数：	无
*
*	返回参数：	总秒数
*
*	说明：
************************************************************
*/
static uint32_t Date2Seconds(RTC_DateTypeDef sDate)
{
	//基于 2000年1月1日 作为时间基点开始计算
	//STM32的Year最小是0，最大为99

	uint32_t TimeCounter = 0;

	for (uint8_t i = 0; i < sDate.Year; i++) //所有年份秒数相加
	{
		if (Is_Leap_Year(i))
		{
			TimeCounter += 31622400; //闰年秒数
		}
		else
		{
			TimeCounter += 31536000; //平年秒数
		}
	}

	for (uint8_t i = 0; i < (sDate.Month - 1); i++) //所有月份秒数相加
	{
		TimeCounter += month_table[i] * 86400;//一天86400秒
		if (Is_Leap_Year(sDate.Year) && i == 1)
		{
			TimeCounter += 86400;//闰月多加一天
		}
	}

	TimeCounter += (sDate.Date - 1) * 86400;

	return TimeCounter;
}

/*
************************************************************
*	函数名称：	void SetTime(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate)
*
*	函数功能：	设置时间
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		给一个时间日期转换成时间戳后写入RTC->CNT
************************************************************
*/
void SetTime(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate)
{
	uint32_t TimeCounter =
		sTime.Seconds +
		sTime.Minutes * 60 +
		sTime.Hours * 3600 +
		Date2Seconds(sDate);

	RTC_WriteTimeCounter(&hrtc, TimeCounter);
}

/*
************************************************************
*	函数名称：	uint8_t GetWeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay)
*
*	函数功能：	获取星期
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		使用基姆拉尔森计算公式,根据日期计算周几
************************************************************
*/
uint8_t GetWeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay)
{
	uint32_t year = 0U, weekday = 0U;

	year = 2000U + nYear;

	if (nMonth < 3U)
	{
		/*D = { [(23 x month)/9] + day + 4 + year + [(year-1)/4] - [(year-1)/100] + [(year-1)/400] } mod 7*/
		weekday = (((23U * nMonth) / 9U) + nDay + 4U + year + ((year - 1U) / 4U) - ((year - 1U) / 100U) + ((year - 1U) / 400U)) % 7U;
	}
	else
	{
		/*D = { [(23 x month)/9] + day + 4 + year + [year/4] - [year/100] + [year/400] - 2 } mod 7*/
		weekday = (((23U * nMonth) / 9U) + nDay + 4U + year + (year / 4U) - (year / 100U) + (year / 400U) - 2U) % 7U;
	}

	return (uint8_t)weekday;
}

/*
************************************************************
*	函数名称：	void AlarmCheck(void)
*
*	函数功能：	检查是否到达闹钟时间
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		采用时间戳格式后，HAL自带 ALARM 不可用
************************************************************
*/
void AlarmCheck(void)
{
	if (SettingRegBuff.sAlarm_hr == RTC_Time.Hours)
	{
		if (SettingRegBuff.sAlarm_min == RTC_Time.Minutes && RTC_Time.Seconds == 0)
		{
			HmiReg.is_alarm = 1;
		}
	}
}