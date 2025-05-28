/**
 * @file encoder.h
 * @brief Módulo para el manejo de encoders incrementales.
 *
 * Este módulo permite contar pulsos de un encoder, calcular la cantidad de pulsos por segundo (pps),
 * y resetear el conteo a intervalos configurables. Está pensado para usarse con tareas periódicas.
 *
 * @date 17 de mayo de 2025
 * @author Agustín Alejandro Mayer
 */

#ifndef INC_MOTORS_ENCODER_H_
#define INC_MOTORS_ENCODER_H_

#include "stdint.h"

/** @brief Identificador del encoder izquierdo */
#define ENCODER_L 0

/** @brief Identificador del encoder derecho */
#define ENCODER_R 1

/**
 * @brief Estructura que representa el estado de un encoder.
 */
typedef struct{
	uint16_t resetBase;     /**< Valor base de tiempo para el reseteo del conteo de pulsos */
	uint16_t timeReset;     /**< Contador descendente para el próximo reseteo de pulsos */
	uint16_t pulses;        /**< Cantidad de pulsos acumulados en el intervalo actual */
	uint16_t pps100;
	uint16_t pps;           /**< Pulsos por segundo acumulados (refrescados periódicamente) */
	uint8_t counter1s;      /**< Contador descendente para resetear el pps una vez por segundo */
}s_encoder;

/**
 * @brief Inicializa la estructura del encoder.
 *
 * Establece los valores iniciales del contador de pulsos, el tiempo de reseteo
 * y el conteo para la actualización del valor de pps.
 *
 * @param enc Puntero a la estructura del encoder a inicializar.
 * @param reset Valor base del intervalo de muestreo, en milisegundos.
 */
void Encoder_Init(s_encoder *enc, uint8_t reset);

/**
 * @brief Función periódica para actualizar los valores del encoder.
 *
 * Esta función debe ser llamada regularmente (cada 1 ms) para gestionar
 * el conteo de pulsos y calcular los pulsos por segundo (pps).
 *
 * @param enc Puntero a la estructura del encoder.
 */
void Encoder_Task(s_encoder *enc);

/**
 * @brief Incrementa el conteo de pulsos del encoder.
 *
 * Esta función debe ser llamada por la interrupción o rutina que detecta
 * los flancos del encoder.
 *
 * @param enc Puntero a la estructura del encoder.
 */
void Encoder_Add_Pulse(s_encoder *enc);

void Encoder_1s_Elapsed(s_encoder *enc);

#endif /* INC_MOTORS_ENCODER_H_ */
