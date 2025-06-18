/*
 * ESP01.c
 * Version: 01b05 - 04/08/2024
 */

#define DEMOSTRACION_REGULARIZACION

#include "WiFi/ESP01.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


static enum {
	ESP01ATIDLE,
	ESP01ATWAITINGIP,
	ESP01ATAT,
	ESP01ATRESPONSE,
	ESP01ATCWMODE,
	ESP01ATCIPCLOSESERVER,
	ESP01ATCIPMUX,
	ESP01ATCWJAP,
	ESP01CWJAPRESPONSE,
	ESP01ATCIFSR,
	ESP01CIFSRRESPONSE,
	ESP01ATCIPCLOSE,
	ESP01ATCIPSTART,
	ESP01CIPSTARTRESPONSE,
	ESP01ATCONNECTED,
	ESP01ATCWQAP,
	ESP01ATCWSAP,
	ESP01ATCWSAP_RESPONSE,
	ESP01ATCWDHCP,
	ESP01_WAITING_CONNECTION,
	ESP01ATCIPSERVER,
	ESP01ATCIPSERVER_RESPONSE,
	ESP01ATCONFIGSERVER,
	ESP01ATCONFIGSERVER_RESPONSE,
	ESP01ATWAITSERVERDATA,
	ESP01ATHARDRST0,
	ESP01ATHARDRST1,
	ESP01ATHARDRSTSTOP,
} esp01ATState = ESP01ATIDLE;

static union{
	struct{
		uint8_t WAITINGSYMBOL: 1;
		uint8_t WIFICONNECTED: 1;
		uint8_t TXCIPSEND: 1;
		uint8_t SENDINGDATA: 1;
		uint8_t HRDRESETON: 1;
		uint8_t ATRESPONSEOK: 1;
		uint8_t UDPTCPCONNECTED: 1;
		uint8_t WAITINGRESPONSE: 1;
	}bit;
	uint8_t byte;
} esp01Flags;

static void ESP01ATDecode();
static void ESP01DOConnection();
static void ESP01SENDData();
static void ESP01StrToBufTX(const char *str);
static void ESP01ByteToBufTX(uint8_t value);
static int is_valid_ip(const char* ip);
static uint8_t ESP01_Check_Credentials(const char* ssid, const char* password, const char* ip);

static uint32_t esp01TimeoutTask = 0;
static uint32_t esp01TimeoutDataRx = 0;
static uint32_t esp01TimeoutTxSymbol = 0;
static void (*ESP01ChangeState)(_eESP01STATUS esp01State);
static void (*ESP01DbgStr)(const char *dbgStr);

static char esp01SSID[64] = {0};
static char esp01PASSWORD[32] = {0};
static char esp01RemoteIP[15] = {0};
static char esp01PROTO[4] = "UDP";
static char esp01RemotePORT[6] = {0};
static char esp01LocalIP[16] = {0};
static char esp01LocalPORT[6] = {0};

static uint8_t esp01HState = 0;
static uint16_t	esp01nBytes = 0;
static uint8_t esp01RXATBuf[ESP01RXBUFAT];
static uint8_t esp01TXATBuf[ESP01TXBUFAT];
static uint16_t	esp01iwRXAT = 0;
static uint16_t	esp01irRXAT = 0;
static uint16_t esp01irTX = 0;
static uint16_t esp01iwTX = 0;

static uint8_t esp01TriesAT = 0;

//static _sESP01Handle esp01Handle = {.DoCHPD = NULL, .WriteUSARTByte = NULL,
//									.bufRX = NULL, .iwRX = NULL, .sizeBufferRX = 0};
static _sESP01Handle esp01Handle = {.DoCHPD = NULL, .WriteUSARTByte = NULL, .WriteByteToBufRX = NULL};

const char ATAT[] = "AT\r\n";
const char ATCIPMUX[] = "AT+CIPMUX=";
const char ATCWQAP[] = "AT+CWQAP\r\n";
const char ATCWMODE[] = "AT+CWMODE=";
const char ATCIPCLOSESERVER[] = "AT+CIPSERVER=0";
const char ATCWJAP[] = "AT+CWJAP=";
const char ATCIFSR[] = "AT+CIFSR\r\n";
const char ATCIPSTART[] = "AT+CIPSTART=";
const char ATCIPCLOSE[] = "AT+CIPCLOSE\r\n";
const char ATCIPSEND[] = "AT+CIPSEND=";

const char ATCWSAP[] = "AT+CWSAP=\"SBR-MAYER\",\"\",5,0\r\n";
const char ATCWDHCP[] = "AT+CWDHCP_CUR=2,1\r\n";
const char ATCIPOPENSERVER[] = "AT+CIPSERVER=1,80\r\n";
//const char CONFIGSERVER[] = "<!DOCTYPE html><html><body><form action=\"/set\" method=\"GET\">SSID: <input name=\"ssid\"><br>PASS: <input name=\"pass\" type=\"password\"><br><input type=\"submit\" value=\"Connect\"></form></body></html>";
const char CONFIGSERVER[] = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Config</title></head>"
		"<body><h3>Mayer SBR Microcontroladores</h3><form>"
		"SSID:<br><input name=\"ssid\"><br>"
		"Contraseña:<br><input name=\"pass\" type=\"password\"><br>"
		"IPv4:<br><input name=\"ip\"><br>"
		"<input type=\"submit\" value=\"Guardar\">"
		"</form></body></html>";

