#pragma once
#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__
#include "main.h"
#include "stdbool.h"

#define SETTING_REG_ADDR	((uint32_t)0x0800F000)

void SaveSettingToFlash(void);
bool SetFlashReadProtection(bool state);

typedef struct SettingREG_s
{
	uint8_t isExist;//不能用bool定义,有可能flash为0xff,也为ture,但实际是空的
	//uint16_t water_out_time;//放水时长，单位s
	//uint16_t water_in_time;//进水时长，单位s	
	//uint16_t water_error_time;//警报检查时长，单位s	
	uint8_t sAlarm_hr;//定时执行的时间---小时
	uint8_t sAlarm_min;//定时执行的时间---分钟
	uint8_t isAlarmOpen;//声音警报开关 1=开 0=关 关闭后，警戒水位超时不会播放音乐
	//uint8_t no_sensor;//忽略传感器开关
	uint8_t auto_run_flag;//定时模式状态
}__packed SettingREG_t;

extern SettingREG_t* pSettingReg;
extern SettingREG_t SettingRegBuff;
extern void setting_init();


#endif // !__BSP_FLASH_H__
