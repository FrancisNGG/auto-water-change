#include "bsp_rtc.h"
#include "bsp_hmi.h"
#include "bsp_flash.h"
#include "stm32f1xx_hal.h"

RTC_TimeTypeDef RTC_Time;
RTC_DateTypeDef RTC_Date;

extern RTC_HandleTypeDef hrtc;
extern HAL_StatusTypeDef  RTC_WriteTimeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter);
extern uint32_t           RTC_ReadTimeCounter(RTC_HandleTypeDef* hrtc);

//ƽ����·����ڱ�
const uint8_t month_table[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/*
************************************************************
*	�������ƣ�	uint8_t Is_Leap_Year(uint16_t year)
*
*	�������ܣ�	�ж��Ƿ�������
*
*	��ڲ�����	��
*
*	���ز�����	������ǲ������ꡣ1��ʾ�ǣ�0��ʾ����
*
*	˵�����·�   1  2  3  4  5  6  7  8  9  10 11 12
*				����   31 29 31 30 31 30 31 31 30 31 30 31
*				������ 31 28 31 30 31 30 31 31 30 31 30 31
************************************************************
*/
uint8_t Is_Leap_Year(uint8_t Year)
{
	//�趨Ϊ2000��1��1�տ�ʼ
	//STM32��Year��С��0�����Ϊ99
	if (Year % 4 == 0) //�����ܱ�4����
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
*	�������ƣ�	uint32_t Seconds2Date(uint32_t Seconds)
*
*	�������ܣ�	����ת������
*
*	��ڲ�����	��
*
*	���ز�����	Date
*
*	˵����
************************************************************
*/
static void Seconds2Date(RTC_DateTypeDef* pDate, uint32_t Seconds)
{
	//���� 2000��1��1�� ��Ϊʱ����㿪ʼ����
	//STM32��Year��С��0�����Ϊ99
	uint8_t Year = 0;
	uint8_t Month = 0;
	uint32_t Date = Seconds / 86400;

	while (Date >= 365)
	{
		if (Is_Leap_Year(Year))//������
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
		if (Is_Leap_Year(pDate->Year) && Month == 1)//����2��
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
*	�������ƣ�	void GetTime(void)
*
*	�������ܣ�	��ȡRTCʱ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		��ȡʱ�����ת�������ڸ�ʽ������ RTC_Time RTC_Date
************************************************************
*/
void GetTime(RTC_TimeTypeDef* gTime, RTC_DateTypeDef* gDate)
{
	/* HAL����ʹ���� HAL_RTC_GetTime ������ CNTH �����Բ���ʹ��HAL�� */

	/* ��ȡ��ʱ���е������� */
	uint32_t timecounter = RTC_ReadTimeCounter(&hrtc);
	RTC_Time.Hours = timecounter / 3600 % 24;
	RTC_Time.Minutes = timecounter / 60 % 60;
	RTC_Time.Seconds = timecounter % 60;

	Seconds2Date(gDate, timecounter);
}

/*
************************************************************
*	�������ƣ�	uint32_t Date2Seconds(RTC_DateTypeDef sDate)
*
*	�������ܣ�	��һ�������ջ�ȡ������
*
*	��ڲ�����	��
*
*	���ز�����	������
*
*	˵����
************************************************************
*/
static uint32_t Date2Seconds(RTC_DateTypeDef sDate)
{
	//���� 2000��1��1�� ��Ϊʱ����㿪ʼ����
	//STM32��Year��С��0�����Ϊ99

	uint32_t TimeCounter = 0;

	for (uint8_t i = 0; i < sDate.Year; i++) //��������������
	{
		if (Is_Leap_Year(i))
		{
			TimeCounter += 31622400; //��������
		}
		else
		{
			TimeCounter += 31536000; //ƽ������
		}
	}

	for (uint8_t i = 0; i < (sDate.Month - 1); i++) //�����·��������
	{
		TimeCounter += month_table[i] * 86400;//һ��86400��
		if (Is_Leap_Year(sDate.Year) && i == 1)
		{
			TimeCounter += 86400;//���¶��һ��
		}
	}

	TimeCounter += (sDate.Date - 1) * 86400;

	return TimeCounter;
}

/*
************************************************************
*	�������ƣ�	void SetTime(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate)
*
*	�������ܣ�	����ʱ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		��һ��ʱ������ת����ʱ�����д��RTC->CNT
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
*	�������ƣ�	uint8_t GetWeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay)
*
*	�������ܣ�	��ȡ����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		ʹ�û�ķ����ɭ���㹫ʽ,�������ڼ����ܼ�
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
*	�������ƣ�	void AlarmCheck(void)
*
*	�������ܣ�	����Ƿ񵽴�����ʱ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		����ʱ�����ʽ��HAL�Դ� ALARM ������
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