const char respAT[] = "0302AT\r";
const char respATp[] = "0302AT+";
const char respOK[] = "0402OK\r\n";
const char respERROR[] = "0702ERROR\r\n";
const char respWIFIGOTIP[] = "1314WIFI GOT IP\r\n";
const char respWIFICONNECTED[] = "1602WIFI CONNECTED\r\n";
const char respWIFIDISCONNECT[] = "1702WIFI DISCONNECT\r\n";
const char respWIFIDISCONNECTED[] = "1902WIFI DISCONNECTED\r\n";
const char respDISCONNECTED[] = "1402DISCONNECTED\r\n";
const char respSENDOK[] = "0902SEND OK\r\n";
const char respCONNECT[] = "0902CONNECT\r\n";
const char respCLOSED[] = "0802CLOSED\r\n";
const char respCIFSRAPIP[] = "1205+CIFSR:STAIP";
const char respBUSY[] = "0602busy .";
const char respIPD[] = "0410+IPD";
const char respReady[] = "0702ready\r\n";
const char respBUSYP[] = "0602busy p";
const char respBUSYS[] = "0602busy s";
//const char respCIPSERVER[] = "0902CIPSERVER";
const char respSSID[] = "0520ssid=";
const char respPASS[] = "0521pass=";
const char respIP[] = "0322ip=";
const char respGETREQ[] = "1113:GET / HTTP"; //actualizar o entrar a la pagina web
//const char respSETREQ[] = "0914:GET /set";		//enviaron los datos del ssid, pass, etc
//const char respREQUEST[] = "1510CONNECT\r\n+IPD";
// 	  const char respCIFSRAPIP[] = "1102+CIFSR:APIP";
//    const char respCIFSRAPMAC[] = "1202+CIFSR:APMAC";
//    const char respCIFSRSTAIP[] = "1205+CIFSR:STAIP";
//    const char respCIFSRSTAMAC[] = "1302+CIFSR:STAMAC";

const char *const responses[] = {respAT, respATp, respOK, respERROR, respWIFIGOTIP, respWIFICONNECTED,
								 respWIFIDISCONNECT, respWIFIDISCONNECTED, respDISCONNECTED, respSENDOK, respCONNECT, respCLOSED,
								 respCIFSRAPIP, respBUSY, respIPD, respReady, respBUSYP, respBUSYS,
								 respSSID, respPASS, respIP, respGETREQ, NULL};

static uint8_t indexResponse = 0;
static uint8_t indexResponseChar = 0;

_emode mode;
uint8_t esp01userConnected = 0;
static char esp01Link;
static uint8_t isValid;

static char ssid_buffer[MAX_SSID_LEN];
static char pass_buffer[MAX_PASS_LEN];
static char ip_buffer[MAX_IP_LEN];
static uint8_t ssid_idx = 0, pass_idx = 0, ip_idx = 0;

//const char _DNSFAIL[] = "DNS FAIL\r";
//const char _ATCIPDNS[] = "AT+CIPDNS_CUR=1,\"208.67.220.220\",\"8.8.8.8\"\r\n";
//const char CIFSRAPIP[] = "+CIFSR:APIP\r";
//const char CIFSRAPMAC[] = "+CIFSR:APMAC\r";
//const char CIFSRSTAIP[] = "+CIFSR:STAIP\r";
//const char CIFSRSTAMAC[] = "+CIFSR:STAMAC\r";


void ESP01_SetWIFI(const char *ssid, const char *password){
	esp01ATState = ESP01ATIDLE;
	esp01Flags.byte = 0;

	strncpy(esp01SSID, ssid, 64);
	esp01SSID[63] = '\0';
	strncpy(esp01PASSWORD, password, 32);
	esp01PASSWORD[31] = '\0';

	esp01TimeoutTask = 50;
	//esp01ATState = ESP01ATHARDRST0;

	esp01TriesAT = 0;

}


_eESP01STATUS ESP01_StartUDP(const char *RemoteIP, uint16_t RemotePORT, uint16_t LocalPORT){
	if(esp01Handle.WriteUSARTByte == NULL)
		return ESP01_NOT_INIT;

	if(LocalPORT == 0)
		LocalPORT = 30000;

	strcpy(esp01PROTO, "UDP");

	strncpy(esp01RemoteIP, RemoteIP, 15);
	esp01RemoteIP[15] = '\0';

	itoa(RemotePORT, esp01RemotePORT, 10);
	itoa(LocalPORT, esp01LocalPORT, 10);

	if(esp01SSID[0] == '\0')
		return ESP01_WIFI_NOT_SETED;

	if(esp01Flags.bit.WIFICONNECTED == 0)
		return ESP01_WIFI_DISCONNECTED;

	//esp01ATState = ESP01ATCIPCLOSE;

	return ESP01_UDPTCP_CONNECTING;
}

