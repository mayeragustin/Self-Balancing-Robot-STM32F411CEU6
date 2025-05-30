/*
 * AT_Commands.c
 *
 *  Created on: May 22, 2025
 *      Author: agust
 */


#include "WiFi/AT_Commands.h"
#include "utilities.h"
#include <stdint.h>

<<<<<<< HEAD
#define TX_BUFFER		256
/*
typedef struct{
	void
}s_Parameter;*/

=======
<<<<<<< HEAD
//uint8_t *AT_CMD_Buffer;
//e_system (*AT_Send_To_UART)(uint8_t *buff, uint16_t len);
/*
AT_STATUS AT_WriteCMD(){
	switch(){
	case 0: // mandar cmd a uart
		AT_Send_To_UART(buff, len);
		//case 1;
		break;
	case 1: // esperar a recibir el ok desde uart si timeout, case 4
=======
#define TX_BUFFER		256
>>>>>>> 3ede334a79d198cd1ab9095a598e9b3782bdaf42

/*typedef struct{
	void
}s_Parameter;*/

>>>>>>> 5ef2b499ed81eda5f9e6b9a46339a7875d4cf8bf
typedef struct{
	uint8_t buffer[TX_BUFFER];
	uint8_t iRead;
	uint8_t iWrite;
}s_comm;

struct{
	s_comm Tx;
	s_comm Rx;
	void (*Send_Data)(uint8_t *buff, uint8_t *indexRead, uint8_t indexWrite);
	void (*Current_CMD_Callback)(void *data);
	uint8_t waitingRta;
}ATCmd;

void AT_Set_UART_Comm(void (*Tx_Function)(uint8_t *buff, uint8_t *indexRead, uint8_t indexWrite)){
	ATCmd.Send_Data = Tx_Function;
}

void AT_Get_Data(uint8_t data){
	ATCmd.Rx.buffer[ATCmd.Rx.iWrite++] = data;
}

void AT_CMD_Task(){
	if(ATCmd.Tx.buffer[ATCmd.Tx.iRead] != ATCmd.Tx.buffer[ATCmd.Tx.iWrite]){
		ATCmd.Send_Data(ATCmd.Tx.buffer, &ATCmd.Tx.iRead, ATCmd.Tx.iWrite);
	}
	if(ATCmd.waitingRta){
		//ATCmd.Rx.buffer[ATCmd.Rx.iWrite];
	}
}

e_system AT_Send_Command(uint8_t *cmd, uint8_t len, void (*CallBack)(void *data), uint16_t timeout){
	if(!ATCmd.waitingRta){
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = 'A';
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = 'T';
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = '+';

		for(uint8_t i=0; i<len; i++){
			ATCmd.Tx.buffer[ATCmd.Tx.iWrite] = cmd[i];
			ATCmd.Tx.iWrite++;
		}

		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = '\r';
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = '\n';

		ATCmd.Current_CMD_Callback = CallBack;

		ATCmd.waitingRta = TRUE;

		return SYS_OK;
	}else{
		return SYS_BUSY;
	}
}

e_system AT_Alive(void (*CallBack)(void *data)){
	if(!ATCmd.waitingRta){
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = 'A';
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = 'T';
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = '\r';
		ATCmd.Tx.buffer[ATCmd.Tx.iWrite++] = '\n';
		ATCmd.Current_CMD_Callback = CallBack;
		ATCmd.waitingRta = TRUE;
		return SYS_OK;
	}else{
		return SYS_BUSY;
	}
}
