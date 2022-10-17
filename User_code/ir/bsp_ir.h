#pragma once
#ifndef __BSP_IR_H__
#define __BSP_IR_H__

#include "main.h"

extern TIM_HandleTypeDef htim4;

#define IR_TIM htim4
#define IR_TIM_CHANNE TIM_CHANNEL_1

#define IR_us_LOGIC_0 560
#define IR_us_LOGIC_1 1640
#define IR_us_START 4480
#define IR_us_REPEAT_START 39350
#define IR_us_REPEAT_END 2220
#define IR_DEVIATION 100	//��Χ

#define IR_CHECK_ADDRESS 1  //1������ַλ 0��ȡ��
#define IR_CHECK_COMMAND 1  //1���������λ 0��ȡ��
#define IR_CHECK_REPEAT 1   //1������ظ�   0��ȡ��

void IR_Init(void);
void IR_CaptureCallback(void);
uint8_t IR_Scanf(uint32_t* data);

/***************************************
 * 1        00FFA25D
 * 2        00FF629D
 * 3        00FFE21D
 * 4        00FF22DD
 * 5        00FF02FD
 * 6        00FFC23D
 * 7        00FFE01F
 * 8        00FFA857
 * 9        00FF906F
 * *        00FF6897
 * 0        00FF9867
 * #        00FFB04F
 * up       00FF18E7
 * left     00FF10EF
 * OK       00FF38C7
 * right    00FF5AA5
 * down     00FF4AB5
*****************************************/
/****************************************
 * ���벶���жϻص�������ʹ�ã������޸�TIM1
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (TIM1 == htim->Instance)
	{
		IR_CaptureCallback();
	}
}
******************************************/


#endif // !__BSP_IR_H__