_eESP01STATUS ESP01_StartTCP(const char *RemoteIP, uint16_t RemotePORT, uint16_t LocalPORT){
	if(esp01Handle.WriteUSARTByte == NULL)
		return ESP01_NOT_INIT;

	if(LocalPORT == 0)
		LocalPORT = 30000;

	strcpy(esp01PROTO, "TCP");

	strncpy(esp01RemoteIP, RemoteIP, 15);
	esp01RemoteIP[15] = '\0';

	itoa(RemotePORT, esp01RemotePORT, 10);
	itoa(LocalPORT, esp01LocalPORT, 10);

	if(esp01SSID[0] == '\0')
		return ESP01_WIFI_NOT_SETED;

	if(esp01Flags.bit.WIFICONNECTED == 0)
		return ESP01_WIFI_DISCONNECTED;

	esp01ATState = ESP01ATCIPCLOSE;

	return ESP01_UDPTCP_CONNECTING;
}


void ESP01_CloseUDPTCP(){
	if(esp01Handle.WriteUSARTByte == NULL)
		return;

	esp01ATState = ESP01ATCIPCLOSE;
}

_eESP01STATUS ESP01_StateWIFI(){
	if(esp01Handle.WriteUSARTByte == NULL)
		return ESP01_NOT_INIT;

	if(esp01Flags.bit.WIFICONNECTED)
		return ESP01_WIFI_CONNECTED;
	else
		return ESP01_WIFI_DISCONNECTED;
}

char *ESP01_GetLocalIP(){
	if(esp01Flags.bit.WIFICONNECTED &&  esp01LocalIP[0]!='\0')
		return esp01LocalIP;

	return NULL;
}


_eESP01STATUS ESP01_StateUDPTCP(){
	if(esp01Handle.WriteUSARTByte == NULL)
		return ESP01_NOT_INIT;

	if(esp01Flags.bit.UDPTCPCONNECTED)
		return ESP01_UDPTCP_CONNECTED;
	else
		return ESP01_UDPTCP_DISCONNECTED;
}


void ESP01_WriteRX(uint8_t value){
//	if(esp01Handle.bufRX == NULL)
//		return;
	esp01RXATBuf[esp01iwRXAT++] = value;
	if(esp01iwRXAT == ESP01RXBUFAT)
		esp01iwRXAT = 0;
}

_eESP01STATUS ESP01_Send(uint8_t *buf, uint16_t irRingBuf, uint16_t length, uint16_t sizeRingBuf){
	if(esp01Handle.WriteUSARTByte == NULL)
		return ESP01_NOT_INIT;

	if(esp01Flags.bit.UDPTCPCONNECTED == 0)
		return ESP01_UDPTCP_DISCONNECTED;

	if(esp01Flags.bit.SENDINGDATA == 0){
		char strInt[10];
		uint8_t l = 0;

		itoa(length, strInt, 10);
		l = strlen(strInt);
		if(l>4 || l==0)
			return ESP01_SEND_ERROR;
		ESP01StrToBufTX(ATCIPSEND);
		ESP01StrToBufTX(strInt);
		ESP01StrToBufTX("\r>");
		for(uint16_t i=0; i<length; i++){
			esp01TXATBuf[esp01iwTX++] = buf[irRingBuf++];
			if(esp01iwTX == ESP01TXBUFAT)
				esp01iwTX = 0;
			if(irRingBuf == sizeRingBuf)
				irRingBuf = 0;
		}
		esp01Flags.bit.TXCIPSEND = 1;
		esp01Flags.bit.SENDINGDATA = 1;
		if(ESP01DbgStr != NULL){
			ESP01DbgStr("+&DBGSENDING DATA ");
			ESP01DbgStr(strInt);
			ESP01DbgStr("\n");
		}
		return ESP01_SEND_READY;
	}
	if(ESP01DbgStr != NULL)
		ESP01DbgStr("+&DBGSENDING DATA BUSY\n");
	return ESP01_SEND_BUSY;
}


void ESP01_Init(_sESP01Handle *hESP01){

	memcpy(&esp01Handle, hESP01, sizeof(_sESP01Handle));

	esp01ATState = ESP01ATIDLE;
	esp01HState = 0;
	esp01irTX = 0;
	esp01iwTX = 0;
	esp01irRXAT = 0;
	esp01iwRXAT = 0;
	esp01Flags.byte = 0;
	ESP01ChangeState = NULL;
	ESP01DbgStr = NULL;
}


void ESP01_Timeout10ms(){
	if(esp01TimeoutTask)
		esp01TimeoutTask--;

	if(esp01TimeoutDataRx){
		esp01TimeoutDataRx--;
		if(!esp01TimeoutDataRx)
			esp01HState = 0;
	}

	if(esp01TimeoutTxSymbol)
		esp01TimeoutTxSymbol--;
}

void ESP01_Task(){
	if(esp01irRXAT != esp01iwRXAT)
		ESP01ATDecode();

	if(!esp01TimeoutTask)
		ESP01DOConnection();

	ESP01SENDData();
}

void ESP01_AttachChangeState(void (*aESP01ChangeState)(_eESP01STATUS esp01State)){
	ESP01ChangeState = aESP01ChangeState;
}

void ESP01_AttachDebugStr(void (*aESP01DbgStr)(const char *dbgStr)){
	ESP01DbgStr = aESP01DbgStr;
}

int ESP01_IsHDRRST(){
	if(esp01ATState==ESP01ATHARDRST0 || esp01ATState==ESP01ATHARDRST1 || esp01ATState==ESP01ATHARDRSTSTOP)
		return 1;
	return 0;
}

