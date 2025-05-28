/**
 * @file mpu6050.h
 * @brief Controlador del sensor MPU6050 mediante comunicación I2C.
 *
 * Este archivo define estructuras, macros y funciones necesarias para inicializar,
 * calibrar y obtener datos del acelerómetro y giróscopo del MPU6050 utilizando una
 * interfaz I2C configurable por el usuario.
 *
 * @author Agustín Alejandro Mayer
 * @date 20 de mayo de 2025
 */

#ifndef INC_I2C_MPU6050_MPU6050_H_
#define INC_I2C_MPU6050_MPU6050_H_

#include "stdint.h"
#include "utilities.h"

// Dirección del dispositivo MPU6050 (0x68 << 1 por comunicación I2C con bit R/W)
#define MPU6050_ADDR 			0xD0

// Registros internos del MPU6050
#define SIGNAL_PATH_RESET  		0x68
#define I2C_SLV0_ADDR      		0x37
#define ACCEL_CONFIG       		0x1C
#define GYRO_CONFIG_REG 		0x1B
#define SMPLRT_DIV_REG 			0x19
#define INT_ENABLE         		0x38
#define CONFIG					0x1A
#define PWR_MGMT_1				0x6B
#define INT_PIN_CFG				0x37
#define POWER_MANAGEMENT_REG 	0x6B
#define ACCEL_CONFIG_REG	 	0x1C
#define INT_STATUS				0x3A
#define INT_STATUS_DATAREADY	0x01
#define ACCEL_XOUT_REG 			0x3B
#define WHO_AM_I_MPU6050   		0x75 ///< Registro de identificación del dispositivo
#define WHO_AM_I_DEFAULT_VALUE 	0x68 ///< Valor esperado del registro WHO_AM_I
#define FIFO_EN					0x23
#define USER_CTRL				0x6A
#define FIFO_COUNTH 			0x72
#define FIFO_R_W				0x74

#define MPU_TIMEOUT				1000 ///< Tiempo de espera predeterminado para operaciones I2C
#define NUM_SAMPLES				4096  ///< Cantidad de muestras para calibración
#define NUM_SAMPLES_BITS		12
#define SCALE_FACTOR			16384 ///< Factor de escala del acelerómetro para ±2g

/**
 * @struct s_Axis
 * @brief Estructura para datos de aceleración o giro.
 */
typedef struct {
	int16_t x; ///< Componente X
	int16_t y; ///< Componente Y
	int16_t z; ///< Componente Z
	struct {
		int16_t x; ///< Offset en X
		int16_t y; ///< Offset en Y
		int16_t z; ///< Offset en Z
	} offset; ///< Compensación por calibración
	struct {
		int32_t x; ///< Offset en X
		int32_t y; ///< Offset en Y
		int32_t z; ///< Offset en Z
	} raw; ///< Compensación por calibración
} s_Axis;

/**
 * @struct s_MPU
 * @brief Estructura principal que representa al sensor MPU6050.
 */
typedef struct MPU_Data {
	s_Axis Acc; ///< Datos del acelerómetro
	s_Axis Gyro; ///< Datos del giróscopo
	struct {
		int16_t pitch; ///< Ángulo de pitch (inclinación)
		int16_t roll;  ///< Ángulo de roll (balanceo)
		int16_t yaw;   ///< Ángulo de yaw (giro)
	} Angle; ///< Ángulos calculados (opcional)
	uint8_t data[14]; ///< Buffer de lectura de datos crudos (acelerómetro + giroscopio)
	uint8_t dataReady; ///< Indicador de disponibilidad de nuevos datos
	uint8_t isInit;
} s_MPU;

/**
 * @brief Configura las funciones de escritura y lectura por I2C.
 *
 * @param Mem_Write_Blocking Función para escritura en memoria I2C (bloqueante).
 * @param Mem_Read_Blocking Función para lectura en memoria I2C (bloqueante).
 */
void MPU6050_Set_I2C_Communication(
	e_system (*Mem_Write_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout),
	e_system (*Mem_Read_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout));

/**
 * @brief Inicializa el sensor MPU6050.
 *
 * Configura los registros básicos del sensor y ejecuta el proceso de calibración.
 *
 * @param mpu Puntero a la estructura que representa al sensor.
 * @return SYS_OK si la inicialización fue exitosa, SYS_ERROR en caso contrario.
 */
e_system MPU6050_Init(s_MPU *mpu);

/**
 * @brief Ejecuta el proceso de calibración del sensor.
 *
 * Calcula y almacena los valores promedio de aceleración y giro cuando el sensor está quieto.
 *
 * @param mpu Puntero a la estructura del sensor.
 */
void MPU6050_Calibrate(s_MPU *mpu);

/**
 * @brief Procesa los datos crudos leídos por DMA.
 *
 * Esta función debe llamarse en el callback de fin de transferencia DMA para interpretar
 * los datos recibidos desde el sensor y aplicar compensación por calibración.
 *
 * @param mpu Puntero a la estructura del sensor.
 */
void MPU6050_I2C_DMA_Cplt(s_MPU *mpu);

#endif /* INC_I2C_MPU6050_MPU6050_H_ */
