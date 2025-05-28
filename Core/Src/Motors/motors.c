/*
 * motors.c
 *
 *  Created on: May 17, 2025
 *      Author: Agustín Alejandro Mayer
 */
#include "Motors/motors.h"
#include <stddef.h>

void Motor_Init(s_motor *motor, void (*PWM_set)(uint16_t dCycle),
		void (*PIN_set)(e_direction dir), uint16_t max_value){

	motor->direction = NO_INIT;
	motor->setPins = PIN_set;
	motor->setPWM = PWM_set;
	motor->maxValue = max_value;
	motor->vel = 0;
	motor->brakeTimeout = 0;
}

void Motor_Set_16_Speed(s_motor *motor, int32_t speed){ //ARREGLAME FLACO PORFIS :)
	if(motor->setPins == NULL || motor->setPWM == NULL)
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
		motor->setPins(FORWARD);
		motor->setPWM((uint16_t)motor->vel);
	}else if(speed < 0){
		motor->direction = FORWARD;
		motor->setPins(BACKWARD);
		motor->setPWM((uint16_t)(motor->vel * -1));
	}else{
		motor->direction = FREE_WHEEL;
		motor->setPins(FREE_WHEEL);
		motor->setPWM(0);
	}
}

void Motor_Set_PER_Speed(s_motor *motor, int8_t speed){
	if(motor->setPins == NULL || motor->setPWM == NULL)
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
		motor->setPins(FORWARD);
		motor->setPWM((uint16_t)motor->vel-1);
	}else if(speed < 0){
		motor->direction = BACKWARD;
		motor->setPins(BACKWARD);
		motor->setPWM((uint16_t)(motor->vel * -1)-1);
	}else{
		motor->direction = FREE_WHEEL;
		motor->setPins(FREE_WHEEL);
		motor->setPWM(0);
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
	motor->setPins(BRAKE);

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