/* Private Functions */
static void ESP01ATDecode(){
	uint16_t i;
	uint8_t value;
	if(esp01ATState==ESP01ATHARDRST0 || esp01ATState==ESP01ATHARDRST1 ||
	   esp01ATState==ESP01ATHARDRSTSTOP){
		esp01irRXAT = esp01iwRXAT;
		return;
	}
	i = esp01iwRXAT;
	esp01TimeoutDataRx = 2;
	while(esp01irRXAT != i){
		value = esp01RXATBuf[esp01irRXAT];
		switch(esp01HState){
		case 0:
            indexResponse = 0;
            indexResponseChar = 4;
            while(responses[indexResponse] != NULL){
                if(value == responses[indexResponse][indexResponseChar]){ // todos los ocmandos de respuesta tienen una cabecer
                    esp01nBytes = (responses[indexResponse][0] - '0');			// aca saca los primeros dos numero que guarda la cantidad de bytes de la cabecera hace el *10 pq pone el primer byte en las decenas  y al otro lo suma así nomas
                    esp01nBytes *= 10;
                    esp01nBytes += (responses[indexResponse][1] - '0');
                    esp01nBytes--;
                    break;
                }
                indexResponse++;
            }
            if(responses[indexResponse] != NULL){
                esp01HState = 1;
                indexResponseChar++;
            }
			else{
				esp01TimeoutDataRx = 0;
				if(esp01Flags.bit.WAITINGSYMBOL){
					if(value == '>'){
						esp01Flags.bit.WAITINGSYMBOL = 0;
						esp01TimeoutTxSymbol = 0;
					}
				}
			}
			break;
		case 1:
            if(value == responses[indexResponse][indexResponseChar]){
                esp01nBytes--;
                if(!esp01nBytes || value=='\r'){
                    esp01HState = (responses[indexResponse][2] - '0');
                    esp01HState *= 10;
                    esp01HState += (responses[indexResponse][3] - '0');
                    break;
                }
            }
            else{
                indexResponse = 0;
                while(responses[indexResponse] != NULL){
                    esp01nBytes = (responses[indexResponse][0] - '0');
                    esp01nBytes *= 10;
                    esp01nBytes += (responses[indexResponse][1] - '0');
                    esp01nBytes -= (indexResponseChar-3);
                    if(esp01nBytes<128 && value==responses[indexResponse][indexResponseChar]){
                        if(esp01nBytes == 0){
                            esp01HState = (responses[indexResponse][2] - '0');
                            esp01HState *= 10;
                            esp01HState += (responses[indexResponse][3] - '0');
                        }
                        break;
                    }
                    indexResponse++;
                }
                if(responses[indexResponse] == NULL){
                    esp01HState = 0;
                    esp01irRXAT--;
                    break;
                }
            }
			indexResponseChar++;
			break;
		case 2:
			if(value == '\n'){
				esp01HState = 0;
				switch(indexResponse){
				case 0://AT
				case 1:
					break;
				case 2://OK
					if(esp01ATState == ESP01ATRESPONSE){
						esp01TimeoutTask = 0;
						esp01Flags.bit.ATRESPONSEOK = 1;
					}
					break;
				case 3://ERROR
					if(esp01Flags.bit.SENDINGDATA){
						esp01Flags.bit.SENDINGDATA = 0;
						esp01Flags.bit.UDPTCPCONNECTED = 0;
						esp01irTX = esp01iwTX;
					}
					break;
				case 4://WIFI GOT IP
					/*esp01TimeoutTask = 0;
					if(esp01ATState == ESP01CWJAPRESPONSE)
						esp01Flags.bit.ATRESPONSEOK = 1;
					esp01Flags.bit.WIFICONNECTED = 1;
					if(ESP01ChangeState != NULL)
						ESP01ChangeState(ESP01_WIFI_CONNECTED);
					if(ESP01DbgStr != NULL)
						ESP01DbgStr("+&DBGESP01GOTIP");*/
					break;
				case 5://WIFI CONNECTED
					break;
				case 6://WIFI DISCONNECT
				case 7://WIFI DISCONNECTED
					esp01Flags.bit.UDPTCPCONNECTED = 0;
					esp01Flags.bit.WIFICONNECTED = 0;
					if(ESP01ChangeState != NULL)
						ESP01ChangeState(ESP01_WIFI_DISCONNECTED);
					if(esp01ATState == ESP01CWJAPRESPONSE)
						break;
					esp01ATState = ESP01ATHARDRSTSTOP;
					break;
				case 8://DISCONNECTED
					esp01Flags.bit.UDPTCPCONNECTED = 0;
					break;
				case 9://SEND OK
					esp01Flags.bit.SENDINGDATA = 0;
					if(ESP01ChangeState != NULL)
						ESP01ChangeState(ESP01_SEND_OK);
					break;
				case 10://CONNECT
					if(mode == CONNECTWIFI){
						esp01TimeoutTask = 0;
						esp01Flags.bit.ATRESPONSEOK = 1;
						esp01Flags.bit.UDPTCPCONNECTED = 1;
						if(ESP01DbgStr != NULL)
								ESP01DbgStr("+&UDPTCPCONNECTED=1\n");
						if(ESP01ChangeState != NULL)
							ESP01ChangeState(ESP01_UDPTCP_CONNECTED);
					}
					break;
				case 11://CLOSED
					esp01Flags.bit.UDPTCPCONNECTED = 0;
					break;
				case 13://busy
					esp01Flags.bit.UDPTCPCONNECTED = 0;
					esp01Flags.bit.WIFICONNECTED = 0;
					break;
				case 15://ready
					esp01Flags.bit.UDPTCPCONNECTED = 0;
					esp01Flags.bit.WIFICONNECTED = 0;
					esp01ATState = ESP01ATHARDRSTSTOP;
					break;
				case 16://busy p
					break;
				case 17://busy s
					break;
				}
			}
			break;
		case 5://CIFR,STAIP
			if(value == ','){
				esp01HState = 6;
				if(ESP01DbgStr != NULL)
					ESP01DbgStr("+&DBGRESPONSE CIFSR\n");
			}
			else{
				esp01HState = 0;
				esp01irRXAT--;
				if(ESP01DbgStr != NULL)
					ESP01DbgStr("+&DBGERROR CIFSR 5\n");
			}
			break;
		case 6:
			if(value == '\"'){
				esp01HState = 7;
				esp01nBytes = 0;
			}
			break;
		case 7:
			if(value == '\"' || esp01nBytes==16)
				esp01HState = 8;
			else
				esp01LocalIP[esp01nBytes++] = value;
			break;
		case 8:
			if(value == '\n'){
				esp01HState = 0;
				if(esp01nBytes < 16){
					esp01LocalIP[esp01nBytes] = '\0';
					esp01Flags.bit.ATRESPONSEOK = 1;
					esp01TimeoutTask = 0;
				}
				else
					esp01LocalIP[0] = '\0';
				if(ESP01ChangeState != NULL)
					ESP01ChangeState(ESP01_WIFI_NEW_IP);
			}
			break;
		case 10://IPD
			if(value == ','){
				esp01HState = 11;
				esp01nBytes = 0;
			}
			else{
				esp01HState = 0;
				esp01irRXAT--;
			}
			break;
		case 11:
			if(value == ':')
				esp01HState = 12;
			else{
				if(value<'0' || value>'9'){
					esp01HState = 0;
					esp01irRXAT--;
				}else{
					/*if(esp01ATState == ESP01_WAITING_CONNECTION){ //,CONNECT\r\n+IPD,<LINK>
						esp01userConnected = 1;
						esp01Link = value;
						if(ESP01DbgStr != NULL)
							ESP01DbgStr("+&DBGUSERREQUEST");
					}else{*/

						esp01nBytes *= 10;
						esp01nBytes += (value - '0');
					//}
				}
			}
			break;
		case 12:
			if(esp01Handle.WriteByteToBufRX != NULL)
				esp01Handle.WriteByteToBufRX(value);
			esp01nBytes--;
			if(!esp01nBytes){
				esp01HState = 0;
				if(ESP01DbgStr != NULL)
					ESP01DbgStr("+&DBGRESPONSE IPD\n");
			}
			break;
		case 13:
			if(esp01ATState == ESP01_WAITING_CONNECTION){
				esp01userConnected = 1;
				esp01Link = esp01RXATBuf[esp01irRXAT-16];
				if(ESP01DbgStr != NULL){
					ESP01DbgStr("+&DBGLINKED");
					ESP01DbgStr(&esp01Link);
				}
			}
			if(esp01ATState == ESP01ATWAITSERVERDATA){
				esp01userConnected = 1;
				esp01Link = esp01RXATBuf[esp01irRXAT-16];
				if(ESP01DbgStr != NULL){
					ESP01DbgStr("+&DBGLINKED");
					ESP01DbgStr(&esp01Link);
				}
				esp01ATState = ESP01_WAITING_CONNECTION;
			}
			esp01HState = 0;
			break;
		case 14:
			esp01TimeoutTask = 0;
			if(esp01ATState == ESP01CWJAPRESPONSE)
				esp01Flags.bit.ATRESPONSEOK = 1;
			esp01Flags.bit.WIFICONNECTED = 1;
			if(ESP01ChangeState != NULL)
				ESP01ChangeState(ESP01_WIFI_CONNECTED);
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01GOTIP");
			break;
		case 20: //SSID
			if (value == '&' || value == '\r' || ssid_idx >= MAX_SSID_LEN-1) {
				// fin de SSID
				ssid_buffer[ssid_idx] = '\0';
				ssid_idx = 0;
				esp01HState = 0;          // volvemos a buscar respuestas
				esp01irRXAT--;
				if(ESP01DbgStr != NULL){
					ESP01DbgStr("SSID:");
					ESP01DbgStr(ssid_buffer);
				}
			} else {
				ssid_buffer[ssid_idx++] = value;
			}
			break;
		case 21: //PASS
			if (value == '&' || value == '\r' || pass_idx >= MAX_PASS_LEN-1) {
				// fin de PASS
				pass_buffer[pass_idx] = '\0';
				pass_idx = 0;
				esp01HState = 0;          // volvemos a buscar respuestas
				esp01irRXAT--;
				if(ESP01DbgStr != NULL){
					ESP01DbgStr("PASS:");
					ESP01DbgStr(pass_buffer);
				}
				//LLAMAR A UN CONTROLADOR DE CONTRASEÑAS Y SSID
				//ESP01_Check_Credentials(&ssid_buffer, pass_buffer);

			} else {
				pass_buffer[pass_idx++] = value;
			}
			break;
		case 22:
			if (value == '&' || value == ' ' || value == '\r' || pass_idx >= MAX_IP_LEN-1) {
				// fin de PASS
				ip_buffer[ip_idx] = '\0';
				ip_idx = 0;
				esp01HState = 0;          // volvemos a buscar respuestas
				esp01irRXAT--;
				if(ESP01DbgStr != NULL){
					ESP01DbgStr("IP:");
					ESP01DbgStr(ip_buffer);
				}
				//LLAMAR A UN CONTROLADOR DE CONTRASEÑAS Y SSID
				isValid = ESP01_Check_Credentials(ssid_buffer, pass_buffer, ip_buffer);

				if(isValid){
					if(ESP01DbgStr != NULL)
						ESP01DbgStr("+&DBG_CRED_VALIDS");
				}else{
					if(ESP01DbgStr != NULL)
						ESP01DbgStr("+&DBG_CRED_NO_VALIDS");
				}
			} else {
				ip_buffer[ip_idx++] = value;
			}
			break;
		default:
			esp01HState = 0;
			esp01TimeoutDataRx = 0;
		}

		esp01irRXAT++;
		if(esp01irRXAT == ESP01RXBUFAT)
			esp01irRXAT = 0;
	}
}

