#include "bsp_uart.h"
#include "user_task.h"
#include "stdlib.h"
#include "string.h"
#include "bsp_hmi.h"
#include "cmsis_os.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern osMessageQueueId_t LcdTxQueueHandle;

uint8_t uart_receive_flag;
uint8_t UartRxBuff[64];

void uart_init(UART_HandleTypeDef* huart)
{
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	HAL_UART_Receive_DMA(huart, UartRxBuff, sizeof(UartRxBuff));
}

void uart_deinit(UART_HandleTypeDef* huart)
{
	HAL_UART_DMAStop(huart);
	__HAL_UART_CLEAR_IDLEFLAG(huart);
}

/*串口空闲中断*/
void UART_IDLECallBack(UART_HandleTypeDef* huart)
{
	uart_deinit(huart);
	uart_receive_flag = 1;
	uart_init(huart);
}

void cmd_uart_transmit(uint8_t type, uint8_t* pdata, uint8_t size)
{
	if (type == EVENT_UART_TX)
	{
		HAL_UART_Transmit(&huart1, pdata, size, 0xff);
	}
}

/*串口指令发送消息处理*/
void hmi_cmd_transmit_handle(void)
{
	if (hdma_usart1_tx.State == HAL_DMA_STATE_READY)//UART1_DMA空闲才会读消息
	{
		HMI_TRANSMIT_MSG_t msg;
		uint8_t status = osMessageQueueGet(LcdTxQueueHandle, &msg, 0, 0);

		if (status == osOK)
		{
			cmd_uart_transmit(msg.type, msg.buff, msg.size);
		}		
	}
}

/*串口指令接收消息处理*/
void hmi_cmd_receive_handle(void)
{
	if (uart_receive_flag)
	{
		uint8_t RxBuff[64];
		memcpy(RxBuff, huart1.pRxBuffPtr, sizeof(UartRxBuff));
		MenuUARTFuntion(RxBuff);
		uart_receive_flag = 0;
	}
}
