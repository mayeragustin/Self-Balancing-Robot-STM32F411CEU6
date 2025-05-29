/*
 * utilities.h
 *
 *  Created on: May 12, 2025
 *      Author: Agustín Alejandro Mayer
 */

#ifndef INC_UTILITIES_H_
#define INC_UTILITIES_H_

#include "stdint.h"

#define TRUE 	1
#define FALSE 	0

typedef enum{
	SYS_OK,
	SYS_ERROR,
	SYS_BUSY
}e_system;

typedef union{
	struct{
		uint8_t b0: 1;
		uint8_t b1: 1;
		uint8_t b2: 1;
		uint8_t b3: 1;
		uint8_t b4: 1;
		uint8_t b5: 1;
		uint8_t b6: 1;
		uint8_t b7: 1;
	}bit;
	struct{
		uint8_t L: 4;
		uint8_t H: 4;
	}nibble;
	uint8_t byte;
}u_flag;

typedef union{
	float f;
	int32_t i32;
	uint32_t ui32;
	int16_t i16[2];
	uint16_t ui16[2];
	int8_t i8[4];
	uint8_t ui8[4];
}u_conv;

#endif /* INC_UTILITIES_H_ */
