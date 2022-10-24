
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

/*ָ����д����Ϣ������*/
void hmi_cmd_transmit(char* cmd, ...)//��κ��������3������
{
	va_list valist;
	va_start(valist, cmd);
	char* p = cmd;

	uint8_t arg_num = 0;//��������
	char* str[3];//ʹ���ַ����������������

	while (*p)
	{
		if (*p == '%' && *(p + 1) == 's')//ʹ��%s���жϲ�������
		{
			str[arg_num] = va_arg(valist, char*);//�����ַ���ָ��
			arg_num++;
		}
		p++;
	}

	HMI_TRANSMIT_MSG_t* hmi_transmit_buff = (HMI_TRANSMIT_MSG_t*)malloc(sizeof(HMI_TRANSMIT_MSG_t));//���䷢�ͻ������ڴ�
	//HMI_TRANSMIT_MSG_t* hmi_transmit_buff = (HMI_TRANSMIT_MSG_t*)pvPortMalloc(sizeof(HMI_TRANSMIT_MSG_t));//���䷢�ͻ������ڴ�
	memset(hmi_transmit_buff, 0x00, sizeof(HMI_TRANSMIT_MSG_t));//��������ʼ��
	hmi_transmit_buff->type = EVENT_UART_TX;//��Ϣ����
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
	memset(hmi_transmit_buff->buff + hmi_transmit_buff->size, 0xff, 3);//hmiָ�������
	hmi_transmit_buff->size += 3;
	osMessageQueuePut(LcdTxQueueHandle, hmi_transmit_buff, 0, 0);
	free(hmi_transmit_buff);//�ͷŷ��ͻ������ڴ��ַ
	//vPortFree(hmi_transmit_buff);

	va_end(valist);//�ͷſɱ�����б��ַ
}

/* ˢ������Ļʱ�� */
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

/* ˢ������Ļ���� */
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
		hmi_cmd_transmit("index.t7.txt=\"����\"");
		break;
	case RTC_WEEKDAY_MONDAY:
		hmi_cmd_transmit("index.t7.txt=\"��һ\"");
		break;
	case RTC_WEEKDAY_TUESDAY:
		hmi_cmd_transmit("index.t7.txt=\"�ܶ�\"");
		break;
	case RTC_WEEKDAY_WEDNESDAY:
		hmi_cmd_transmit("index.t7.txt=\"����\"");
		break;
	case RTC_WEEKDAY_THURSDAY:
		hmi_cmd_transmit("index.t7.txt=\"����\"");
		break;
	case RTC_WEEKDAY_FRIDAY:
		hmi_cmd_transmit("index.t7.txt=\"����\"");
		break;
	case RTC_WEEKDAY_SATURDAY:
		hmi_cmd_transmit("index.t7.txt=\"����\"");
		break;
	default:
		break;
	}
}



/* ִ�з�ˮ */
void water_out_handle()
{
	hmi_cmd_transmit("index.t5.txt=\"����ִ�з�ˮ...\"");
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//�򿪳�ˮ����
}
/* ִ�н�ˮ */
void water_in_handle()
{
	hmi_cmd_transmit("index.t5.txt=\"����ִ�н�ˮ...\"");
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_RESET);//�򿪽�ˮ����
}
/* ֹͣ */
void water_stop_handle()
{
	hmi_cmd_transmit("vis b2,0");
	hmi_cmd_transmit("vis b1,1");
	hmi_cmd_transmit("index.t5.txt=\"ֹͣ��������\"");
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
}

/* ˮ������ */
void water_full_handle()
{
	if (HmiReg.now_page != INDEX)//�����ǰҳ�治����ҳ���򷵻���ҳ�ٿ�ʼ����
	{
		hmi_cmd_transmit("page index");
	}
	hmi_cmd_transmit("vis bt0,0");
	hmi_cmd_transmit("vis b1,0");
	hmi_cmd_transmit("vis b2,0");
	hmi_cmd_transmit("index.t5.txt=\"���ﾯ��ˮλ����������ˮ...\"");
	hmi_cmd_transmit("index.t5.pco=%s", RED);
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//�򿪳�ˮ����
}

void warning_clear_handle()
{
	hmi_cmd_transmit("vis bt0,1");
	hmi_cmd_transmit("vis b1,1");
	hmi_cmd_transmit("vis b2,1");
	hmi_cmd_transmit("index.t5.txt=\"ˮλ�ָ����������������...\"");
	hmi_cmd_transmit("index.t5.pco=%s", GREEN);
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
}

