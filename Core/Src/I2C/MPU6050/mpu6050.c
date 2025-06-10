/*
 * mpu6050.c
 *
 *  Created on: May 20, 2025
 *      Author: Agustín Alejandro Mayer
 */

#include "I2C/MPU6050/mpu6050.h"
#include "math.h"

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
		status += I2C_Master_Transmit_Blocking(MPU6050_ADDR, POWER_MANAGEMENT_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set data rate of 1 KHz (default)
		data = 0x07;
		status +=I2C_Master_Transmit_Blocking(MPU6050_ADDR, SMPLRT_DIV_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set accelerometer range of +/- 2g (default)
		data = 0x00;
		status += I2C_Master_Transmit_Blocking(MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set gyroscope range of +/- 250 degree/s (default)
		data = 0x00;
		status += I2C_Master_Transmit_Blocking(MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1, MPU_TIMEOUT);

		// Set Digital Low Pass Filter
		data = 0x03;
		status += I2C_Master_Transmit_Blocking(MPU6050_ADDR, CONFIG, 1, &data, 1, MPU_TIMEOUT);
		/*
		data = 0x20;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, INT_PIN_CFG, 1, &data, 1, MPU_TIMEOUT);

		data = 0x01;
		I2C_Master_Transmit_Blocking(MPU6050_ADDR, INT_ENABLE, 1, &data, 1, MPU_TIMEOUT);
		 */
		*mpu = (s_MPU){
		    .Acc = {
		        .x = 0, .y = 0, .z = 0,
		        .offset = { .x = 0, .y = 0, .z = 0 }
		    },
		    .Gyro = {
		        .x = 0, .y = 0, .z = 0,
		        .offset = { .x = 0, .y = 0, .z = 0 }
		    },
		    .Angle = {
		        .pitch = 0, .roll = 0, .yaw = 0
		    },
		    .MAF = {
		        .sumData = {0},
		        .mediaBuffer = {{0}},
		        .rawData = {0},
		        .filtredData = {0},
		        .index = 0,
		        .isOn = 0
		    },
		    .bit_data = {0},
		    .isInit = 1
		};

		if(status != SYS_OK){
			return SYS_ERROR;
		}

	}else{
		return SYS_ERROR;
	}
	return SYS_OK;
}

void MPU6050_Calibrate(s_MPU *mpu){
	int32_t temp_raw[6] = {0};
	for (uint16_t i=0; i < NUM_SAMPLES; i++){
		I2C_Mem_Read(MPU6050_ADDR, ACCEL_XOUT_REG, 1, mpu->bit_data, 14, MPU_TIMEOUT);
		temp_raw[0] += (int16_t)((mpu->bit_data[0] << 8) | mpu->bit_data[1]);
		temp_raw[1] += (int16_t)((mpu->bit_data[2] << 8) | mpu->bit_data[3]);
		temp_raw[2] += (int16_t)((mpu->bit_data[4] << 8) | mpu->bit_data[5]);

		temp_raw[3] += (int16_t)((mpu->bit_data[8 ] << 8) | mpu->bit_data[9 ]);
		temp_raw[4] += (int16_t)((mpu->bit_data[10] << 8) | mpu->bit_data[11]);
		temp_raw[5] += (int16_t)((mpu->bit_data[12] << 8) | mpu->bit_data[13]);
	}
    mpu->Acc.offset.x = (int16_t)(temp_raw[0] >> NUM_SAMPLES_BITS);
    mpu->Acc.offset.y = (int16_t)(temp_raw[1] >> NUM_SAMPLES_BITS);
    mpu->Acc.offset.z = (int16_t)(temp_raw[2] >> NUM_SAMPLES_BITS)/* - SCALE_FACTOR*/;

    mpu->Gyro.offset.x = (int16_t)(temp_raw[3] >> NUM_SAMPLES_BITS);
	mpu->Gyro.offset.y = (int16_t)(temp_raw[4] >> NUM_SAMPLES_BITS);
	mpu->Gyro.offset.z = (int16_t)(temp_raw[5] >> NUM_SAMPLES_BITS);

	//mpu->Angle.pitch = atan2f(mpu->Acc.offset.y, sqrtf(mpu->Acc.offset.x * mpu->Acc.offset.x + mpu->Acc.offset.z * mpu->Acc.offset.z)) * 180.0f / M_PI;
	//mpu->Angle.roll  = atan2f(-mpu->Acc.offset.x, mpu->Acc.offset.z) * 180.0f / M_PI;
}

void MPU6050_I2C_DMA_Cplt(s_MPU *mpu){
	// ACC: GET RAW INFORMATION
	mpu->MAF.rawData[0] = (((mpu->bit_data[0] << 8) | mpu->bit_data[1]));
	mpu->MAF.rawData[1] = (((mpu->bit_data[2] << 8) | mpu->bit_data[3]));
	mpu->MAF.rawData[2] = (((mpu->bit_data[4] << 8) | mpu->bit_data[5]));
	// GYR: GET RAW INFORMATION
	mpu->MAF.rawData[3] = (((mpu->bit_data[8 ] << 8) | mpu->bit_data[9 ]));
	mpu->MAF.rawData[4] = (((mpu->bit_data[10] << 8) | mpu->bit_data[11]));
	mpu->MAF.rawData[5] = (((mpu->bit_data[12] << 8) | mpu->bit_data[13]));
	mpu->MAF.isOn = TRUE;
}

void MPU6050_MAF(s_MPU *mpu){ //Moving Average Filter
	if(mpu->MAF.isOn){
		mpu->MAF.isOn = FALSE;
		for(uint8_t channel = 0; channel < NUM_AXIS; channel++){
			mpu->MAF.sumData[channel] -= mpu->MAF.mediaBuffer[mpu->MAF.index][channel];
			mpu->MAF.sumData[channel] += mpu->MAF.rawData[channel];
			mpu->MAF.mediaBuffer[mpu->MAF.index][channel] = mpu->MAF.rawData[channel];
			mpu->MAF.filtredData[channel] = (mpu->MAF.sumData[channel] >> NUM_MAF_BITS);
		}
		mpu->MAF.index++;
		mpu->MAF.index &= (NUM_MAF - 1);

		// ACC: CALCULATE TRUE ACCELERATION
		mpu->Acc.x = mpu->MAF.filtredData[0] - mpu->Acc.offset.x;
		mpu->Acc.y = mpu->MAF.filtredData[1] - mpu->Acc.offset.y;
		mpu->Acc.z = mpu->MAF.filtredData[2] - mpu->Acc.offset.z;
		// GYR: CALCULATE TRUE ACCELERATION
		mpu->Gyro.x = mpu->MAF.filtredData[3] - mpu->Gyro.offset.x;
		mpu->Gyro.y = mpu->MAF.filtredData[4] - mpu->Gyro.offset.y;
		mpu->Gyro.z = mpu->MAF.filtredData[5] - mpu->Gyro.offset.z;
	}
}
