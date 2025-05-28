/*
 * mpu6050.c
 *
 *  Created on: May 20, 2025
 *      Author: Agustín Alejandro Mayer
 */

#include "I2C/MPU6050/mpu6050.h"

static e_system (*I2C_Master_Transmit_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout);
static e_system (*I2C_Mem_Read)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout);

void MPU6050_Set_I2C_Communication(
		e_system (*Mem_Write_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout),
		e_system (*Mem_Read_Blocking)(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout)){
	I2C_Master_Transmit_Blocking = Mem_Write_Blocking;
	I2C_Mem_Read = Mem_Read_Blocking;
}

e_system MPU6050_Init(s_MPU *mpu){
	uint8_t data = 0;
	e_system status = SYS_OK;
	status += I2C_Mem_Read(MPU6050_ADDR, WHO_AM_I_MPU6050, 1, &data, 1, MPU_TIMEOUT);
	if(data == WHO_AM_I_DEFAULT_VALUE){
		data = 0x00;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, POWER_MANAGEMENT_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set data rate of 1 KHz (default)
		data = 0x07;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, SMPLRT_DIV_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set accelerometer range of +/- 2g (default)
		data = 0x00;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set gyroscope range of +/- 250 degree/s (default)
		data = 0x00;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set Digital Low Pass Filter
		data = 0x03;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, CONFIG, 1, &data, 1, MPU_TIMEOUT);
		/*
		data = 0x20;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, INT_PIN_CFG, 1, &data, 1, MPU_TIMEOUT);

		data = 0x01;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, INT_ENABLE, 1, &data, 1, MPU_TIMEOUT);
		 */
		*mpu = (s_MPU){
			.Acc = {0, 0, 0, {0, 0, 0}},
			.Gyro = {0, 0, 0, {0, 0, 0}},
			.Angle = {0, 0, 0},
			.dataReady = 0,
			.isInit = 1
		};

		for(uint8_t i=0; i<14; i++){
			mpu->data[i] = 0;
		}

		if(status != SYS_OK){
			return SYS_ERROR;
		}


	}else{
		return SYS_ERROR;
	}
	return SYS_OK;
}

void MPU6050_Calibrate(s_MPU *mpu){
	for (uint16_t i=0; i < NUM_SAMPLES; i++){
		I2C_Mem_Read(MPU6050_ADDR, ACCEL_XOUT_REG, 1, mpu->data, 14, MPU_TIMEOUT);
		mpu->Acc.raw.x += (int16_t)((mpu->data[0] << 8) | mpu->data[1]);
		mpu->Acc.raw.y += (int16_t)((mpu->data[2] << 8) | mpu->data[3]);
		mpu->Acc.raw.z += (int16_t)((mpu->data[4] << 8) | mpu->data[5]);

		mpu->Gyro.raw.x += (int16_t)((mpu->data[8 ] << 8) | mpu->data[9 ]);
		mpu->Gyro.raw.y += (int16_t)((mpu->data[10] << 8) | mpu->data[11]);
		mpu->Gyro.raw.z += (int16_t)((mpu->data[12] << 8) | mpu->data[13]);
	}
    mpu->Acc.offset.x = (int16_t)(mpu->Acc.raw.x >> NUM_SAMPLES_BITS);
    mpu->Acc.offset.y = (int16_t)(mpu->Acc.raw.y >> NUM_SAMPLES_BITS);
    mpu->Acc.offset.z = (int16_t)(mpu->Acc.raw.z >> NUM_SAMPLES_BITS) - SCALE_FACTOR;

    mpu->Gyro.offset.x = (int16_t)(mpu->Gyro.raw.x >> NUM_SAMPLES_BITS);
	mpu->Gyro.offset.y = (int16_t)(mpu->Gyro.raw.y >> NUM_SAMPLES_BITS);
	mpu->Gyro.offset.z = (int16_t)(mpu->Gyro.raw.z >> NUM_SAMPLES_BITS);
}

void MPU6050_I2C_DMA_Cplt(s_MPU *mpu){
	// ACC: GET RAW INFORMATION
	mpu->Acc.raw.x = (((mpu->data[0] << 8) | mpu->data[1]));
	mpu->Acc.raw.y = (((mpu->data[2] << 8) | mpu->data[3]));
	mpu->Acc.raw.z = (((mpu->data[4] << 8) | mpu->data[5]));
	// ACC: CALCULATE TRUE ACCELERATION
	mpu->Acc.x = mpu->Acc.raw.x - mpu->Acc.offset.x;
	mpu->Acc.y = mpu->Acc.raw.y - mpu->Acc.offset.y;
	mpu->Acc.z = mpu->Acc.raw.z - mpu->Acc.offset.z;

	// GYR: GET RAW INFORMATION
	mpu->Gyro.raw.x = (((mpu->data[8 ] << 8) | mpu->data[9 ]));
	mpu->Gyro.raw.y = (((mpu->data[10] << 8) | mpu->data[11]));
	mpu->Gyro.raw.z = (((mpu->data[12] << 8) | mpu->data[13]));
	// GYR: CALCULATE TRUE ACCELERATION
	mpu->Gyro.x = mpu->Gyro.raw.x - mpu->Gyro.offset.x;
	mpu->Gyro.y = mpu->Gyro.raw.y - mpu->Gyro.offset.y;
	mpu->Gyro.z = mpu->Gyro.raw.z - mpu->Gyro.offset.z;

}

void MPU6050_I2C_Read(s_MPU *mpu){

}
