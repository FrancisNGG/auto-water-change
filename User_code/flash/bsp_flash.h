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
	uint8_t isExist;//������bool����,�п���flashΪ0xff,ҲΪture,��ʵ���ǿյ�
	//uint16_t water_out_time;//��ˮʱ������λs
	//uint16_t water_in_time;//��ˮʱ������λs	
	//uint16_t water_error_time;//�������ʱ������λs	
	uint8_t sAlarm_hr;//��ʱִ�е�ʱ��---Сʱ
	uint8_t sAlarm_min;//��ʱִ�е�ʱ��---����
	uint8_t isAlarmOpen;//������������ 1=�� 0=�� �رպ󣬾���ˮλ��ʱ���Ქ������
	//uint8_t no_sensor;//���Դ���������
	uint8_t auto_run_flag;//��ʱģʽ״̬
}__packed SettingREG_t;

extern SettingREG_t* pSettingReg;
extern SettingREG_t SettingRegBuff;
extern void setting_init();


#endif // !__BSP_FLASH_H__
