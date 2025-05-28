/**
 * @file debouncer.h
 * @brief Módulo de antirrebote para entradas digitales.
 *
 * Este módulo permite manejar múltiples entradas digitales con protección por software
 * contra rebotes mecánicos, detectando transiciones de flanco y manteniendo el estado
 * estable de cada entrada. Admite funciones de callback por cambio de estado.
 *
 * @date 13 de mayo de 2025
 * @author Agustín Alejandro Mayer
 */

#ifndef INC_DEBOUNCER_DEBOUNCER_H_
#define INC_DEBOUNCER_DEBOUNCER_H_

#include <stdint.h>

/** @brief Número máximo de entradas gestionadas por el debouncer */
#define MAX_INPUTS_DEBOUNCED    1

/** @brief Identificador para detección de flanco descendente */
#define FALLING_EDGE            0

/** @brief Identificador para detección de flanco ascendente */
#define RISING_EDGE             1

/** @brief Estado lógico alto */
#define SETED                   1

/** @brief Estado lógico bajo */
#define CLEARED                 0

/**
 * @brief Estados internos del sistema de debounce.
 */
typedef enum {
    DOWN,       /**< Entrada en nivel bajo estable */
    UP,         /**< Entrada en nivel alto estable */
    FALLING,    /**< Transición de alto a bajo detectada */
    RISING      /**< Transición de bajo a alto detectada */
} e_Estados;

/**
 * @brief Estructura que representa una entrada digital antirreboteada.
 */
typedef struct {
    uint8_t value;                         /**< Valor lógico leído directamente del hardware */
    e_Estados state;                       /**< Estado interno del sistema de debounce */
    uint8_t val;                           /**< Valor lógico filtrado de la entrada */
    void (*stateChanged)(e_Estados estado);/**< Función callback que se llama ante un cambio de estado */
    uint8_t (*getInputState)();            /**< Función abstracta para leer el estado lógico del hardware */
} s_Input;

/**
 * @brief Inicializa el sistema de debounce.
 *
 * Esta función reinicia todos los registros internos y deja el buffer de entradas vacío.
 * Debe llamarse antes de utilizar otras funciones del módulo.
 */
void Debounce_Init(void);

/**
 * @brief Ejecuta la tarea de debounce.
 *
 * Esta función debe ser llamada periódicamente (ej. cada 1 ms) desde el scheduler o un temporizador
 * para procesar el estado de cada entrada, detectar cambios y ejecutar callbacks.
 */
void Debouncer_Task(void);

/**
 * @brief Agrega una nueva entrada digital al sistema de debounce.
 *
 * @param AbstHard Función abstracta que devuelve el estado lógico actual del pin (0 o 1).
 * @param STATECHANGED Función callback que se ejecuta al detectar un flanco (cambio de estado).
 * @return uint8_t Identificador de la entrada agregada, o 0 si se excedió el máximo permitido.
 */
uint8_t Debounce_Add(uint8_t (*AbstHard)(), void (*STATECHANGED)(e_Estados estado));

/**
 * @brief Actualiza el estado interno de una entrada.
 *
 * Esta función es utilizada internamente para procesar las transiciones y disparar eventos.
 *
 * @param stateInput Puntero a la estructura de entrada que se desea actualizar.
 */
void inputState(s_Input *stateInput);

/**
 * @brief Obtiene el estado lógico actual (estable) de una entrada.
 *
 * @param input Identificador de la entrada consultada.
 * @return uint8_t Estado lógico actual (0 = LOW, 1 = HIGH).
 */
uint8_t getInputState(uint8_t input);

#endif /* INC_DEBOUNCER_DEBOUNCER_H_ */
