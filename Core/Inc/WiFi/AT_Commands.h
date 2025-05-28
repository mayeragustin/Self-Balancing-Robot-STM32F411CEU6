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

#endif /* INC_WIFI_AT_COMMANDS_H_ */
