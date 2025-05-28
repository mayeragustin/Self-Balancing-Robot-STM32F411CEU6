/*
 * communication_handler.c
 *
 *  Created on: Apr 27, 2025
 *      Author: Agustín Alejandro Mayer
 */

#include "Protocol_Handler/protocol_handler.h"
#include <string.h>
//#include "Communication/ESP01.h"

static uint8_t i = 0;
static uint8_t auxIndex = 0;
static uint8_t TxAuxBuffer[30];
static uint8_t indexStart;
static uint8_t indexStartValue;
static uint8_t checksum;

void Comm_Init(s_commData* comm, void (*dataD)(s_commData *comm), void (*dataW)(s_commData *comm)){
	comm->dataDecoder = dataD;
	comm->dataWriter = dataW;
	comm->timeOut = 0;
	comm->indexStart = 0;
	comm->checksumRx = 0;
	comm->Tx.write=0;
	comm->Tx.read=0;
	comm->Rx.write=0;
	comm->Rx.read=0;
	comm->isESP01 = 0;
	comm->protocolState = START;
}

void Comm_Task(s_commData* comm){
	if(comm->Rx.read != comm->Rx.write){
		decodeProtocol(comm);
	}
	if (comm->Tx.read != comm->Tx.write) {
		if(!comm->isESP01){
			if(comm->dataWriter != NULL)
				comm->dataWriter(comm);
		}else{
			//ESP01_Send((unsigned char*)&comm->Tx.buffer,  comm->Tx.read,  1,  RINGBUFFLENGTH);
		}
	}
}

void decodeProtocol(s_commData *datosCom){
	static uint8_t nBytes=0;
	uint8_t indexWriteRxCopy = datosCom->Rx.write;

	while (datosCom->Rx.read != indexWriteRxCopy){
		switch(datosCom->protocolState){
		case START:
			if (datosCom->Rx.buffer[datosCom->Rx.read++] == 'U'){
				datosCom->protocolState = HEADER_1;
				datosCom->checksumRx = 0;
			}
			break;
		case HEADER_1:
			if (datosCom->Rx.buffer[datosCom->Rx.read++] == 'N')
				datosCom->protocolState = HEADER_2;
			else{
				datosCom->Rx.read--;
				datosCom->protocolState = START;
			}
			break;
		case HEADER_2:
			if (datosCom->Rx.buffer[datosCom->Rx.read++] == 'E')
				datosCom->protocolState = HEADER_3;
			else{
				datosCom->Rx.read--;
				datosCom->protocolState = START;
			}
			break;
		case HEADER_3:
			if (datosCom->Rx.buffer[datosCom->Rx.read++] == 'R')
				datosCom->protocolState = NBYTES;
			else{
				datosCom->Rx.read--;
				datosCom->protocolState = START;
			}
			break;
		case NBYTES:
			datosCom->indexStart = datosCom->Rx.read;
			nBytes = datosCom->Rx.buffer[datosCom->Rx.read++];
			datosCom->protocolState = TOKEN;
			break;
		case TOKEN:
			if (datosCom->Rx.buffer[datosCom->Rx.read++] == ':'){
				datosCom->protocolState = PAYLOAD;
				datosCom->checksumRx ='U' ^ 'N' ^ 'E' ^ 'R' ^ nBytes ^ ':';
			}
			else{
				datosCom->Rx.read--;
				datosCom->protocolState = START;
			}
			break;
		case PAYLOAD:
			if (nBytes > 1){
				datosCom->checksumRx ^= datosCom->Rx.buffer[datosCom->Rx.read++];
			}
			nBytes--;
			if(nBytes<=0){
				datosCom->protocolState = START;
				if(datosCom->checksumRx == datosCom->Rx.buffer[datosCom->Rx.read]){
					if(datosCom->dataDecoder != NULL)
						datosCom->dataDecoder(datosCom);
				}
			}
			break;
		default:
			datosCom->protocolState = START;
			break;
		}
	}
}

void comm_sendCMD(s_commData *datosCom, _eID cmd, uint8_t *str, uint8_t len){
    i = 0;
    auxIndex = 0;

    // Cabecera
    TxAuxBuffer[auxIndex++] = 'U'; // Start
    TxAuxBuffer[auxIndex++] = 'N'; // Header 1
    TxAuxBuffer[auxIndex++] = 'E'; // Header 2
    TxAuxBuffer[auxIndex++] = 'R'; // Header 3

    indexStart = auxIndex; // Se guarda la posición del primer dato del mensaje (cmd)
    TxAuxBuffer[auxIndex++] = 0;   // Placeholder del indexStart, se sobrescribirá después

    TxAuxBuffer[auxIndex++] = ':'; // Token
    TxAuxBuffer[auxIndex++] = cmd; // Comando

    if(cmd == USERTEXT || cmd == SYSERROR)
    	TxAuxBuffer[auxIndex++] = len;

    // Copia de datos si hay
    if (str != NULL && len > 0) {
        memcpy(&TxAuxBuffer[auxIndex], str, len);
        auxIndex += len;
    }

    indexStartValue = auxIndex - indexStart - 1; // Cantidad de datos desde cmd hasta checksum
    TxAuxBuffer[indexStart] = indexStartValue;

    // Checksum
    checksum = 0;
    for (i = 0; i < auxIndex; i++) {
        checksum ^= TxAuxBuffer[i];
        datosCom->Tx.buffer[datosCom->Tx.write++] = TxAuxBuffer[i];
    }

    datosCom->Tx.buffer[datosCom->Tx.write++] = checksum;
}

