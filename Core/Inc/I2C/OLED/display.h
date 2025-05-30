/**
 * @file display.h
 * @brief Biblioteca para control de un display OLED SSD1306 vía I2C.
 *
 * Esta biblioteca permite inicializar, controlar y dibujar en un display OLED
 * basado en el controlador SSD1306 mediante comunicación I2C.
 *
 * Incluye funciones para escribir texto, dibujar gráficos, llenar la pantalla,
 * manejar la actualización asíncrona con DMA, y controlar directamente el encendido
 * o apagado del panel.
 *
 * @author Agustín Alejandro Mayer
 * @date 19 de mayo de 2025
 *
 * @note Requiere que el usuario asigne funciones de transmisión I2C.
 *
 * @par Ejemplo de uso:
 * @code
 * // Inicialización en main.c
 * Display_Set_I2C_Master_Transmit(&I2C_Send, &I2C_Send_Blocking);
 * Display_Init();
 * Display_Clear();
 * Display_SetCursor(0, 0);
 * Display_WriteString("Hola, mundo!", Font_7x10, SSD1306_COLOR_WHITE);
 *
 * // Cada 100ms (o tasa de refresco elegida) llamar a
 * Display_I2C_Refresh_Ready(TRUE);
 *
 * // Cada vez que se libere el canal de DMA llamar a
 * Display_I2C_DMA_Ready(TRUE);
 * @endcode
 */

#ifndef INC_I2C_OLED_DISPLAY_H_
#define INC_I2C_OLED_DISPLAY_H_

#include "utilities.h"
#include "I2C/OLED/fonts.h"
#include "stdlib.h"
#include "string.h"

#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR         	0x78
#endif

#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH            	128
#endif

#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT           	64
#endif

#ifndef Display_COM_LR_REMAP
#define Display_COM_LR_REMAP    	0
#endif

#ifndef Display_COM_ALTERNATIVE_PIN_CONFIG
#define Display_COM_ALTERNATIVE_PIN_CONFIG    1
#endif

#define OLED_DMA_BUFFER_SIZE 		1024

/**
 * @brief Colores disponibles para píxeles del display.
 */
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /*!< Pixel apagado (negro) */
	SSD1306_COLOR_WHITE = 0x01  /*!< Pixel encendido (blanco) */
} SSD1306_COLOR_t;

/**
 * @brief Asigna las funciones para transmisión I2C del sistema.
 *
 * @param Master_Transmit Función I2C no bloqueante (DMA).
 * @param Master_Transmit_Blocking Función I2C bloqueante.
 */
void Display_Set_I2C_Master_Transmit(
		e_system (*Master_Transmit)(uint16_t DevAddress, uint8_t reg, uint8_t *pData, uint16_t Size),
		e_system (*Master_Transmit_Blocking)(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout));

/**
 * @brief Inicializa el display con la configuración del controlador SSD1306.
 *
 * @return SYS_OK si la inicialización fue exitosa.
 */
e_system Display_Init(void);

/**
 * @brief Marca si el DMA ha finalizado la transmisión de datos al display.
 *
 * @param val TRUE si está listo, FALSE si no.
 */
void Display_I2C_DMA_Ready(uint8_t val);

/**
 * @brief Marca si se puede iniciar una nueva actualización de pantalla.
 *
 * @param val TRUE si está listo para actualizar, FALSE si no.
 */
void Display_I2C_Refresh_Ready(uint8_t val);

/**
 * @brief Escribe un solo byte en un registro del dispositivo I2C.
 *
 * @param address Dirección del dispositivo I2C.
 * @param reg Dirección del registro.
 * @param data Byte de datos a escribir.
 * @return SYS_OK si la operación fue exitosa.
 */
e_system ssd1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data);

/**
 * @brief Ejecuta una tarea de actualización de pantalla (envía una porción de datos).
 *
 * Debe llamarse periódicamente para realizar la actualización completa de pantalla.
 *
 * @return SYS_OK si el envío fue exitoso.
 */
e_system Display_UpdateScreen_Task(void);

/**
 * @brief Enciende el display (despierta del modo apagado).
 */
void SSD1306_ON(void);

/**
 * @brief Apaga completamente el display (modo de ahorro energético).
 */
void SSD1306_OFF(void);

/**
 * @brief Rellena toda la pantalla con un solo color.
 *
 * @param color SSD1306_COLOR_BLACK para apagar todos los píxeles,
 *              SSD1306_COLOR_WHITE para encender todos.
 */
void Display_Fill(SSD1306_COLOR_t color);

/**
 * @brief Borra la pantalla (equivalente a Display_Fill(SSD1306_COLOR_BLACK)).
 */
void Display_Clear(void);

/**
 * @brief Dibuja un único píxel en una posición determinada.
 *
 * @param x Coordenada horizontal (0 a SSD1306_WIDTH - 1).
 * @param y Coordenada vertical (0 a SSD1306_HEIGHT - 1).
 * @param color Color del píxel.
 */
void Display_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);

/**
 * @brief Dibuja un bitmap en una región del display.
 *
 * @param x Coordenada X inicial.
 * @param y Coordenada Y inicial.
 * @param bitmap Puntero a los datos del bitmap.
 * @param w Ancho del bitmap.
 * @param h Alto del bitmap.
 * @param color Color de los píxeles a dibujar.
 */
void Display_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Muestra un bitmap completo de 128x64 directamente en pantalla.
 *
 * @param bitmap Puntero al arreglo de 1024 bytes del bitmap.
 */
void Display_ShowBitmap(const unsigned char bitmap[]);

/**
 * @brief Establece la posición del cursor para escritura de texto.
 *
 * @param x Coordenada horizontal (columna).
 * @param y Coordenada vertical (fila).
 */
void Display_SetCursor(uint8_t x, uint8_t y);

/**
 * @brief Escribe una cadena de caracteres en la pantalla con una fuente y color.
 *
 * @param str Cadena de texto a mostrar.
 * @param Font Fuente utilizada.
 * @param color Color del texto.
 * @return Último carácter impreso (o 0 si falló).
 */
char Display_WriteString(char* str, FontDef_t Font, SSD1306_COLOR_t color);

/**
 * @brief Escribe un solo carácter en la pantalla.
 *
 * @param ch Carácter a mostrar.
 * @param Font Fuente utilizada.
 * @param color Color del carácter.
 * @return Carácter impreso (o 0 si falló).
 */
char Display_WriteChar(char ch, FontDef_t Font, SSD1306_COLOR_t color);

/**
 * @brief Dibuja una línea recta entre dos puntos.
 *
 * @param x0 Coordenada X inicial.
 * @param y0 Coordenada Y inicial.
 * @param x1 Coordenada X final.
 * @param y1 Coordenada Y final.
 * @param c Color de la línea.
 */
void Display_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c);

/**
 * @brief Dibuja un rectángulo sin rellenar.
 *
 * @param x Coordenada X de la esquina superior izquierda.
 * @param y Coordenada Y de la esquina superior izquierda.
 * @param w Ancho del rectángulo.
 * @param h Alto del rectángulo.
 * @param c Color de los bordes.
 */
void Display_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);

/**
 * @brief Dibuja un rectángulo sólido (relleno).
 *
 * @param x Coordenada X de la esquina superior izquierda.
 * @param y Coordenada Y de la esquina superior izquierda.
 * @param w Ancho del rectángulo.
 * @param h Alto del rectángulo.
 * @param c Color del relleno.
 */
void Display_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c);

#endif /* INC_I2C_OLED_DISPLAY_H_ */
