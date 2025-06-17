/*
 * ADC_handler.c
 *
 *  Created on: May 12, 2025
 *      Author: agust
 */
#include "ADC/ADC_handler.h"




//static const uint16_t LUT_For_Linealization[ADC_NUM_SENSORS][ADC_LUT_SIZE];

static uint16_t mediaBuffer[ADC_MEDIA_SIZE][ADC_NUM_SENSORS] = {0};
static uint16_t sumData[ADC_NUM_SENSORS]= {0};
static uint8_t index = 0;


void ADC_Conversion_Cplt(uint16_t *rawData, uint16_t *filtredData){
	for(uint8_t channel = 0; channel < ADC_NUM_SENSORS; channel++){
		sumData[channel] -= mediaBuffer[index][channel];
		sumData[channel] += rawData[channel];
		mediaBuffer[index][channel] = rawData[channel];
		filtredData[channel] = (sumData[channel] >> ADC_DESPLAZAMIENTOS);
	}
	index++;
	index &= (ADC_MEDIA_SIZE - 1);
}

void ADC_Linealization(){
 //mausque herramienta misteriosa que usaremos mas tarde!!

}