_emode ESP01_GETMODE(){
	return mode;
}

static void ESP01DOConnection(){
	esp01TimeoutTask = 100;
	switch(esp01ATState){
	case ESP01ATIDLE:
		esp01TimeoutTask = 500;
		esp01ATState = ESP01ATWAITINGIP;
		break;
	case ESP01ATWAITINGIP:
		if(esp01Flags.bit.WIFICONNECTED == 1){
#ifdef DEMOSTRACION_REGULARIZACION
			esp01ATState = ESP01ATHARDRST0;
			mode = CREATEWIFI;
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01_DEMOSTR");
#else
			esp01ATState = ESP01ATCONNECTED;
			mode = CONNECTWIFI;
			if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01_RED_ENCONTRADA");
#endif
		}else{
			esp01ATState = ESP01ATHARDRST0;
			mode = CREATEWIFI;
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01_NO_RED");
		}
		break;
	case ESP01ATHARDRST0:
		esp01Handle.DoCHPD(0);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01HARDRESET0\n");
		esp01ATState = ESP01ATHARDRST1;
		break;
	case ESP01ATHARDRST1:
		esp01Handle.DoCHPD(1);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01HARDRESET1\n");
		esp01ATState = ESP01ATHARDRSTSTOP;
		esp01TimeoutTask = 500;
		break;
	case ESP01ATHARDRSTSTOP:
		esp01ATState = ESP01ATAT;
		esp01TriesAT = 0;
		break;
	case ESP01ATAT:
		if(esp01TriesAT){
			esp01TriesAT--;
			if(!esp01TriesAT){
				esp01ATState = ESP01ATHARDRST0;
				break;
			}
		}
		else
			esp01TriesAT = 4;

		esp01Flags.bit.ATRESPONSEOK = 0;
		ESP01StrToBufTX(ATAT);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01AT\n");
		esp01ATState = ESP01ATRESPONSE;
		break;
	case ESP01ATRESPONSE:
		if(esp01Flags.bit.ATRESPONSEOK)
			esp01ATState = ESP01ATCWMODE;
		else
			esp01ATState = ESP01ATAT;
		break;
	case ESP01ATCWMODE:
		/* acá separamos de station a soft ap*/
		if(mode == CONNECTWIFI){
			esp01ATState = ESP01ATCIPCLOSESERVER;
			/*ESP01StrToBufTX(ATCWMODE);
			ESP01StrToBufTX("1\r\n");
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01ATCWMODE1\n");*/
		}else{
			esp01ATState = ESP01ATCIPMUX;
			/*ESP01StrToBufTX(ATCWMODE);
			ESP01StrToBufTX("2\r\n");
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01ATCWMODE2\n");*/
		}
		ESP01StrToBufTX(ATCWMODE);
		ESP01StrToBufTX("3\r\n");
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01ATCWMODE2\n");
		break;
	case ESP01ATCIPCLOSESERVER:
		ESP01StrToBufTX(ATCIPCLOSESERVER);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGATCLOSESERVER\n");
		esp01ATState = ESP01ATCIPMUX;
		break;
	case ESP01ATCIPMUX:
		ESP01StrToBufTX(ATCIPMUX);

		if(mode == CONNECTWIFI){
			ESP01StrToBufTX("0\r\n");
			esp01ATState = ESP01ATCWJAP;
		}else{
			ESP01StrToBufTX("1\r\n");
			esp01ATState = ESP01ATCWQAP;
		}


		//esp01ATState = ESP01ATCWJAP;//COMENTAR

		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01ATCIPMUX\n");
		break;
	case ESP01ATCWJAP:
		if(esp01Flags.bit.WIFICONNECTED){
			esp01ATState = ESP01ATCIFSR;
			break;
		}
		if(esp01SSID[0] == '\0')
			break;
		ESP01StrToBufTX(ATCWJAP);
		ESP01ByteToBufTX('\"');
		ESP01StrToBufTX(esp01SSID);
		ESP01ByteToBufTX('\"');
		ESP01ByteToBufTX(',');
		ESP01ByteToBufTX('\"');
		ESP01StrToBufTX(esp01PASSWORD);
		ESP01ByteToBufTX('\"');
		ESP01ByteToBufTX('\r');
		ESP01ByteToBufTX('\n');
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01ATCWJAP");
		esp01Flags.bit.ATRESPONSEOK = 0;
		esp01ATState = ESP01CWJAPRESPONSE;
		esp01TimeoutTask = 500;
		break;
	case ESP01CWJAPRESPONSE:
		if(esp01Flags.bit.ATRESPONSEOK){
			esp01ATState = ESP01ATCIFSR;
			esp01TriesAT = 4;
		}
		else
			esp01ATState = ESP01ATAT;
		break;
	case ESP01ATCIFSR:
		esp01LocalIP[0] = '\0';
		ESP01StrToBufTX(ATCIFSR);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01CIFSR");
		esp01Flags.bit.ATRESPONSEOK = 0;
		esp01ATState = ESP01CIFSRRESPONSE;
		break;
	case ESP01CIFSRRESPONSE:
		if(esp01Flags.bit.ATRESPONSEOK)
			esp01ATState = ESP01ATCIPCLOSE;
		else{
			esp01TriesAT--;
			if(esp01TriesAT == 0){
				esp01ATState = ESP01ATAT;
				break;
			}
			esp01ATState = ESP01ATCIFSR;
		}
		break;
	case ESP01ATCIPCLOSE:
		if(esp01RemoteIP[0] == '\0')
			break;
		ESP01StrToBufTX(ATCIPCLOSE);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01ATCIPCLOSE");
		esp01ATState = ESP01ATCIPSTART;
		break;
	case ESP01ATCIPSTART:
		ESP01StrToBufTX(ATCIPSTART);
		ESP01ByteToBufTX('\"');
		ESP01StrToBufTX(esp01PROTO);
		ESP01ByteToBufTX('\"');
		ESP01ByteToBufTX(',');
		ESP01ByteToBufTX('\"');
		ESP01StrToBufTX(esp01RemoteIP);
		ESP01ByteToBufTX('\"');
		ESP01ByteToBufTX(',');
		ESP01StrToBufTX(esp01RemotePORT);
		ESP01ByteToBufTX(',');
		ESP01StrToBufTX(esp01LocalPORT);
		ESP01ByteToBufTX(',');
		ESP01ByteToBufTX('0');
		ESP01ByteToBufTX('\r');
		ESP01ByteToBufTX('\n');
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01ATCIPSTART");
		esp01Flags.bit.ATRESPONSEOK = 0;
		esp01Flags.bit.UDPTCPCONNECTED = 0;
		esp01ATState = ESP01CIPSTARTRESPONSE;
		esp01TimeoutTask = 500;
		break;
	case ESP01CIPSTARTRESPONSE:
		if(esp01Flags.bit.ATRESPONSEOK){
			esp01ATState = ESP01ATCONNECTED;
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01ATCONNECTED");
		}else
			esp01ATState = ESP01ATAT;
		break;
	case ESP01ATCONNECTED:
		if(esp01Flags.bit.WIFICONNECTED == 0){
			esp01ATState = ESP01ATAT;
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01desconec");
			break;
		}
		if(esp01Flags.bit.UDPTCPCONNECTED == 0){
			esp01ATState = ESP01ATCIPCLOSE;
			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01noudp");
			break;
		}
		esp01TimeoutTask = 0;
		break;
	/*********** SOFT AP STATES **********/
	case ESP01ATCWQAP:
		ESP01StrToBufTX(ATCWQAP);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01CWQAP");
		esp01ATState = ESP01ATCWSAP;
		esp01TimeoutTask = 20;

		break;
	case ESP01ATCWSAP:
		ESP01StrToBufTX(ATCWSAP);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01CWSAP");
		esp01ATState = ESP01ATCWDHCP;
		esp01TimeoutTask = 100;
		esp01userConnected = 0;
		esp01Flags.bit.ATRESPONSEOK = 0;
		//esp01Flags.bit.ATRESPONSEOK=0;
		//esp01TimeoutTask = 4000;
		break;
	case ESP01ATCWSAP_RESPONSE:
		if(esp01Flags.bit.ATRESPONSEOK)
			esp01ATState = ESP01ATCWDHCP;
		else
			esp01ATState = ESP01ATAT;
		break;
	case ESP01ATCWDHCP:
		ESP01StrToBufTX(ATCWDHCP);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01CWDHCP");
		esp01ATState = ESP01ATCIPSERVER;

		esp01TimeoutTask = 100;
		break;

	case ESP01ATCIPSERVER:
		ESP01StrToBufTX(ATCIPOPENSERVER);
		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01CIPSERVER");
		esp01ATState = ESP01_WAITING_CONNECTION;
		esp01Flags.bit.ATRESPONSEOK = 0;
		esp01TimeoutTask = 300;
		break;
	case ESP01_WAITING_CONNECTION:
		if(esp01userConnected){
			esp01ATState = ESP01ATCONFIGSERVER;
			esp01userConnected = 0;
		}
		break;
	case ESP01ATCIPSERVER_RESPONSE:
		if(esp01Flags.bit.ATRESPONSEOK)
			esp01ATState = ESP01ATCONFIGSERVER;
		else
			esp01ATState = ESP01ATAT;
	break;
	case ESP01ATCONFIGSERVER:
		ESP01StrToBufTX(ATCIPSEND);
		ESP01ByteToBufTX(esp01Link);
		ESP01StrToBufTX(",300");
		ESP01StrToBufTX("\r>");

		esp01Flags.bit.TXCIPSEND = 1;
		esp01Flags.bit.SENDINGDATA = 1;

		if(ESP01DbgStr != NULL)
			ESP01DbgStr("+&DBGESP01ATESP01ATCONFIGSERVER");

		esp01ATState = ESP01ATCONFIGSERVER_RESPONSE;
		esp01Flags.bit.ATRESPONSEOK = 0;
		//esp01TimeoutTask = 200;
	break;
	case ESP01ATCONFIGSERVER_RESPONSE:
		if(!esp01Flags.bit.WAITINGSYMBOL){
			ESP01StrToBufTX(CONFIGSERVER);

			esp01TimeoutTask = 200;

			esp01ATState = ESP01ATWAITSERVERDATA;

			if(ESP01DbgStr != NULL)
				ESP01DbgStr("+&DBGESP01ATWAITSERVERDATA");
		}
	case ESP01ATWAITSERVERDATA:
		if(isValid){
			ESP01_setMode(CONNECTWIFI);
			ESP01_SetWIFI(ssid_buffer, pass_buffer);
			ESP01_StartUDP(ip_buffer, 30010, 30000);
			esp01ATState = ESP01ATHARDRST0;
			isValid = 0;
		}
	break;
	}
}

