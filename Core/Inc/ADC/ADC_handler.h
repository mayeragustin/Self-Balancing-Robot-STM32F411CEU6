/**
 * @file ADC_handler.h
 * @brief Módulo para el manejo y procesamiento de datos del ADC.
 *
 * Este módulo implementa un filtro de media móvil circular para estabilizar
 * las lecturas de sensores analógicos obtenidas mediante el ADC. Está preparado
 * para aplicar una linealización posterior utilizando una tabla LUT.
 *
 * @date 12 de mayo de 2025
 * @author Agustín Alejandro Mayer
 */

#ifndef INC_ADC_ADC_HANDLER_H_
#define INC_ADC_ADC_HANDLER_H_

#include <stdint.h>

/** @brief Tamaño de la tabla de búsqueda (LUT) para la linealización del ADC */
#define ADC_LUT_SIZE            20

/** @brief Cantidad de sensores conectados al ADC */
#define ADC_NUM_SENSORS         9

/**
 * @brief Tamaño del filtro de media móvil.
 *
 * Debe ser una potencia de 2 para asegurar el funcionamiento correcto del índice circular.
 */
#define ADC_MEDIA_SIZE          32

/**
 * @brief Cantidad de veces a desplazar "dividir" el promedio
 * Usar 2^ADC_DESPLAZAMIENTOS = ADC_MEDIA_SIZE
 */
#define ADC_DESPLAZAMIENTOS 	5

/**
 * @brief Filtro de media móvil aplicado a las conversiones ADC.
 *
 * Una vez que se completa la conversión del ADC, esta función aplica un filtro de media móvil
 * a los datos crudos y los almacena en el vector de datos filtrados. Usa un buffer circular
 * de tamaño `ADC_MEDIA_SIZE` para cada canal.
 *
 * @param rawData Puntero al vector que contiene los datos crudos leídos del ADC.
 * @param filtredData Puntero al vector donde se almacenarán los datos filtrados.
 */
void ADC_Conversion_Cplt(uint16_t *rawData, uint16_t *filtredData);

/**
 * @brief Linealización de datos ADC mediante una LUT.
 *
 * Esta función está prevista para implementar una corrección no lineal
 * sobre los datos filtrados del ADC. Actualmente es un espacio reservado.
 */
void ADC_Linealization(void);

#endif /* INC_ADC_ADC_HANDLER_H_ */
