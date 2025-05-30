/**
 * @file utilities.h
 * @brief Definiciones y utilidades generales para el sistema.
 *
 * Este archivo contiene definiciones comunes como macros, enumeraciones y uniones útiles para
 * el desarrollo de firmware embebido. Facilita la manipulación de bits, conversiones de tipos
 * y estados del sistema.
 *
 * @author Agustín Alejandro Mayer
 * @date 12 de mayo de 2025
 */

#ifndef INC_UTILITIES_H_
#define INC_UTILITIES_H_

#include "stdint.h"

/**
 * @def TRUE
 * @brief Valor lógico verdadero (1)
 */
#define TRUE 	1

/**
 * @def FALSE
 * @brief Valor lógico falso (0)
 */
#define FALSE 	0

/**
 * @enum e_system
 * @brief Estados generales del sistema.
 */
typedef enum{
	SYS_OK,     /**< El sistema se encuentra en estado correcto. */
	SYS_ERROR,  /**< Ocurrió un error en el sistema. */
	SYS_BUSY    /**< El sistema está ocupado procesando. */
} e_system;

/**
 * @union u_flag
 * @brief Unión para manipulación de un byte como bits individuales o como nibbles.
 *
 * Permite el acceso a:
 * - Bits individuales mediante `bit.b0` a `bit.b7`.
 * - Nibbles bajos y altos mediante `nibble.L` y `nibble.H`.
 * - Acceso completo al byte mediante `byte`.
 */
typedef union{
	struct{
		uint8_t b0: 1; /**< Bit 0 */
		uint8_t b1: 1; /**< Bit 1 */
		uint8_t b2: 1; /**< Bit 2 */
		uint8_t b3: 1; /**< Bit 3 */
		uint8_t b4: 1; /**< Bit 4 */
		uint8_t b5: 1; /**< Bit 5 */
		uint8_t b6: 1; /**< Bit 6 */
		uint8_t b7: 1; /**< Bit 7 */
	} bit;
	struct{
		uint8_t L: 4; /**< Nibble bajo (bits 0 a 3) */
		uint8_t H: 4; /**< Nibble alto (bits 4 a 7) */
	} nibble;
	uint8_t byte; /**< Acceso completo al byte */
}u_flag;

/**
 * @union u_conv
 * @brief Unión para conversión entre diferentes tipos de datos de 32 bits.
 *
 * Permite acceder a la misma región de memoria como:
 * - Flotante de 32 bits (`float f`)
 * - Entero con signo de 32 bits (`int32_t i32`)
 * - Entero sin signo de 32 bits (`uint32_t ui32`)
 * - Dos enteros de 16 bits (`int16_t i16[2]`)
 * - Dos enteros sin signo de 16 bits (`uint16_t ui16[2]`)
 * - Cuatro enteros de 8 bits (`int8_t i8[4]`)
 * - Cuatro enteros sin signo de 8 bits (`uint8_t ui8[4]`)
 */
typedef union{
	float    f;       /**< Acceso como flotante de 32 bits */
	int32_t  i32;     /**< Acceso como entero con signo de 32 bits */
	uint32_t ui32;    /**< Acceso como entero sin signo de 32 bits */
	int16_t  i16[2];  /**< Acceso como dos enteros con signo de 16 bits */
	uint16_t ui16[2]; /**< Acceso como dos enteros sin signo de 16 bits */
	int8_t   i8[4];   /**< Acceso como cuatro enteros con signo de 8 bits */
	uint8_t  ui8[4];  /**< Acceso como cuatro enteros sin signo de 8 bits */
} u_conv;

#endif /* INC_UTILITIES_H_ */
