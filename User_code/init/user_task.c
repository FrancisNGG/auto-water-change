#include "user_task.h"
#include "main.h"
#include "bsp_hmi.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "bsp_uart.h"
#include "bsp_flash.h"
#include "bsp_hmi.h"
#include "bsp_rtc.h"
#include "bsp_ir.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern RTC_HandleTypeDef hrtc;

extern void debug();

/* Definitions for LcdTxRxTask */
osThreadId_t LcdTaskHandle;
const osThreadAttr_t LcdTask_attributes = {
  .name = "LcdTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for RtcTask */
osThreadId_t RtcTaskHandle;
const osThreadAttr_t RtcTask_attributes = {
  .name = "RtcTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t)osPriorityNormal,
};

/* Definitions for LcdTxQueue */
osMessageQueueId_t LcdTxQueueHandle;
const osMessageQueueAttr_t LcdTxQueue_attributes = {
  .name = "LcdTxQueue"
};

//LCD����
void StartLcdTask(void* argument)
{
	uart_init(&huart1);//��ʼ������
	for (;;)
	{
		hmi_cmd_receive_handle();
		hmi_cmd_transmit_handle();
	}
}

//RTC����
void StartRtcTask(void* argument)
{
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);//������ʱ����ȡʱ��
	for (;;)
	{
		if (HmiReg.is_time_up)//ˢ��ʱ��
		{
			reflush_hmi_time();
			HmiReg.is_time_up = 0;
		}
		if (HmiReg.is_date_up)//ˢ������
		{
			reflush_hmi_date();
			HmiReg.is_date_up = 0;
		}
	}
}

/* �����ʼ�� */
void user_task_init(void)
{
	/* creation of LcdTxQueue */
	LcdTxQueueHandle = osMessageQueueNew(16, sizeof(HMI_TRANSMIT_MSG_t), &LcdTxQueue_attributes);

	/* creation of LcdTxRxTask */
	LcdTaskHandle = osThreadNew(StartLcdTask, NULL, &LcdTask_attributes);
	/* creation of RtcTask */
	RtcTaskHandle = osThreadNew(StartRtcTask, NULL, &RtcTask_attributes);
}

/* Ĭ�������ʼ�� */
void begin()
{
	debug();

	/* ���߹رյ�ŷ� */
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);

	/*��ȡ������Ϣ*/
	memset(&SettingRegBuff, 0x00, sizeof(SettingRegBuff));
	memcpy(&SettingRegBuff, pSettingReg, sizeof(SettingRegBuff));

	/*������Ϣ��������д��Ĭ������*/
	if (SettingRegBuff.isExist != 1)
	{
		setting_init();
		SaveSettingToFlash();
	}

	HmiReg.ir_data = (uint32_t*)malloc(sizeof(uint32_t));
	IR_Init();

	HAL_ADC_Start(&hadc1);

	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);

	osDelay(500);//��Ļ������
	hmi_cmd_transmit("page index");//ˢ����Ļ

	//adc_val = HAL_ADC_GetValue(&hadc1);
	//adc_val_last = adc_val;
}

/* Ĭ������ѭ���� */
void loop()
{
	if (HmiReg.is_alarm && SettingRegBuff.auto_run_flag && HmiReg.is_water_full == 0)//�����Զ���ˮʱ��
	{
		click_run();
		osDelay(100);
		HmiReg.is_alarm = 0;
	}
	if (HmiReg.run_once && HmiReg.is_water_low)
	{
		water_in_handle();
	}
	if (HmiReg.is_alarm_paly)//�����ӳٺ���δ�ָ��������򲥷������澯
	{
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_SET);
		osDelay(100);
		HAL_GPIO_WritePin(PLAY_E_GPIO_Port, PLAY_E_Pin, GPIO_PIN_RESET);
		osDelay(100);
	}
	if (IR_Scanf(HmiReg.ir_data))
	{
		switch (*HmiReg.ir_data)
		{
		case IS_WATER_ON:
			click_run();
			break;
		case IS_WATER_IN:
			water_in_handle();
			break;
		case IS_WATER_OUT:
			water_out_handle();
			break;
		case IS_STOP:
			water_stop_handle();
			break;
		default:
			break;
		}
	}
}

