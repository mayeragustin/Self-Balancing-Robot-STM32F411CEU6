/*
 * encoder.c
 *
 *  Created on: May 17, 2025
 *      Author: Agustín Alejandro Mayer
 */

#include "Motors/encoder.h"

void Encoder_Init(s_encoder *enc, uint8_t reset){
	enc->pulses = 0;
	enc->resetBase = reset;
	enc->timeReset = reset;
	enc->counter1s = 1000 / reset;
}

void Encoder_Task(s_encoder *enc){
	enc->timeReset--;
	if(!enc->timeReset){

		enc->fastPPS += enc->pulses;

		enc->timeReset = enc->resetBase;
		enc->pulses = 0;
	}
	enc->counter1s--;
	if(!enc->counter1s){
		enc->pps = 0;
	}
}

void Encoder_Add_Pulse(s_encoder *enc){
	enc->pulses++;
}

void Encoder_1s_Elapsed(s_encoder *enc){
	enc->pps = enc->fastPPS;
	enc->fastPPS=0;
}
