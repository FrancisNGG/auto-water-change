
#include "bsp_HMI.h"
#include "user_task.h"
#include "string.h"
#include "stdlib.h"
#include "main.h"
#include "stdio.h"
#include "bsp_uart.h"
#include "bsp_flash.h"
#include "stdbool.h"
#include "cmsis_os.h"
#include "stdarg.h"
#include "bsp_rtc.h"

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim3;
extern osMessageQueueId_t LcdTxQueueHandle;

//HMI_TIME_t hmi_time;
HMI_REG_t HmiReg;

/*指令打包写入消息缓冲区*/
void hmi_cmd_transmit(char* cmd, ...)//变参函数，最多3个参数
{
	va_list valist;
	va_start(valist, cmd);
	char* p = cmd;

	uint8_t arg_num = 0;//参数数量
	char* str[3];//使用字符串数组来保存参数

	while (*p)
	{
		if (*p == '%' && *(p + 1) == 's')//使用%s来判断参数数量
		{
			str[arg_num] = va_arg(valist, char*);//返回字符串指针
			arg_num++;
		}
		p++;
	}

	HMI_TRANSMIT_MSG_t* hmi_transmit_buff = (HMI_TRANSMIT_MSG_t*)malloc(sizeof(HMI_TRANSMIT_MSG_t));//分配发送缓冲区内存
	//HMI_TRANSMIT_MSG_t* hmi_transmit_buff = (HMI_TRANSMIT_MSG_t*)pvPortMalloc(sizeof(HMI_TRANSMIT_MSG_t));//分配发送缓冲区内存
	memset(hmi_transmit_buff, 0x00, sizeof(HMI_TRANSMIT_MSG_t));//缓冲区初始化
	hmi_transmit_buff->type = EVENT_UART_TX;//消息类型
	switch (arg_num)
	{
	case 0:
		hmi_transmit_buff->size += sprintf((char*)&hmi_transmit_buff->buff[hmi_transmit_buff->size], cmd);
		break;
	case 1:
		hmi_transmit_buff->size += sprintf((char*)&hmi_transmit_buff->buff[hmi_transmit_buff->size], cmd, str[0]);
		break;
	case 2:
		hmi_transmit_buff->size += sprintf((char*)&hmi_transmit_buff->buff[hmi_transmit_buff->size], cmd, str[0], str[1]);
		break;
	case 3:
		hmi_transmit_buff->size += sprintf((char*)&hmi_transmit_buff->buff[hmi_transmit_buff->size], cmd, str[0], str[1], str[2]);
		break;
	default:
		break;
	}
	memset(hmi_transmit_buff->buff + hmi_transmit_buff->size, 0xff, 3);//hmi指令结束符
	hmi_transmit_buff->size += 3;
	osMessageQueuePut(LcdTxQueueHandle, hmi_transmit_buff, 0, 0);
	free(hmi_transmit_buff);//释放发送缓冲区内存地址
	//vPortFree(hmi_transmit_buff);

	va_end(valist);//释放可变参数列表地址
}

/* 刷新主屏幕时间 */
void reflush_hmi_time()
{
	char str[10];
	Int2String(RTC_Time.Hours, str);
	hmi_cmd_transmit("index.hr.val=%s", str);
	Int2String(RTC_Time.Minutes, str);
	hmi_cmd_transmit("index.min.val=%s", str);
	Int2String(RTC_Time.Seconds, str);
	hmi_cmd_transmit("index.sec.val=%s", str);
}

/* 刷新主屏幕日期 */
void reflush_hmi_date()
{
	char str[10];
	Int2String(RTC_Date.Year + 2000, str);
	hmi_cmd_transmit("index.y.val=%s", str);
	Int2String(RTC_Date.Month, str);
	hmi_cmd_transmit("index.m.val=%s", str);
	Int2String(RTC_Date.Date, str);
	hmi_cmd_transmit("index.d.val=%s", str);

	uint8_t WeekDay = GetWeekDayNum(RTC_Date.Year, RTC_Date.Month, RTC_Date.Date);
	switch (WeekDay)
	{
	case RTC_WEEKDAY_SUNDAY:
		hmi_cmd_transmit("index.t7.txt=\"周日\"");
		break;
	case RTC_WEEKDAY_MONDAY:
		hmi_cmd_transmit("index.t7.txt=\"周一\"");
		break;
	case RTC_WEEKDAY_TUESDAY:
		hmi_cmd_transmit("index.t7.txt=\"周二\"");
		break;
	case RTC_WEEKDAY_WEDNESDAY:
		hmi_cmd_transmit("index.t7.txt=\"周三\"");
		break;
	case RTC_WEEKDAY_THURSDAY:
		hmi_cmd_transmit("index.t7.txt=\"周四\"");
		break;
	case RTC_WEEKDAY_FRIDAY:
		hmi_cmd_transmit("index.t7.txt=\"周五\"");
		break;
	case RTC_WEEKDAY_SATURDAY:
		hmi_cmd_transmit("index.t7.txt=\"周六\"");
		break;
	default:
		break;
	}
}



