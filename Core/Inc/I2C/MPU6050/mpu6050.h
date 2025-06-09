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

#include <stdint.h>
#include "utilities.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dirección del dispositivo (0x68 << 1 por bit R/W en I2C)
#define MPU6050_ADDR                0xD0

// Registros internos
#define SIGNAL_PATH_RESET           0x68
#define I2C_SLV0_ADDR               0x37
#define ACCEL_CONFIG                0x1C
#define GYRO_CONFIG_REG             0x1B
#define SMPLRT_DIV_REG              0x19
#define INT_ENABLE                  0x38
#define CONFIG                      0x1A
#define PWR_MGMT_1                  0x6B
#define INT_PIN_CFG                 0x37
#define POWER_MANAGEMENT_REG        0x6B
#define ACCEL_CONFIG_REG            0x1C
#define INT_STATUS                  0x3A
#define INT_STATUS_DATAREADY        0x01
#define ACCEL_XOUT_REG              0x3B
#define WHO_AM_I_MPU6050            0x75 ///< Registro de identificación
#define WHO_AM_I_DEFAULT_VALUE      0x68 ///< Valor esperado del WHO_AM_I
#define FIFO_EN                     0x23
#define USER_CTRL                   0x6A
#define FIFO_COUNTH                 0x72
#define FIFO_R_W                    0x74

// Parámetros de configuración
#define MPU_TIMEOUT                 1000    ///< Timeout para operaciones I2C
#define NUM_SAMPLES                 4096    ///< Muestras para calibración
#define NUM_SAMPLES_BITS            12      ///< Bits de desplazamiento equivalente a 4096
#define SCALE_FACTOR                16384   ///< Factor de escala para ±2g

#ifndef NUM_MAF_BITS
#define NUM_MAF_BITS				3
#endif

#ifndef NUM_MAF
#define NUM_MAF						8
#endif

#ifndef NUM_AXIS
#define NUM_AXIS					6
#endif

/**
 * @struct s_Axis
 * @brief Representa un eje de aceleración o giro con compensaciones y valores crudos.
 */
typedef struct {
    int16_t x;  ///< Componente X corregido
    int16_t y;  ///< Componente Y corregido
    int16_t z;  ///< Componente Z corregido

    struct {
        int16_t x; ///< Offset en X
        int16_t y; ///< Offset en Y
        int16_t z; ///< Offset en Z
    } offset;
} s_Axis;

/**
 * @struct s_MPU
 * @brief Representa el estado completo del sensor MPU6050.
 */
typedef struct {
    s_Axis Acc;    ///< Datos del acelerómetro
    s_Axis Gyro;   ///< Datos del giróscopo
    struct{
        int16_t pitch; ///< Ángulo de pitch (inclinación)
        int16_t roll;  ///< Ángulo de roll (balanceo)
        int16_t yaw;   ///< Ángulo de yaw (giro)
    }Angle;          ///< Ángulos calculados
    struct{
    	int32_t sumData[NUM_AXIS];
    	int16_t mediaBuffer[NUM_MAF][NUM_AXIS];
    	int16_t rawData[NUM_AXIS];
    	int16_t filtredData[NUM_AXIS];
    	uint8_t index;
    	uint8_t isOn;
    }MAF;
    uint8_t bit_data[14]; ///< Buffer de datos crudos leídos por DMA
    uint8_t isInit;   ///< Flag de inicialización
} s_MPU;

/**
 * @brief Establece las funciones de comunicación I2C personalizadas.
 *
 * Estas funciones permiten al usuario definir su propia implementación de
 * transmisión y recepción por I2C para adaptarse a diferentes controladores o HAL.
 *
 * @param Mem_Write_Blocking Función para escritura en memoria I2C.
 * @param Mem_Read_Blocking  Función para lectura desde memoria I2C.
 */
void MPU6050_Set_I2C_Communication(
    e_system (*Mem_Write_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize,
                                   uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout),
    e_system (*Mem_Read_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize,
                                  uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout));

/**
 * @brief Inicializa el sensor MPU6050.
 *
 * Configura los registros internos básicos y ejecuta una calibración inicial.
 *
 * @param mpu Puntero a la estructura del sensor.
 * @return SYS_OK si fue exitoso, SYS_ERROR en caso de fallo de comunicación.
 */
e_system MPU6050_Init(s_MPU *mpu);

/**
 * @brief Calibra el sensor MPU6050.
 *
 * Promedia las lecturas cuando el sensor está estático para calcular los offsets.
 *
 * @param mpu Puntero a la estructura del sensor.
 */
void MPU6050_Calibrate(s_MPU *mpu);

/**
 * @brief Procesa los datos luego de una transferencia por DMA.
 *
 * Interpreta los datos almacenados en el buffer y aplica la calibración.
 *
 * @param mpu Puntero a la estructura del sensor.
 */
void MPU6050_I2C_DMA_Cplt(s_MPU *mpu);

void MPU6050_MAF(s_MPU *mpu);

#ifdef __cplusplus
}
#endif

#endif /* INC_I2C_MPU6050_MPU6050_H_ */
