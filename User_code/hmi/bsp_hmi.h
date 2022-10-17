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

typedef enum
{
	WAITING = 0,
	WATER_ERROR,
	WATER_OUT,
	WATER_IN,
	WATER_FINISH,
	WATER_FULL,
	WATER_FULL_CHECK
}STATUS_e;


typedef struct HMI_REG_s
{
	//uint8_t status;//整体状态
	uint8_t run_once;//执行一次标志位
	uint32_t *ir_data;//接收到的红外数据
	uint8_t is_time_up;//每秒刷新HMI时间标志位
	uint8_t is_date_up;//每秒刷新HMI时间标志位
	uint8_t is_alarm;//设置自动换水时间标志位
	uint8_t is_water_full;//水满标志位
	uint8_t is_water_low;//低水位标志位
	uint16_t water_error_time;//水满，自动放水的时长		
	uint8_t is_alarm_paly;//到达声音警报时间标志位
	uint16_t adc_val_last;//上一次ADC记录值
}__packed HMI_REG_t;

extern HMI_REG_t HmiReg;

extern void reflush_hmi_time();
extern void reflush_hmi_date();
extern void click_run();
extern void water_in_handle();
extern void water_out_handle();
extern void water_stop_handle();
extern void water_full_handle();
extern void warning_clear_handle();
extern void hmi_cmd_transmit(char* cmd, ...);
extern void MenuUARTFuntion(uint8_t* dat);

#endif // !__bsp_HMI_H__
