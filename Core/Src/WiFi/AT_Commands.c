/*
 * AT_Commands.c
 *
 *  Created on: May 22, 2025
 *      Author: agust
 */


#include "WiFi/AT_Commands.h"

uint8_t *AT_CMD_Buffer;
e_system (*AT_Send_To_UART)(uint8_t *buff, uint16_t len);
/*
AT_STATUS AT_WriteCMD(){
	switch(){
	case 0: // mandar cmd a uart
		AT_Send_To_UART(buff, len);
		//case 1;
		break;
	case 1: // esperar a recibir el ok desde uart si timeout, case 4

		break;
	case 2: // verificar lo que llegó, si es ok ir a case 3, sinó case 4
		break;
	case 3: // devolver un OK en la función
		return AT_SENT;
		break;
	case 4: // devolver un error al usuario
		return AT_ERROR;
		break;
	default:
		return AT_ERROR;
	}
	if(){
		return AT_TIMEOUT;
	}
	return AT_BUSY;
}
*/
