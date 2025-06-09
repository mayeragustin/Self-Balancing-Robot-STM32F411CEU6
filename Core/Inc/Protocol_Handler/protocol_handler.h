/**
 * @file protocol_handler.h
 * @brief Módulo de manejo de protocolo de comunicación serial con buffer circular.
 *
 * Este archivo define las estructuras, enumeraciones y funciones necesarias para
 * implementar un protocolo de comunicación serie basado en una máquina de estados finitos (MEF),
 * incluyendo decodificación, transmisión y recepción de datos.
 *
 * @date 7 de mayo de 2025
 * @author Agustín Alejandro Mayer
 */

#ifndef INC_PROTOCOL_HANDLER_PROTOCOL_HANDLER_H_
#define INC_PROTOCOL_HANDLER_PROTOCOL_HANDLER_H_

#include <stdint.h>

/** @brief Longitud del buffer circular. No está preparado para modificarse. */
#define RINGBUFFLENGTH      		256

/** @brief Tamaño máximo del buffer auxiliar para almacenamiento temporal de datos. */
#define MAXAUXBUFFER				30

/** @brief Posición en el paquete donde se encuentra el ID del comando. */
#define POSID               		2

/** @brief Posición en el paquete donde inician los datos del payload. */
#define POSDATA             		5

/**
 * @brief Enumeración de comandos soportados por el protocolo.
 *
 * Esta enumeración puede ser modificada por el usuario según las necesidades del sistema.
 */
typedef enum{
    ACK=				0x0D,		/**< Confirmación de recepción exitosa */
	NOCMD=				0x1D,		/**< Comando inexistente o no reconocido */

	DEBUGER=			0xDE,		/**< Mensaje para depuración */
	SYSERROR= 			0xEE,		/**< Error de sistema */
	SYSWARNING=			0xEF,		/**< Aviso de sistema */

    GETALIVE=			0xF0,		/**< Solicitud de verificación de conexión (keep-alive) */
    FIRMWARE=			0xF1,		/**< Solicitud de versión de firmware */

	SETPID=				0xC0,		/**< Seteo de variables de PID */

	USERTEXT=			0xB1,		/**< Mensaje de texto definido por el usuario */
	USERNUMBER= 		0xB2,		/**< Número definido por el usuario */

	ADCSINGLE=			0xA0,		/**< Lectura de un valor ADC único */
	ADCBLOCK=			0xA1,		/**< Lectura de un bloque de valores ADC */
	SETMOTOR=			0xA2,		/**< Comando para control de motor */
	GET_ENCODER=		0xA3,		/**< Solicitud de lectura de encoder */
	MPUBLOCK=			0xA4,
	MPUOFFSET=			0xA5
}_eID;

/**
 * @brief Lista de PID configurables
 *
 * Esta lista se utiliza para identificar que valores de PID modificar desde la PC
 */
typedef enum PID_Index{
	MOTORL= 			0,
	MOTORR= 			1,
	BALANCE= 			2,
	LINE_FOLLOWER= 		3,
}s_pid;

/**
 * @brief Estados de la máquina de estados finitos (MEF) para decodificación de paquetes.
 */
typedef enum{
    START= 		0,		/**< Esperando el inicio de un paquete */
    HEADER_1=	1,		/**< Lectura del primer byte de cabecera */
    HEADER_2=	2,		/**< Lectura del segundo byte de cabecera */
    HEADER_3=	3,		/**< Lectura del tercer byte de cabecera */
    NBYTES=		4,		/**< Lectura del número de bytes del payload */
    TOKEN=		5,		/**< Lectura del token separador */
    PAYLOAD=	6		/**< Lectura de datos del payload */
}e_protocolo;

/**
 * @brief Estructura de buffer circular para transmisión o recepción.
 */
typedef struct{
	uint8_t read;						/**< Índice de lectura del buffer */
	uint8_t write;						/**< Índice de escritura del buffer */
	uint8_t buffer[RINGBUFFLENGTH];		/**< Arreglo de datos */
}s_bus;

/**
 * @brief Estructura principal de manejo de datos de comunicación serial.
 *
 * Contiene buffers, estado del protocolo y punteros a funciones de decodificación
 * y escritura personalizada.
 */
typedef struct Datos_UART{
    uint8_t timeOut;								/**< Temporizador de espera para reinicio del protocolo */
    uint8_t indexStart;								/**< Índice donde se encontró el ID en el buffer circular */
    s_bus Tx;										/**< Buffer de transmisión */
    s_bus Rx;										/**< Buffer de recepción */
    uint8_t checksumRx;								/**< Valor de checksum del paquete recibido */
    void (*dataDecoder)(struct Datos_UART *selfDD);	/**< Puntero a función encargada de decodificar el payload */
    void (*dataWriter)(struct Datos_UART *selfDW);	/**< Puntero a función encargada de enviar datos por el puerto */
    uint8_t auxBuffer[MAXAUXBUFFER];				/**< Buffer auxiliar para almacenamiento temporal del payload */
    e_protocolo protocolState;						/**< Estado actual del protocolo */
    uint8_t isESP01;								/**< Flag para indicar si se comunica a través de ESP-01 */
}s_commData;

/**
 * @brief Inicializa la estructura de comunicación y asigna funciones de usuario.
 *
 * @param comm Puntero a la estructura de comunicación a inicializar.
 * @param dataD Puntero a la función que manejará los datos recibidos.
 * @param dataW Puntero a la función que escribirá datos al canal de salida.
 */
void Comm_Init(s_commData* comm, void (*dataD)(s_commData *comm), void (*dataW)(s_commData *comm));

/**
 * @brief Ejecuta el procesamiento periódico de la máquina de estados del protocolo.
 *
 * Esta función debe ser llamada frecuentemente para procesar la entrada y decodificar los paquetes.
 *
 * @param comm Puntero a la estructura de comunicación.
 */
void Comm_Task(s_commData* comm);

/**
 * @brief Función de decodificación del protocolo.
 *
 * Se encarga de analizar el buffer y extraer los datos útiles según el estado actual.
 *
 * @param datosCom Puntero a la estructura de comunicación.
 */
void decodeProtocol(s_commData *datosCom);

/**
 * @brief Envía un comando con datos mediante el protocolo implementado.
 *
 * @param datosCom Puntero a la estructura de comunicación.
 * @param cmd ID del comando a enviar.
 * @param str Puntero al buffer de datos a enviar como payload.
 * @param len Longitud del payload a enviar.
 */
void comm_sendCMD(s_commData *datosCom, _eID cmd, uint8_t *str, uint8_t len);

#endif /* INC_PROTOCOL_HANDLER_PROTOCOL_HANDLER_H_ */
