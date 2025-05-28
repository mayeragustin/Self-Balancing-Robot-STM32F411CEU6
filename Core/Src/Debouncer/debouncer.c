/*
 * debouncer.c
 *
 *  Created on: May 13, 2025
 *      Author: Agustín Alejandro Mayer
 */

#include "Debouncer/debouncer.h"
#include <stdlib.h>
#include "utilities.h"

s_Input debouncerBuff[MAX_INPUTS_DEBOUNCED];
uint8_t	inputIndex = 0;

void inputState(s_Input *stateInput){
	switch(stateInput->state){
		case UP:
			if(stateInput->value == DOWN)
				stateInput->state = FALLING;
		break;
		case DOWN:
			if(stateInput->value == UP)
				stateInput->state = RISING;
		break;
		case RISING:
			if(stateInput->value == UP){
				stateInput->state = UP;
				/*------------------------------*/
				stateInput->val = UP;
				if(stateInput->stateChanged != NULL)
					stateInput->stateChanged(RISING);
				/*------------------------------*/
			}else{
				stateInput->state = DOWN;
			}
		break;
		case FALLING:
			if(stateInput->value == DOWN){
				stateInput->state = DOWN;
				/*------------------------------*/
				stateInput->val = DOWN;
				if(stateInput->stateChanged != NULL)
					stateInput->stateChanged(FALLING);
				/*------------------------------*/
			}else{
				stateInput->state = UP;
			}
		break;
		default:
		stateInput->state = UP;
	}
}

void Debounce_Init(){
	for(inputIndex=0; inputIndex<MAX_INPUTS_DEBOUNCED; inputIndex++){
		debouncerBuff[inputIndex].getInputState = NULL;
		debouncerBuff[inputIndex].stateChanged = NULL;
	}
	inputIndex = 0;
}

uint8_t Debounce_Add(uint8_t (*AbstHard)(), void (*STATECHANGED)(e_Estados estado)){
	if(inputIndex >= MAX_INPUTS_DEBOUNCED)
		return 0;
	debouncerBuff[inputIndex].getInputState = AbstHard;
	debouncerBuff[inputIndex].stateChanged = STATECHANGED;
	return inputIndex++;
}

void Debouncer_Task(){
	for(uint8_t i = 0; i < MAX_INPUTS_DEBOUNCED; i++){
		debouncerBuff[i].value = debouncerBuff[i].getInputState();
		inputState(&debouncerBuff[i]);
	}
}

uint8_t getInputState(uint8_t input){
	return debouncerBuff[input].val;
}
