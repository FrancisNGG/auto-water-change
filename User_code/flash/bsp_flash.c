#include "bsp_flash.h"
#include "bsp_hmi.h"

SettingREG_t* pSettingReg = (SettingREG_t*)SETTING_REG_ADDR;
SettingREG_t SettingRegBuff;

//初始化flash保存的参数
void setting_init()
{
	SettingRegBuff.isExist = 1;
	//SettingRegBuff.water_in_time = 10;//300s
	//SettingRegBuff.water_out_time = 10;//300s
	//SettingRegBuff.water_error_time = 10;//300s
	SettingRegBuff.sAlarm_hr = 8;
	SettingRegBuff.sAlarm_min = 28;
	SettingRegBuff.isAlarmOpen = 1;
	//SettingRegBuff.no_sensor = 1;
	SettingRegBuff.auto_run_flag = 1;
}

void SaveSettingToFlash()
{
	static FLASH_EraseInitTypeDef rease_addr;
	uint32_t pageerror = 0;  //¶¨Òå²Á³ý³ö´íÊ±·µ»ØµÄflashµØÖ·
	uint16_t* pDATA = (uint16_t*)&SettingRegBuff;
	uint8_t page = sizeof(SettingRegBuff) / sizeof(uint16_t) + 1;
	HAL_FLASH_Unlock();
	rease_addr.TypeErase = FLASH_TYPEERASE_PAGES;
	rease_addr.PageAddress = SETTING_REG_ADDR;  //¶¨ÒåÊý¾ÝµØÖ·
	rease_addr.NbPages = 1;  //¶¨Òå²Á³ýÒ³ÃæÊý Ò»Ò³=1024byte
	HAL_FLASHEx_Erase(&rease_addr, &pageerror);  //²Á³ýflash
	for (uint8_t i = 0; i < page; i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, SETTING_REG_ADDR + i * 2, *(pDATA + i));
	}
	HAL_FLASH_Lock();
}

bool SetFlashReadProtection(bool state)
{
	FLASH_OBProgramInitTypeDef OptionsBytesStruct = { 0 };
	HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

	if (state == 1)
	{
		if (OptionsBytesStruct.RDPLevel == OB_RDP_LEVEL_0)
		{
			OptionsBytesStruct.OptionType = OPTIONBYTE_RDP;
			OptionsBytesStruct.RDPLevel = OB_RDP_LEVEL_1;

			HAL_FLASH_Unlock();
			HAL_FLASH_OB_Unlock();

			if (HAL_FLASHEx_OBProgram(&OptionsBytesStruct) != HAL_OK)
			{
				HAL_FLASH_OB_Lock();
				HAL_FLASH_Lock();

				return 0;
			}

			HAL_FLASH_OB_Lock();
			HAL_FLASH_Lock();
		}
	}
	else
	{
		if (OptionsBytesStruct.RDPLevel == OB_RDP_LEVEL_1)
		{
			OptionsBytesStruct.OptionType = OPTIONBYTE_RDP;
			OptionsBytesStruct.RDPLevel = OB_RDP_LEVEL_0;

			HAL_FLASH_Unlock();
			HAL_FLASH_OB_Unlock();

			if (HAL_FLASHEx_OBProgram(&OptionsBytesStruct) != HAL_OK)
			{
				HAL_FLASH_OB_Lock();
				HAL_FLASH_Lock();

				return 0;
			}

			HAL_FLASH_OB_Lock();
			HAL_FLASH_Lock();
		}
	}
	return 1;
}