/* 执行放水 */
void water_out_handle()
{
	hmi_cmd_transmit("index.t5.txt=\"正在执行放水...\"");
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//打开出水阀门
}
/* 执行进水 */
void water_in_handle()
{
	hmi_cmd_transmit("index.t5.txt=\"正在执行进水...\"");
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_RESET);//打开进水阀门
}
/* 停止 */
void water_stop_handle()
{
	hmi_cmd_transmit("vis b2,0");
	hmi_cmd_transmit("vis b1,1");
	hmi_cmd_transmit("index.t5.txt=\"停止操作！！\"");
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
}

/* 水满操作 */
void water_full_handle()
{
	if (HmiReg.now_page != INDEX)//如果当前页面不在主页，则返回主页再开始操作
	{
		hmi_cmd_transmit("page index");
	}
	hmi_cmd_transmit("vis bt0,0");
	hmi_cmd_transmit("vis b1,0");
	hmi_cmd_transmit("vis b2,0");
	hmi_cmd_transmit("index.t5.txt=\"到达警戒水位，将持续放水...\"");
	hmi_cmd_transmit("index.t5.pco=%s", RED);
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//打开出水阀门
}

void warning_clear_handle()
{
	hmi_cmd_transmit("vis bt0,1");
	hmi_cmd_transmit("vis b1,1");
	hmi_cmd_transmit("vis b2,1");
	hmi_cmd_transmit("index.t5.txt=\"水位恢复正常，持续检测中...\"");
	hmi_cmd_transmit("index.t5.pco=%s", GREEN);
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
}

/* 执行一次换水操作 */
void click_run()
{
	HmiReg.run_once = 1;
	//if (HmiReg.is_water_low)//已经到达低水位
	//{
	//	water_in_handle();
	//}
	//else
	//{
	water_out_handle();
	//}
}


