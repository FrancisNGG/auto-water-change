#pragma once
#ifndef __BSP_UART_H__
#define __BSP_UART_H__
#include "main.h"

void UART_IDLECallBack(UART_HandleTypeDef* huart);

//typedef struct HMIUartReg_s
//{
//	uint8_t tx_buff[128];
//	uint8_t tx_buff_size;
//	uint8_t rx_buff[64];
//
//}__packed HMIUartReg_t;

typedef struct HMI_TRANSMIT_MSG_s
{
	uint8_t type;
	uint8_t size;
	uint8_t buff[128];
}__packed HMI_TRANSMIT_MSG_t;

//typedef struct HMI_RECEIVE_MSG_s
//{
//	uint8_t type;
//	uint8_t *pdata;
//}__packed HMI_RECEIVE_MSG_t;

typedef enum
{
	EVENT_UART_RX = 1,
	EVENT_UART_TX,
}EVENT_UART_e;

//extern HMIUartReg_t HMIUartReg;
extern void uart_init(UART_HandleTypeDef* huart);
extern void uart_deinit(UART_HandleTypeDef* huart);
extern void hmi_cmd_transmit_handle(void);
extern void hmi_cmd_receive_handle(void);

#endif // !__BSP_UART_H__
