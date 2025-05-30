/*
 * ESP01.h
 *
 *  Created on: May 22, 2025
 *      Author: agust
 */

#ifndef INC_WIFI_ESP01_H_
#define INC_WIFI_ESP01_H_

/*
#include <stdint.h>
#include "utilities.h"
//#include "WiFi/AT_Config.h"


#define WIFI_AT						"AT"

#define WIFI_SET_MODE_RAM			"CWMODE_CUR"
#define WIFI_CONFIG_SOFTAP_RAM		"CWSAP_CUR"
#define WIFI_SET_CONNECTION			"CIPSTART"

#define WIFI_PING					"PING"
#define WIFI_SEND_DATA				"CIPSEND"

#define WIFI_CONNECT_AP_RAM			"CWJAP_CUR"
#define WIFI_CONNECT_AP_FLASH		"CWJAP_DEF"|
#define WIFI_ACTIVE_AUTOCONNECT		"CWAUTOCONN"
#define	WIFI_GET_IPMAC				"CIFSR"

#define WIFI_STATION_MODE			1
#define	WIFI_SOFTAP_MODE			2
#define	WIFI_SOFTAP_STATION_MODE	3


typedef enum{
	ESP_ERROR = -1,
	ESP_OK = 0,
}e_ESPSTATE;

e_system ESP01_Init();

e_system ESP01_Task();
*/

void ESP01_Task();
#endif /* INC_WIFI_ESP01_H_ */
