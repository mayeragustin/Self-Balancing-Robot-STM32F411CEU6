/*
 * motors.c
 *
 *  Created on: May 17, 2025
 *      Author: Agustín Alejandro Mayer
 */
#include "Motors/motors.h"
#include <stddef.h>

void Motor_Init(s_motor *motor, void (*PWM_set)(uint16_t dCycle),
		void (*PIN_set)(uint8_t A, uint8_t B), uint16_t max_value){

	motor->direction = NO_INIT;
	motor->setPins = PIN_set;
	motor->setPWM = PWM_set;
	motor->maxValue = max_value;
	motor->vel = 0;
	motor->brakeTimeout = 0;
}

void Motor_Set_16_Speed(s_motor *motor, int32_t speed){ //ARREGLAME FLACO PORFIS :)
	if(motor->setPWM == NULL)
		return;
	if(speed > motor->maxValue)
		speed = motor->maxValue;
	if(speed < -motor->maxValue)
		speed = -motor->maxValue;
	if(speed == motor->vel)
		return;

	motor->vel = speed;

	if(speed > 0){
		motor->direction = FORWARD;
		Motor_Set_Direction(motor, FORWARD);
		motor->setPWM((uint16_t)motor->vel);
	}else if(speed < 0){
		motor->direction = FORWARD;
		Motor_Set_Direction(motor, BACKWARD);
		motor->setPWM((uint16_t)(motor->vel * -1));
	}else{
		motor->direction = FREE_WHEEL;
		Motor_Set_Direction(motor, FREE_WHEEL);
		motor->setPWM(0);
	}
}

void Motor_Set_PER_Speed(s_motor *motor, int8_t speed){
	if( motor->setPWM == NULL)
		return;
	if(speed > 100)
		speed = 100;
	if(speed < -100)
		speed = -100;
	if(speed == motor->vel)
		return;

	motor->vel = speed * 600;

	if(speed > 0){
		motor->direction = FORWARD;
		Motor_Set_Direction(motor, FORWARD);
		motor->setPWM((uint16_t)motor->vel-1);
	}else if(speed < 0){
		motor->direction = BACKWARD;
		Motor_Set_Direction(motor, BACKWARD);
		motor->setPWM((uint16_t)(motor->vel * -1)-1);
	}else{
		motor->direction = FREE_WHEEL;
		Motor_Set_Direction(motor, FREE_WHEEL);
		motor->setPWM(0);
	}
}

void Motor_Set_Direction(s_motor *motor, e_direction direction){
	if(motor->setPins == NULL)
		return;
	switch(direction){
		case NO_INIT:
			break;
		case FREE_WHEEL:
			motor->setPins(0, 0);
			break;
		case FORWARD:
			motor->setPins(1, 0);
			break;
		case BACKWARD:
			motor->setPins(0, 1);
			break;
		case BRAKE:
			motor->setPins(1, 1);
			break;
	}
}

void Motor_Set_Break_10ms(s_motor *motor, uint8_t timeout){
	if(motor->setPins == NULL)
		return;
	if(timeout > 150)
		motor->brakeTimeout = 150;
	else
		motor->brakeTimeout = timeout;

	motor->direction = BRAKE;
	Motor_Set_Direction(motor, BRAKE);

}

void Motor_Break_Timeout(s_motor *motor){
	if(motor->direction == BRAKE){
		motor->brakeTimeout--;
		if(!motor->brakeTimeout)
			motor->direction = FREE_WHEEL;
	}
}

void Motor_Set_MaxValue(s_motor *motor, uint32_t value){
	motor->maxValue = value;
}
