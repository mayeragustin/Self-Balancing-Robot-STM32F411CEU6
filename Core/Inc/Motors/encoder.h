/**
 * @file encoder.h
 * @brief Módulo para el manejo de encoders incrementales.
 *
 * Este módulo permite contar los pulsos generados por un encoder incremental, calcular
 * la cantidad de pulsos por segundo (PPS) en intervalos definidos y gestionar el reseteo
 * de los contadores. Está diseñado para ser utilizado en tareas periódicas que se ejecutan
 * cada 1 ms, así como también con interrupciones externas para el conteo de pulsos.
 *
 * @date 17 de mayo de 2025
 * @author Agustín Alejandro Mayer
 */

#ifndef INC_MOTORS_ENCODER_H_
#define INC_MOTORS_ENCODER_H_

#include "stdint.h"

/**
 * @def ENCODER_L
 * @brief Identificador del encoder izquierdo.
 */
#define ENCODER_L 0

/**
 * @def ENCODER_R
 * @brief Identificador del encoder derecho.
 */
#define ENCODER_R 1

/**
 * @brief Estructura que representa el estado interno de un encoder incremental.
 */
typedef struct {
    uint16_t resetBase;   /**< Intervalo base en milisegundos para el reseteo del conteo de pulsos. */
    uint16_t timeReset;   /**< Contador descendente hasta el próximo reseteo del conteo de pulsos. */
    uint16_t pulses;      /**< Pulsos acumulados desde el último reseteo. */
    uint16_t fastPPS;      /**< Pulsos acumulados por segundo, divididos en múltiplos del período de muestreo. */
    uint16_t pps;         /**< Pulsos por segundo consolidados al final del segundo. */
    uint8_t counter1s;    /**< Contador descendente que marca cuándo ha pasado un segundo completo. */
}s_encoder;

/**
 * @brief Inicializa la estructura del encoder.
 *
 * Establece los contadores en cero y configura los intervalos de reseteo con base en el valor especificado.
 *
 * @param enc Puntero a la estructura del encoder a inicializar.
 * @param reset Intervalo de muestreo en milisegundos (por ejemplo, 10 ms, 20 ms...).
 */
void Encoder_Init(s_encoder *enc, uint8_t reset);

/**
 * @brief Rutina periódica de actualización del encoder.
 *
 * Debe ser llamada desde una tarea o interrupción de sistema cada 1 ms.
 * Calcula la cantidad de pulsos por segundo y gestiona el reseteo de contadores.
 *
 * @param enc Puntero a la estructura del encoder a actualizar.
 */
void Encoder_Task(s_encoder *enc);

/**
 * @brief Incrementa el contador de pulsos del encoder.
 *
 * Esta función debe ser llamada desde una interrupción que detecte flancos del encoder.
 *
 * @param enc Puntero a la estructura del encoder a modificar.
 */
void Encoder_Add_Pulse(s_encoder *enc);

/**
 * @brief Señala que ha transcurrido un segundo completo.
 *
 * Consolida el conteo de PPS acumulado (pps100) y lo transfiere al valor final (pps).
 * Debe ser llamada cada 1000 ms desde una función temporizada o por el sistema.
 *
 * @param enc Puntero a la estructura del encoder.
 */
void Encoder_1s_Elapsed(s_encoder *enc);

#endif /* INC_MOTORS_ENCODER_H_ */