//ˮ�����
void WaterFullCheck(void)
{
	if ((HAL_ADC_GetValue(&hadc1) != HmiReg.adc_val_last) &&
		(HmiReg.adc_val_last + 100) < HAL_ADC_GetValue(&hadc1) ||
		(HmiReg.adc_val_last - 100) > HAL_ADC_GetValue(&hadc1))
	{
		HmiReg.adc_val_last = HAL_ADC_GetValue(&hadc1);
		if (HAL_ADC_GetValue(&hadc1) > 3400)
		{
			HmiReg.is_water_full = 0;
			HmiReg.run_once = 0;
			warning_clear_handle();
			HmiReg.is_alarm_paly = 0;
		}
		else
		{
			HmiReg.is_water_full = 1;
			if (SettingRegBuff.isAlarmOpen)
			{
				HmiReg.is_alarm_paly = 1;
			}
			water_full_handle();
		}
	}
}

/*
TIM �����������ʱ����㣺

Tout=��ARR+1)(PSC+1)/TIMxCLK

���У�Tout�ĵ�λΪus��TIMxCLK�ĵ�λΪMHz��

����( 7200 * 10000 ) / 72M = 1000000 us = 1000 ms = 1 s

*/
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim)
{
	if (htim == &htim3)//ÿ�������Ӻ�ˮ��
	{
		AlarmCheck();
		WaterFullCheck();
	}
	if (htim == &htim2)
	{
		GetTime(&RTC_Time, &RTC_Date);
		HmiReg.is_time_up = 1;
		HmiReg.is_date_up = 1;
	}
}


//�����жϴ���
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case WATER_LOW_Pin:
		if (HAL_GPIO_ReadPin(WATER_LOW_GPIO_Port, WATER_LOW_Pin) == GPIO_PIN_SET)//�����ˮλ
		{
			HmiReg.is_water_low = 1;
		}
		else
		{
			HmiReg.is_water_low = 0;
		}
		break;
	default:
		break;

	}
}

/* ���Ź���ʱʱ����㹫ʽ��

��ʱʱ��Tout = 4 * 2 ^ PR * (����ֵ + 1) / LSI

����ֵȡֵ��Χ��0~4095

��F1Ϊ����4��Ƶʱ����Ӧ��PRλΪ0������ֵΪ0ʱ����̵ĳ�ʱʱ�䣬���ù�ʽ��
4 * 2 ^ 0 * (0 + 1) / 40000HZ = 0.0001s = 0.1ms

������ֵΪ4095ʱ�����ʱʱ�䣬���ù�ʽ��
4 * 2 ^ 0 (4095 + 1) / 40000HZ = 0.4096s = 409.6ms

*/


char* Int2String(int num, char* str)//10���� 
{
	int i = 0;//ָʾ���str 
	if (num < 0)//���numΪ��������num���� 
	{
		num = -num;
		str[i++] = '-';
	}
	//ת�� 
	do
	{
		str[i++] = num % 10 + 48;//ȡnum���λ �ַ�0~9��ASCII����48~57������˵����0+48=48��ASCII���Ӧ�ַ�'0' 
		num /= 10;//ȥ�����λ    
	} while (num);//num��Ϊ0����ѭ��

	str[i] = '\0';

	//ȷ����ʼ������λ�� 
	int j = 0;
	if (str[0] == '-')//����и��ţ����Ų��õ��� 
	{
		j = 1;//�ӵڶ�λ��ʼ���� 
		++i;//�����и��ţ����Խ����ĶԳ���ҲҪ����1λ 
	}
	//�Գƽ��� 
	for (; j < i / 2; j++)
	{
		//�Գƽ������˵�ֵ ��ʵ����ʡ���м��������a+b��ֵ��a=a+b;b=a-b;a=a-b; 
		str[j] = str[j] + str[i - 1 - j];
		str[i - 1 - j] = str[j] - str[i - 1 - j];
		str[j] = str[j] - str[i - 1 - j];
	}

	return str;//����ת�����ֵ 
}

int String2Int(char* str)//�ַ���ת���� 
{
	char flag = '+';//ָʾ����Ƿ������ 
	long res = 0;

	if (*str == '-')//�ַ��������� 
	{
		++str;//ָ����һ���ַ� 
		flag = '-';//����־��Ϊ���� 
	}
	//����ַ�ת�������ۼӵ����res 
	while (*str >= 48 && *str < 57)//��������ֲŽ���ת��������0~9��ASCII�룺48~57 
	{
		res = 10 * res + *str++ - 48;//�ַ�'0'��ASCII��Ϊ48,48-48=0�պ�ת��Ϊ����0 
	}

	if (flag == '-')//�����Ǹ��������
	{
		res = -res;
	}

	return (int)res;
}