void MenuUARTFuntion(uint8_t* dat)
{
	/* 打开主页 */
	if (memcmp(dat, "index", 5) == 0)
	{
		HmiReg.now_page = INDEX;
		char str[10];
		Int2String(SettingRegBuff.isAlarmOpen, str);
		hmi_cmd_transmit("setting.c0.val=%s", str);
		Int2String(SettingRegBuff.auto_run_flag, str);
		hmi_cmd_transmit("index.bt0.val=%s", str);
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_RESET);
	}

	/* 保存到flash */
	else if (memcmp(dat, "save", 4) == 0)
	{
		SaveSettingToFlash();
		//SetAlarm_IT(SettingRegBuff.sAlarm_hr, SettingRegBuff.sAlarm_min, 0);
	}

	/* 定时自动模式开关 a_r_flag = auto_run_flag */
	else if (memcmp(dat, "a_r_flag", 8) == 0)
	{
		//保存按钮开关状态
		if (*(dat + 8) == 1)
		{
			hmi_cmd_transmit("index.t5.txt=\"定时模式开启\"");
			SettingRegBuff.auto_run_flag = 1;
		}
		else
		{
			hmi_cmd_transmit("index.t5.txt=\"定时模式关闭\"");
			SettingRegBuff.auto_run_flag = 0;
		}
		HmiReg.is_alarm = 0;
		SaveSettingToFlash();
	}

	/* 立即执行 */
	else if (memcmp(dat, "run_now", 7) == 0)
	{
		click_run();
	}

	/* 放水 */
	else if (memcmp(dat, "water_out", 9) == 0)
	{
		if (!HmiReg.is_water_full)//非警戒水位
		{
			water_in_handle();
		}
	}
	/* 进水 */
	else if (memcmp(dat, "water_in", 8) == 0)
	{
		if (!HmiReg.is_water_full)//非警戒水位
		{
			water_in_handle();
		}
	}

	/* 完成 */
	else if (memcmp(dat, "water_finish", 12) == 0)
	{
		if (!HmiReg.is_water_full)//非警戒水位
		{
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
			hmi_cmd_transmit("index.t5.txt=\"所有操作已完成\"");
			hmi_cmd_transmit("index.t5.pco=%s", GREEN);
			hmi_cmd_transmit("vis t6,0");
			hmi_cmd_transmit("vis time,0");
			hmi_cmd_transmit("vis b1,1");
		}
	}

	/* 停止操作 wt_stop = water_stop */
	else if (memcmp(dat, "wt_stop", 7) == 0)
	{
		HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
		HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
	}

	/* 打开设置 */
	else if (memcmp(dat, "setting", 7) == 0)
	{
		HmiReg.now_page = SETTING;
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
		HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
		char str[10];
		Int2String(SettingRegBuff.sAlarm_hr, str);
		hmi_cmd_transmit("setting.hr.val=%s", str);
		Int2String(SettingRegBuff.sAlarm_min, str);
		hmi_cmd_transmit("setting.min.val=%s", str);
		Int2String(SettingRegBuff.isAlarmOpen, str);
		hmi_cmd_transmit("setting.c0.val=%s", str);
	}

	/* 时间设置页面 */
	else if (memcmp(dat, "time_set", 8) == 0)
	{
		HmiReg.now_page = TIME_SET;
	}

	/* 时间设置 */
	else if (memcmp(dat, "tm_set", 6) == 0)
	{
		//往RTC写时间
		RTC_TimeTypeDef sTime;
		RTC_DateTypeDef sDate;
		sTime.Hours = (uint8_t)(*(int8_t*)(dat + 14));
		sTime.Minutes = (uint8_t)(*(int8_t*)(dat + 16));
		sTime.Seconds = (uint8_t)(*(int8_t*)(dat + 18));
		sDate.Year = (uint8_t)(*(int16_t*)(dat + 6) - 2000);
		sDate.Month = (uint8_t)(*(int8_t*)(dat + 10));
		sDate.Date = (uint8_t)(*(int8_t*)(dat + 12));
		SetTime(sTime, sDate);
	}

	/* 定时设置---小时 p_set_tm_hr = param_set_timer */
	else if (memcmp(dat, "p_set_tm_hr", 11) == 0)
	{
		SettingRegBuff.sAlarm_hr = *(int8_t*)(dat + 11);
	}

	/* 定时设置---分钟 p_set_tm_min = param_set_timer */
	else if (memcmp(dat, "p_set_tm_min", 12) == 0)
	{
		SettingRegBuff.sAlarm_min = *(int8_t*)(dat + 12);
	}

	/* 声音警报设置 */
	else if (memcmp(dat, "alarm", 5) == 0)
	{
		SettingRegBuff.isAlarmOpen = *(uint16_t*)(dat + 5);
	}

	/* 录音 */
	else if (memcmp(dat, "rec", 3) == 0)
	{
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_SET);
	}

	/* 停止录音 */
	else if (memcmp(dat, "stop_rec", 8) == 0)
	{
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_RESET);
	}

	/* 播放 */
	else if (memcmp(dat, "playe", 5) == 0)
	{
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_SET);
	}

	/* 手动模式页面 mcrtl = manual_control */
	else if (memcmp(dat, "m_ctrl", 6) == 0)
	{
		HmiReg.now_page = M_CTRL;
	}

	/* 手动放水 m_wt_o = manual_control_water_out */
	else if (memcmp(dat, "m_wt_o", 6) == 0)
	{
		if (*(uint16_t*)(dat + 6))//判断双态按钮的值是否按下
		{
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
			HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//打开出水阀门
			hmi_cmd_transmit("mctrl.bt0.txt=\"结束放水\"");
			hmi_cmd_transmit("mctrl.bt1.txt=\"开始进水\"");
		}
		else
		{
			HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
			hmi_cmd_transmit("mctrl.bt0.txt=\"开始放水\"");
		}
	}

	/* 手动进水 m_wt_i = manual_control_water_in */
	else if (memcmp(dat, "m_wt_i", 6) == 0)
	{
		if (*(uint16_t*)(dat + 6))
		{
			HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_RESET);//打开进水阀门
			hmi_cmd_transmit("mctrl.bt1.txt=\"结束进水\"");
			hmi_cmd_transmit("mctrl.bt0.txt=\"开始放水\"");
		}
		else
		{
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门
			hmi_cmd_transmit("mctrl.bt1.txt=\"开始进水\"");
		}
	}

	/* 键盘页面 */
	else if (memcmp(dat, "keyboard", 8) == 0)
	{
		HmiReg.now_page = KEY;
	}
}