void ESP01_setMode(_emode _mode){
	mode = _mode;
}

static void ESP01SENDData(){
	uint8_t value;
	if(esp01Flags.bit.WAITINGSYMBOL){
		if(!esp01TimeoutTxSymbol){
			esp01irTX = esp01iwTX;
			esp01Flags.bit.WAITINGSYMBOL = 0;
			esp01ATState = ESP01ATAT;
			esp01TimeoutTask = 10;
		}
		return;
	}
	if(esp01irTX != esp01iwTX){
		value = esp01TXATBuf[esp01irTX];
		if(esp01Flags.bit.TXCIPSEND){
			if(value == '>')
				value = '\n';
		}
		if(esp01Handle.WriteUSARTByte(value)){
			if(esp01Flags.bit.TXCIPSEND){
				if(esp01TXATBuf[esp01irTX] == '>'){
					esp01Flags.bit.TXCIPSEND = 0;
					esp01Flags.bit.WAITINGSYMBOL = 1;
					esp01TimeoutTxSymbol = 5;
				}
			}
			esp01irTX++;
			if(esp01irTX == ESP01TXBUFAT)
				esp01irTX = 0;
		}
	}
}

static void ESP01StrToBufTX(const char *str){
	for(int i=0; str[i]; i++){
		esp01TXATBuf[esp01iwTX++] = str[i];
		if(esp01iwTX == ESP01TXBUFAT)
			esp01iwTX = 0;
	}
}

