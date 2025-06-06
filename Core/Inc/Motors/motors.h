/**
 * @file motors.h
 * @brief Módulo de control de motores con capacidad de velocidad y dirección.
 *
 * Este archivo define las funciones y estructuras necesarias para controlar motores
 * mediante señales PWM y pines de dirección, permitiendo el ajuste de velocidad,
 * el control de la dirección y la activación de frenado.
 *
 * @date 17 de mayo de 2025
 * @author Agustín Alejandro Mayer
 */

#ifndef INC_MOTORS_MOTORS_H_
#define INC_MOTORS_MOTORS_H_

#include "stdint.h"
#include "utilities.h"

/** @brief Identificador para el motor izquierdo */
#define MOTOR_L 0

/** @brief Identificador para el motor derecho */
#define MOTOR_R	1

/**
 * @brief Enumeración de las posibles direcciones del motor.
 *
 * Esta enumeración define los estados de dirección del motor, incluyendo
 * estados para no inicializado, libre, avance, retroceso y frenado.
 */
typedef enum{
	NO_INIT,			/**< Motor no inicializado */
	FREE_WHEEL,			/**< Motor en modo libre (sin movimiento) */
	FORWARD,			/**< Motor en movimiento hacia adelante */
	BACKWARD,			/**< Motor en movimiento hacia atrás */
	BRAKE				/**< Motor frenado */
}e_direction;

/**
 * @brief Estructura de datos para almacenar la información y el control de un motor.
 *
 * Esta estructura contiene los parámetros de dirección, velocidad, tiempo de frenado,
 * y punteros a funciones para el control de pines y PWM del motor.
 */
typedef struct Motor_Data{
	e_direction direction;			/**< Dirección actual del motor */
	uint32_t 	maxValue;			/**< Valor máximo de velocidad permitida */
	int32_t 	vel;				/**< Velocidad actual del motor */
	uint16_t 	brakeTimeout;		/**< Tiempo de espera para el frenado */
	void 		(*setPins)(uint8_t a, uint8_t b);	/**< Puntero a función para establecer la dirección del motor */
	void 		(*setPWM)(uint16_t dCycle);	/**< Puntero a función para establecer el ciclo de trabajo del PWM */
}s_motor;

/**
 * @brief Inicializa los parámetros del motor, incluyendo las funciones de control de pines y PWM.
 *
 * Esta función debe ser llamada para configurar un motor con su respectiva
 * función de control de pines y PWM, y el valor máximo de velocidad permitido.
 *
 * @param motor Puntero a la estructura del motor a inicializar.
 * @param PWM_set Puntero a la función para establecer el ciclo de trabajo del PWM.
 * @param PIN_set Puntero a la función para establecer la dirección del motor.
 * @param max_value Valor máximo de velocidad permitida para el motor.
 */
void Motor_Init(s_motor *motor, void (*PWM_set)(uint16_t dCycle),
		void (*PIN_set)(uint8_t A, uint8_t B), uint16_t max_value);

/**
 * @brief Ajusta la velocidad del motor utilizando un valor de 16 bits.
 *
 * Esta función establece la velocidad del motor en un rango de valores
 * entre -maxValue y maxValue, donde los valores negativos indican retroceso.
 *
 * @param motor Puntero a la estructura del motor.
 * @param speed Velocidad deseada para el motor, en el rango de -maxValue a maxValue.
 */
void Motor_Set_16_Speed(s_motor *motor, int32_t speed);

/**
 * @brief Ajusta la velocidad del motor utilizando un valor porcentual.
 *
 * Esta función ajusta la velocidad del motor en función de un valor
 * porcentual entre -100 y 100, donde los valores negativos indican retroceso.
 *
 * @param motor Puntero a la estructura del motor.
 * @param speed Velocidad deseada para el motor, en porcentaje de -100 a 100.
 */
void Motor_Set_PER_Speed(s_motor *motor, int8_t speed);

void Motor_Set_Direction(s_motor *motor, e_direction direction);

/**
 * @brief Establece el tiempo de frenado del motor en milisegundos.
 *
 * Esta función permite activar el frenado del motor y configurar el tiempo
 * de espera antes de que el motor pase a un estado libre (sin movimiento).
 *
 * @param motor Puntero a la estructura del motor.
 * @param timeout Tiempo de frenado en milisegundos, con un valor máximo de 150 ms.
 */
void Motor_Set_Break_10ms(s_motor *motor, uint8_t timeout);

/**
 * @brief Actualiza el estado del motor para gestionar el tiempo de frenado.
 *
 * Esta función debe ser llamada periódicamente para gestionar el tiempo restante
 * de frenado. Cuando el tiempo de frenado se agota, el motor pasa a estado libre.
 *
 * @param motor Puntero a la estructura del motor.
 */
void Motor_Break_Timeout(s_motor *motor);


/**
 * @brief Actualiza el valor de max value, puede ser utilizado para mantener la velocidad
 * en función de la tensión aplicada al motor.
 *
 * @param motor Puntero a la estructura del motor.
 */
void Motor_Set_MaxValue(s_motor *motor, uint32_t value);

#endif /* INC_MOTORS_MOTORS_H_ */
