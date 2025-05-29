/*
 * display.h
 *
 *  Created on: May 19, 2025
 *      Author: Agustín Alejandro Mayer
 */

#ifndef INC_I2C_OLED_DISPLAY_H_
#define INC_I2C_OLED_DISPLAY_H_

// Inclusión de dependencias necesarias para la funcionalidad del display OLED I2C
#include "utilities.h"                   // Utilidades generales del sistema
#include "I2C/OLED/fonts.h"             // Fuentes para display OLED
#include "stdlib.h"                     // Funciones estándar de C
#include "string.h"                     // Funciones para manejo de cadenas
#include <stdarg.h>                     // Para manejo de argumentos variables

/* Dirección I2C por defecto para el SSD1306 */
#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR         	0x78
#endif

/* Dimensiones del display SSD1306 en píxeles */
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH            	128
#endif

#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT           	64
#endif

/* Configuraciones específicas de hardware (remapeo de pines COM) */
#ifndef Display_COM_LR_REMAP
#define Display_COM_LR_REMAP    	0
#endif

#ifndef Display_COM_ALTERNATIVE_PIN_CONFIG
#define Display_COM_ALTERNATIVE_PIN_CONFIG    1
#endif

// Definiciones internas para buffer y manejo de datos
#define OLED_DMA_BUFFER_SIZE 		1024

/* Enumeración para definir colores en el display (blanco o negro) */
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /*!< Color negro, pixel apagado */
	SSD1306_COLOR_WHITE = 0x01  /*!< Color blanco, pixel encendido */
} SSD1306_COLOR_t;

/**
 * @brief Configura las funciones de transmisión I2C para el display.
 * @param Master_Transmit Puntero a función para transmisión I2C no bloqueante.
 * @param Master_Transmit_Blocking Puntero a función para transmisión I2C bloqueante.
 */
void Display_Set_I2C_Master_Transmit(
		e_system (*Master_Transmit)(uint16_t DevAddress, uint8_t reg, uint8_t *pData, uint16_t Size),
		e_system (*Master_Transmit_Blocking)(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout));

/**
 * @brief Inicializa el display OLED con configuración estándar SSD1306.
 * @return Estado del sistema (e_system), SYS_OK si la inicialización fue exitosa.
 */
e_system Display_Init(void);

void Display_I2C_DMA_Ready(uint8_t val);

/**
 * @brief Indica si el display está listo para refrescar la pantalla.
 * @param val Estado (TRUE/FALSE) de disponibilidad para refrescar.
 */
void Display_I2C_Refresh_Ready(uint8_t val);

/**
 * @brief Escribe un solo byte en el dispositivo I2C en un registro específico.
 * @param address Dirección I2C del dispositivo.
 * @param reg Registro del dispositivo donde escribir.
 * @param data Byte de datos a escribir.
 * @return Estado del sistema (e_system).
 */
e_system ssd1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data);

/**
 * @brief Tarea para actualizar el contenido de la pantalla OLED de forma asíncrona.
 * Debe llamarse periódicamente para enviar datos al display.
 */
e_system Display_UpdateScreen_Task(void);

/**
 * @brief Enciende el display OLED.
 */
void SSD1306_ON(void);

/**
 * @brief Apaga el display OLED.
 */
void SSD1306_OFF(void);

/**
 * @brief Llena toda la pantalla con un color específico (blanco o negro).
 * @param color Color para llenar la pantalla (SSD1306_COLOR_BLACK o SSD1306_COLOR_WHITE).
 */
void Display_Fill(SSD1306_COLOR_t color);

/**
 * @brief Limpia la pantalla, equivalente a llenarla de negro.
 */
void Display_Clear(void);

/**
 * @brief Dibuja un pixel en coordenadas específicas con un color dado.
 * @param x Coordenada horizontal (0 a SSD1306_WIDTH - 1).
 * @param y Coordenada vertical (0 a SSD1306_HEIGHT - 1).
 * @param color Color del pixel (blanco o negro).
 */
void Display_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);

/**
 * @brief Dibuja un bitmap en la pantalla en la posición (x, y).
 * @param x Coordenada horizontal donde iniciar el dibujo.
 * @param y Coordenada vertical donde iniciar el dibujo.
 * @param bitmap Puntero al arreglo de bytes del bitmap.
 * @param w Ancho del bitmap en píxeles.
 * @param h Alto del bitmap en píxeles.
 * @param color Color para dibujar (normalmente blanco).
 */
void Display_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Muestra un bitmap completo de 128x64 pixeles en la pantalla.
 * @param bitmap Bitmap completo a mostrar.
 */
void Display_ShowBitmap(const unsigned char bitmap[]);

void Display_SetCursor(uint8_t x, uint8_t y);

char Display_WriteString(char* str, FontDef_t Font, SSD1306_COLOR_t color);

char Display_WriteChar(char ch, FontDef_t Font, SSD1306_COLOR_t color);

void Display_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c);

void Display_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);

void Display_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);

#endif /* INC_I2C_OLED_DISPLAY_H_ */