static void ESP01ByteToBufTX(uint8_t value){
	esp01TXATBuf[esp01iwTX++] = value;
	if(esp01iwTX == ESP01TXBUFAT)
		esp01iwTX = 0;
}

static int is_valid_ip(const char* ip) {
    if (ip == NULL) return 0;

    // Hacemos una copia local modificable para strtok
    char ip_copy[16];
    strncpy(ip_copy, ip, sizeof(ip_copy) - 1);
    ip_copy[sizeof(ip_copy) - 1] = '\0';  // Garantizar terminación

    int dots = 0;
    char *ptr = strtok(ip_copy, ".");

    while (ptr != NULL) {
        // Verificar que solo haya dígitos
        for (size_t i = 0; ptr[i]; ++i) {
            if (!isdigit((unsigned char)ptr[i])) return 0;
        }

        // Convertir a número y verificar rango
        long num = strtol(ptr, NULL, 10);
        if (num < 0 || num > 255) return 0;

        // Prevenir ceros a la izquierda (ej: "01" o "001")
        if (ptr[0] == '0' && strlen(ptr) > 1) return 0;

        ptr = strtok(NULL, ".");
        dots++;
    }

    return (dots == 4);
}

static uint8_t ESP01_Check_Credentials(const char* ssid, const char* password, const char* ip) {
    // Validar SSID
    if (ssid == NULL) return 0;
    size_t ssid_len = strlen(ssid);
    if (ssid_len < 1 || ssid_len > 32) return 0;

    // Validar contraseña (WPA-PSK requiere entre 8 y 63 caracteres)
    if (password == NULL) return 0;
    size_t pass_len = strlen(password);
    if (pass_len != 0 && (pass_len < 8 || pass_len > 63)) return 0;

    // Validar IP
    if (ip == NULL || !is_valid_ip(ip)) return 0;

    return 1;
}
/* END Private Functions*/



