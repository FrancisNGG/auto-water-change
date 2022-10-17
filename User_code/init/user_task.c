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

//LCD任务
void StartLcdTask(void* argument)
{
	uart_init(&huart1);//初始化串口
	for (;;)
	{
		hmi_cmd_receive_handle();
		hmi_cmd_transmit_handle();
	}
}

//RTC任务
void StartRtcTask(void* argument)
{
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);//启动定时器获取时间
	for (;;)
	{
		if (HmiReg.is_time_up)//刷新时间
		{
			reflush_hmi_time();
			HmiReg.is_time_up = 0;
		}
		if (HmiReg.is_date_up)//刷新日期
		{
			reflush_hmi_date();
			HmiReg.is_date_up = 0;
		}
	}
}

/* 任务初始化 */
void user_task_init(void)
{
	/* creation of LcdTxQueue */
	LcdTxQueueHandle = osMessageQueueNew(16, sizeof(HMI_TRANSMIT_MSG_t), &LcdTxQueue_attributes);

	/* creation of LcdTxRxTask */
	LcdTaskHandle = osThreadNew(StartLcdTask, NULL, &LcdTask_attributes);
	/* creation of RtcTask */
	RtcTaskHandle = osThreadNew(StartRtcTask, NULL, &RtcTask_attributes);
}

/* 默认任务初始化 */
void begin()
{
	debug();

	/* 拉高关闭电磁阀 */
	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);

	/*读取配置信息*/
	memset(&SettingRegBuff, 0x00, sizeof(SettingRegBuff));
	memcpy(&SettingRegBuff, pSettingReg, sizeof(SettingRegBuff));

	/*配置信息不存在则写入默认配置*/
	if (SettingRegBuff.isExist != 1)
	{
		setting_init();
		SaveSettingToFlash();
	}

	HmiReg.ir_data = (uint32_t*)malloc(sizeof(uint32_t));
	IR_Init();

	HAL_ADC_Start(&hadc1);

	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);

	osDelay(500);//屏幕跟不上
	hmi_cmd_transmit("page index");//刷新屏幕

	//adc_val = HAL_ADC_GetValue(&hadc1);
	//adc_val_last = adc_val;
}

/* 默认任务循环体 */
void loop()
{
	if (HmiReg.is_alarm && SettingRegBuff.auto_run_flag && HmiReg.is_water_full == 0)//到达自动换水时间
	{
		click_run();
		osDelay(100);
		HmiReg.is_alarm = 0;
	}
	if (HmiReg.run_once && HmiReg.is_water_low)
	{
		water_in_handle();
	}
	if (HmiReg.is_alarm_paly)//警报延迟后，如未恢复正常，则播放声音告警
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

//水满检测
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
TIM 超出（溢出）时间计算：

Tout=（ARR+1)(PSC+1)/TIMxCLK

其中：Tout的单位为us，TIMxCLK的单位为MHz。

例：( 7200 * 10000 ) / 72M = 1000000 us = 1000 ms = 1 s

*/
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim)
{
	if (htim == &htim3)//每秒检测闹钟和水满
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


//警报中断处理
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case WATER_LOW_Pin:
		if (HAL_GPIO_ReadPin(WATER_LOW_GPIO_Port, WATER_LOW_Pin) == GPIO_PIN_SET)//到达低水位
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

/* 看门狗超时时间计算公式：

超时时间Tout = 4 * 2 ^ PR * (重载值 + 1) / LSI

重载值取值范围：0~4095

以F1为例，4分频时所对应的PR位为0，重载值为0时是最短的超时时间，套用公式后：
4 * 2 ^ 0 * (0 + 1) / 40000HZ = 0.0001s = 0.1ms

当重载值为4095时是最大超时时间，套用公式：
4 * 2 ^ 0 (4095 + 1) / 40000HZ = 0.4096s = 409.6ms

*/


char* Int2String(int num, char* str)//10进制 
{
	int i = 0;//指示填充str 
	if (num < 0)//如果num为负数，将num变正 
	{
		num = -num;
		str[i++] = '-';
	}
	//转换 
	do
	{
		str[i++] = num % 10 + 48;//取num最低位 字符0~9的ASCII码是48~57；简单来说数字0+48=48，ASCII码对应字符'0' 
		num /= 10;//去掉最低位    
	} while (num);//num不为0继续循环

	str[i] = '\0';

	//确定开始调整的位置 
	int j = 0;
	if (str[0] == '-')//如果有负号，负号不用调整 
	{
		j = 1;//从第二位开始调整 
		++i;//由于有负号，所以交换的对称轴也要后移1位 
	}
	//对称交换 
	for (; j < i / 2; j++)
	{
		//对称交换两端的值 其实就是省下中间变量交换a+b的值：a=a+b;b=a-b;a=a-b; 
		str[j] = str[j] + str[i - 1 - j];
		str[i - 1 - j] = str[j] - str[i - 1 - j];
		str[j] = str[j] - str[i - 1 - j];
	}

	return str;//返回转换后的值 
}

int String2Int(char* str)//字符串转数字 
{
	char flag = '+';//指示结果是否带符号 
	long res = 0;

	if (*str == '-')//字符串带负号 
	{
		++str;//指向下一个字符 
		flag = '-';//将标志设为负号 
	}
	//逐个字符转换，并累加到结果res 
	while (*str >= 48 && *str < 57)//如果是数字才进行转换，数字0~9的ASCII码：48~57 
	{
		res = 10 * res + *str++ - 48;//字符'0'的ASCII码为48,48-48=0刚好转化为数字0 
	}

	if (flag == '-')//处理是负数的情况
	{
		res = -res;
	}

	return (int)res;
}




