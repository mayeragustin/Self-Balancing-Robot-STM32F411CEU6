/*
 * AT_Commands.h
 *
 *  Created on: May 22, 2025
 *      Author: agust
 */

#ifndef INC_WIFI_AT_COMMANDS_H_
#define INC_WIFI_AT_COMMANDS_H_

#include "utilities.h"


typedef enum{
	AT_BUSY,
	AT_ERROR,
	AT_TIMEOUT,
	AT_SENT
}AT_STATUS;


AT_STATUS AT_WriteCMD();

void AT_Set_UART_Communication(void (*Tx_Func)(uint8_t *buff, uint8_t *indexRead, uint8_t indexWrite));

/**
 * @brief Esta función agrega un dato al buffer de recepción
 *
 * @param data: valor recibido
 */
void AT_Get_Data(uint8_t data);

/**
 * void dataTo_PC(s_uartData* dataCom){
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE)){
		USART1->DR = dataCom->Tx.buffer[dataCom->Tx.read++];
	}
}
 *
 */

e_system AT_Send_Command(uint8_t *cmd, uint8_t len, void (*CallBack)(void *data), uint16_t timeout);

e_system AT_Alive();

#endif /* INC_WIFI_AT_COMMANDS_H_ */