/* ִ��һ�λ�ˮ���� */
void click_run()
{
	HmiReg.run_once = 1;
	//if (HmiReg.is_water_low)//�Ѿ������ˮλ
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
	/* ����ҳ */
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

	/* ���浽flash */
	else if (memcmp(dat, "save", 4) == 0)
	{
		SaveSettingToFlash();
		//SetAlarm_IT(SettingRegBuff.sAlarm_hr, SettingRegBuff.sAlarm_min, 0);
	}

	/* ��ʱ�Զ�ģʽ���� a_r_flag = auto_run_flag */
	else if (memcmp(dat, "a_r_flag", 8) == 0)
	{
		//���水ť����״̬
		if (*(dat + 8) == 1)
		{
			hmi_cmd_transmit("index.t5.txt=\"��ʱģʽ����\"");
			SettingRegBuff.auto_run_flag = 1;
		}
		else
		{
			hmi_cmd_transmit("index.t5.txt=\"��ʱģʽ�ر�\"");
			SettingRegBuff.auto_run_flag = 0;
		}
		HmiReg.is_alarm = 0;
		SaveSettingToFlash();
	}

	/* ����ִ�� */
	else if (memcmp(dat, "run_now", 7) == 0)
	{
		click_run();
	}

	/* ��ˮ */
	else if (memcmp(dat, "water_out", 9) == 0)
	{
		if (!HmiReg.is_water_full)//�Ǿ���ˮλ
		{
			water_in_handle();
		}
	}
	/* ��ˮ */
	else if (memcmp(dat, "water_in", 8) == 0)
	{
		if (!HmiReg.is_water_full)//�Ǿ���ˮλ
		{
			water_in_handle();
		}
	}

	/* ��� */
	else if (memcmp(dat, "water_finish", 12) == 0)
	{
		if (!HmiReg.is_water_full)//�Ǿ���ˮλ
		{
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
			hmi_cmd_transmit("index.t5.txt=\"���в��������\"");
			hmi_cmd_transmit("index.t5.pco=%s", GREEN);
			hmi_cmd_transmit("vis t6,0");
			hmi_cmd_transmit("vis time,0");
			hmi_cmd_transmit("vis b1,1");
		}
	}

	/* ֹͣ���� wt_stop = water_stop */
	else if (memcmp(dat, "wt_stop", 7) == 0)
	{
		HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
		HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
	}

	/* ������ */
	else if (memcmp(dat, "setting", 7) == 0)
	{
		HmiReg.now_page = SETTING;
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
		HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
		char str[10];
		Int2String(SettingRegBuff.sAlarm_hr, str);
		hmi_cmd_transmit("setting.hr.val=%s", str);
		Int2String(SettingRegBuff.sAlarm_min, str);
		hmi_cmd_transmit("setting.min.val=%s", str);
		Int2String(SettingRegBuff.isAlarmOpen, str);
		hmi_cmd_transmit("setting.c0.val=%s", str);
	}

	/* ʱ������ҳ�� */
	else if (memcmp(dat, "time_set", 8) == 0)
	{
		HmiReg.now_page = TIME_SET;
	}

	/* ʱ������ */
	else if (memcmp(dat, "tm_set", 6) == 0)
	{
		//��RTCдʱ��
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

	/* ��ʱ����---Сʱ p_set_tm_hr = param_set_timer */
	else if (memcmp(dat, "p_set_tm_hr", 11) == 0)
	{
		SettingRegBuff.sAlarm_hr = *(int8_t*)(dat + 11);
	}

	/* ��ʱ����---���� p_set_tm_min = param_set_timer */
	else if (memcmp(dat, "p_set_tm_min", 12) == 0)
	{
		SettingRegBuff.sAlarm_min = *(int8_t*)(dat + 12);
	}

	/* ������������ */
	else if (memcmp(dat, "alarm", 5) == 0)
	{
		SettingRegBuff.isAlarmOpen = *(uint16_t*)(dat + 5);
	}

	/* ¼�� */
	else if (memcmp(dat, "rec", 3) == 0)
	{
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_SET);
	}

	/* ֹͣ¼�� */
	else if (memcmp(dat, "stop_rec", 8) == 0)
	{
		HAL_GPIO_WritePin(REC_GPIO_Port, REC_Pin, GPIO_PIN_RESET);
	}

	/* ���� */
	else if (memcmp(dat, "playe", 5) == 0)
	{
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_SET);
	}

	/* �ֶ�ģʽҳ�� mcrtl = manual_control */
	else if (memcmp(dat, "m_ctrl", 6) == 0)
	{
		HmiReg.now_page = M_CTRL;
	}

	/* �ֶ���ˮ m_wt_o = manual_control_water_out */
	else if (memcmp(dat, "m_wt_o", 6) == 0)
	{
		if (*(uint16_t*)(dat + 6))//�ж�˫̬��ť��ֵ�Ƿ���
		{
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
			HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//�򿪳�ˮ����
			hmi_cmd_transmit("mctrl.bt0.txt=\"������ˮ\"");
			hmi_cmd_transmit("mctrl.bt1.txt=\"��ʼ��ˮ\"");
		}
		else
		{
			HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
			hmi_cmd_transmit("mctrl.bt0.txt=\"��ʼ��ˮ\"");
		}
	}

	/* �ֶ���ˮ m_wt_i = manual_control_water_in */
	else if (memcmp(dat, "m_wt_i", 6) == 0)
	{
		if (*(uint16_t*)(dat + 6))
		{
			HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//�رճ�ˮ����
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_RESET);//�򿪽�ˮ����
			hmi_cmd_transmit("mctrl.bt1.txt=\"������ˮ\"");
			hmi_cmd_transmit("mctrl.bt0.txt=\"��ʼ��ˮ\"");
		}
		else
		{
			HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//�رս�ˮ����
			hmi_cmd_transmit("mctrl.bt1.txt=\"��ʼ��ˮ\"");
		}
	}

	/* ����ҳ�� */
	else if (memcmp(dat, "keyboard", 8) == 0)
	{
		HmiReg.now_page = KEY;
	}
}