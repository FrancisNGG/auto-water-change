#ifndef __bsp_HMI_H__
#define __bsp_HMI_H__
#include "main.h"
#include "stdbool.h"

#define RED "63488"
#define GREEN "2016"

#define IS_WATER_ON 0x00FF38C7
#define IS_WATER_IN 0x00FF18E7
#define IS_WATER_OUT 0x00FF4AB5
#define IS_STOP 0x00FFB04F

#define WATER_OUT_ON	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_RESET);//打开出水阀门
#define WATER_OUT_OFF	HAL_GPIO_WritePin(WATER_OUT_SW_GPIO_Port, WATER_OUT_SW_Pin, GPIO_PIN_SET);//关闭出水阀门
#define WATER_IN_ON		HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_RESET);//打开进水阀门
#define WATER_IN_OFF	HAL_GPIO_WritePin(WATER_IN_SW_GPIO_Port, WATER_IN_SW_Pin, GPIO_PIN_SET);//关闭进水阀门

typedef enum
{
	INDEX = 0,
	M_CTRL,
	TIME_SET,
	SETTING,
	KEY
}NOW_PAGE_e;


typedef struct HMI_REG_s
{
	uint8_t now_page;//当前页面
	uint8_t run_once;//执行一次标志位
	uint32_t *ir_data;//接收到的红外数据
	uint8_t is_time_up;//每秒刷新HMI时间标志位
	uint8_t is_date_up;//每秒刷新HMI时间标志位
	uint8_t is_alarm;//设置自动换水时间标志位
	uint8_t is_water_warning;//水满标志位
	uint8_t is_water_low;//低水位标志位
	uint8_t is_water_high;//高水位标志位
	uint16_t water_error_time;//水满，自动放水的时长		
	uint8_t is_alarm_paly;//到达声音警报时间标志位
	int16_t adc_val_last;//上一次ADC记录值
}__packed HMI_REG_t;

extern HMI_REG_t HmiReg;

extern void reflush_hmi_time();
extern void reflush_hmi_date();
extern void click_run();
extern void water_in_handle();
extern void water_out_handle();
extern void water_stop_handle();
extern void water_high_handle();
extern void water_warning_handle();
extern void warning_clear_handle();
extern void hmi_cmd_transmit(char* cmd, ...);
extern void MenuUARTFuntion(uint8_t* dat);

#endif // !__bsp_HMI_H__
