/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "utilities.h"
#include "usbd_cdc_if.h"
#include "Protocol_Handler/protocol_handler.h"
#include "ADC/ADC_handler.h"
#include "Debouncer/debouncer.h"
#include "Motors/motors.h"
#include "Motors/encoder.h"
#include "I2C/OLED/display.h"
#include "I2C/MPU6050/mpu6050.h"

#include "WiFi/ESP01.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
	IDLE,
	FOLLOW_LINE,
	GO_FROM_TO
}e_Car_state;

typedef enum{
	INIT,
	MENU,
	INPUTS
}e_Disp_state;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DISPLAY_TYPICAL_REFRESH_RATE_10MS		10 	//< 100ms de periodo, 10FPS
#define DISPLAY_MEDIUM_REFRESH_RATE_10MS		20 	//< 250ms de periodo, 5FPS
#define DISPLAY_LOW_REFRESH_RATE_10MS			100 //< 1000ms de periodo, 1FPS

#define ENCODER_FASTPPS_COUNTER_10MS			10 //< Toma el valor de los encoders cada 100ms

#define RXBUF									data->Rx.buffer
#define RXCMD									data->indexStart+POSID

#define IS10MS									generalFlags.bit.b0
#define IS5MS									generalFlags.bit.b1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;
DMA_HandleTypeDef hdma_i2c1_rx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
u_flag generalFlags;

uint8_t is100ms1 = 10, is1s = 10, is5ms = 20, is20s = 10;

u_conv decom;

uint8_t key;

s_motor MotorL, MotorR;

s_encoder EncoderL, EncoderR;

s_MPU MPU6050;

struct{
	uint8_t isInit;
	uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
	uint8_t state;
	uint32_t timer;
	uint8_t auxString[10];
	uint8_t refreshRate_10ms;
	uint8_t refreshCounter_10ms;

	uint8_t auxXPos;
	uint8_t auxYPos;
}Display;

struct{
	uint16_t raw[ADC_NUM_SENSORS];
	uint16_t value[ADC_NUM_SENSORS];
}Analog;

struct USB_DATA{
	s_commData data;
	uint8_t bytesToTx;
}USB;

struct ESP_DATA{
	_sESP01Handle Config;
	char *ssid;
	char *password;
	char *IP;
	s_commData data;
	uint8_t AT_Rx_data;
	uint8_t bytesToTx;
}ESP;

struct CAR_DATA{
	e_Car_state state;
}Car;

uint8_t	dataRx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void Init_Timing();
void Init_MPU6050();
void Init_Display();
void Init_WiFi();
/* HAL FUNCTIONS */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
/* END HAL FUNCTIONS */

/**
* @brief esta funcion se llama desde la librería "protocol_handler" y
* ejecuta una acción según el comando decodificado
*
* @param data puntero a la estructura de datos de comunicación
*/
void decodeOn_USB(s_commData *data);

/**
 * @brief Funcion llamada automaticamente al haber un cambio de estado en user key, simula una interrupción
 *
 * @param value: Estado en el cual se detectó el cambio
 */
void onKeyChangeState(e_Estados value);

/************************************ FUNCIONES PARA ABSTRACCIÓN DE HARDWARE ************************************/

/**
 * @brief funcion para abstraer la librería "debouncer" del hardware
 *
 * @retval Valor del pin PA0, lee el valor del keyuser
 */
uint8_t KEY_Read_Value();

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

/**
 * @brief esta funcion se llama desde la librería "protocol_handler" y
 * envía un bloque de datos a la librería USB
 *
 * @param data puntero a la estructura de datos de comunicación
 */
void writeOn_USB(s_commData *data);

/**
 * @brief esta funcion se llama desde la librería USB y
 * envía un bloque de datos a la librería "protocol_handler"
 *
 * @param buff recibe un puntero al buffer de datos recibidos
 * @param len tamaño del buffer
 */
void dataRxOn_USB(uint8_t *buff, uint32_t len);

/**
 * @brief abstracción de hardware para setear el valor de PMW del motor izquierdo
 *
 * @param dCycle: valor del PWM a setear en el motor
 */
void Motor_Left_SetPWM(uint16_t dCycle);

/**
 * @brief abstracción de hardware para setear los pines de dirección del motor izquierdo
 *
 * @param dir: Indica la forma en la que hay que setear los pines
 */
void Motor_Left_SetPins(uint8_t pinA, uint8_t pinB);

/**
 * @brief abstracción de hardware para setear el valor de PMW del motor derecho
 *
 * @param dCycle: valor del PWM a setear en el motor
 */
void Motor_Right_SetPWM(uint16_t dCycle);

/**
 * @brief abstracción de hardware para setear los pines de dirección del motor derecho
 *
 * @param dir: Indica la forma en la que hay que setear los pines
 */
void Motor_Right_SetPins(uint8_t pinA, uint8_t pinB);

/**
 * @brief abstracción de hardware de la función HAL_I2C_Mem_Write_DMA()
 * 		Write an amount of data in non-blocking mode with DMA to a specific memory address
 *
 * @param  DevAddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress Internal memory address
 * @param  MemAddSize Size of internal memory address
 * @param  pData Pointer to data buffer
 * @param  Size Amount of data to be sent
 * @retval HAL status
 */
e_system I2C1_DMA_Mem_Write(uint16_t Dev_Address, uint8_t reg, uint8_t *p_Data, uint16_t _Size);

/**
  * @brief abstracción de hardware de la función HAL_I2C_Master_Transmit()
  * 		Transmits in master mode an amount of data in blocking mode.
  *
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
e_system I2C1_Master_Transmit(uint16_t Dev_Address, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout);

/**
  * @brief abstracción de hardware de la función HAL_I2C_Mem_Read()
  * 		Read an amount of data in blocking mode from a specific memory address
  *
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
e_system I2C1_Mem_Read(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout);

/**
  * @brief abstracción de hardware de la función HAL_I2C_Mem_Write()
  * 		Write an amount of data in blocking mode to a specific memory address
  *
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
e_system I2C1_Mem_Write(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout);

void OLED_Print_Data_Task();

void BateryLevel_Set();

int ESP01_UART_Transmit(uint8_t val);

void ESP01_Data_Recived(uint8_t value);

void onESP01ChangeState(_eESP01STATUS esp01State);

void onESP01Debug(const char *dbgStr);

void setESP01_CHPD(uint8_t val);

void writeOn_ESP(s_commData *data);
/************************************ FIN FUNCIONES PARA ABSTRACCIÓN DE HARDWARE ************************************/
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void OLED_Print_Data_Task(){
	if(Display.isInit){
		switch(Display.state){
		case INIT:
			if(HAL_GetTick() - Display.timer > 2000){
				Display_Fill(SSD1306_COLOR_BLACK);
				Display_DrawBitmap(0, 0, status_screen, 128, 64, SSD1306_COLOR_WHITE);
				Display.state = INPUTS;
			}
			break;
		case MENU:

			break;
		case INPUTS:
			BateryLevel_Set();

			Display_DrawBitmap(2, 17, ADC_Blackout, 37, 44, SSD1306_COLOR_BLACK);
			for(uint8_t i = 0; i < 8; i++){
				Display.auxYPos = 107 * Analog.value[i] + 170000;
				Display.auxYPos += 5000;
				Display.auxYPos /= 10000;
				Display.auxXPos = 2 + i * 3;
				Display_DrawLine(Display.auxXPos, 61,  Display.auxXPos, Display.auxYPos, SSD1306_COLOR_WHITE);
				Display_DrawLine(Display.auxXPos+1, 61,  Display.auxXPos+1, Display.auxYPos, SSD1306_COLOR_WHITE);
			}

			if(MPU6050.isInit){
				MPU6050.Acc.x = (MPU6050.Acc.x / 16384.0) * 9.8f;
				MPU6050.Acc.y = (MPU6050.Acc.y / 16384.0) * 9.8f;
				MPU6050.Acc.z = (MPU6050.Acc.z / 16384.0) * 9.8f;
				sprintf((char*)Display.auxString, "Ax:%d", MPU6050.Acc.x);
				Display_SetCursor(25, 17);
				Display_WriteString((char*)Display.auxString, Font_7x10, SSD1306_COLOR_WHITE);
				sprintf((char*)Display.auxString, "Ay:%d", MPU6050.Acc.y);
				Display_SetCursor(25, 34);
				Display_WriteString((char*)Display.auxString, Font_7x10, SSD1306_COLOR_WHITE);
				sprintf((char*)Display.auxString, "Az:%d", MPU6050.Acc.z);
				Display_SetCursor(25, 51);
				Display_WriteString((char*)Display.auxString, Font_7x10, SSD1306_COLOR_WHITE);
				sprintf((char*)Display.auxString, "Gx:%d", MPU6050.Gyro.x);
				Display_SetCursor(73, 17);
				Display_WriteString((char*)Display.auxString, Font_7x10, SSD1306_COLOR_WHITE);
				sprintf((char*)Display.auxString, "Gy:%d", MPU6050.Gyro.y);
				Display_SetCursor(73, 34);
				Display_WriteString((char*)Display.auxString, Font_7x10, SSD1306_COLOR_WHITE);
				sprintf((char*)Display.auxString, "Gz:%d", MPU6050.Gyro.z);
				Display_SetCursor(73, 51);
				Display_WriteString((char*)Display.auxString, Font_7x10, SSD1306_COLOR_WHITE);
			}
			break;
		}
	}

	Display_I2C_Refresh_Ready(TRUE);
}

void BateryLevel_Set(){
	Display_DrawFilledRectangle(3, 4, 6, 9, SSD1306_COLOR_BLACK);
	if(Analog.value[8] >= 3900){
		Display_DrawFilledRectangle(3, 4, 6, 9, SSD1306_COLOR_WHITE);
	}else if(Analog.value[8] >= 3000){
		Display_DrawFilledRectangle(3, 6, 6, 7, SSD1306_COLOR_WHITE);
	}else if(Analog.value[8] >= 2047){
		Display_DrawFilledRectangle(3, 8, 6, 5, SSD1306_COLOR_WHITE);
	}else if(Analog.value[8] >= 1023){
		Display_DrawFilledRectangle(3, 11, 6, 2, SSD1306_COLOR_WHITE);
	}
}

void decodeOn_USB(s_commData *data){
	switch(RXBUF[RXCMD]){
	case GETALIVE:
		data->auxBuffer[0] = ACK;
		comm_sendCMD(data, GETALIVE, &data->auxBuffer[0], 1);
		break;
	case FIRMWARE:
		break;
	case USERTEXT:
		break;
	case ADCSINGLE:
		if(RXBUF[RXCMD + 1] <= 8 && RXBUF[RXCMD + 1] >= 0){
			decom.ui16[0] = Analog.value[RXBUF[RXCMD + 1]];
			data->auxBuffer[0] = RXBUF[RXCMD + 1];
			data->auxBuffer[1] = decom.ui8[0];
			data->auxBuffer[2] = decom.ui8[1];
			comm_sendCMD(data, ADCSINGLE, &data->auxBuffer[0], 3);
		}else{
			comm_sendCMD(data, SYSWARNING, (uint8_t*)"NO ADC", 6);
		}
		break;
	case ADCBLOCK:
		for(uint8_t i = 0; i < ADC_NUM_SENSORS; i++){
			decom.ui16[0] = Analog.value[i];
			data->auxBuffer[i*2] = decom.ui8[0];
			data->auxBuffer[i*2+1] = decom.ui8[1];
		}
		comm_sendCMD(data, ADCBLOCK, data->auxBuffer, 17);
		break;
	case DEBUGER:

		break;
	case SETMOTOR:
		if(RXBUF[RXCMD + 1] == MOTOR_L){
			Motor_Set_Speed(&MotorL, RXBUF[RXCMD + 2]);
			USB.data.auxBuffer[0] = ACK;
			comm_sendCMD(data, SETMOTOR, data->auxBuffer, 1);
		}
		if(RXBUF[RXCMD + 1] == MOTOR_R){
			Motor_Set_Speed(&MotorR, RXBUF[RXCMD + 2]);
			USB.data.auxBuffer[0] = ACK;
			comm_sendCMD(data, SETMOTOR, data->auxBuffer, 1);
		}else{
			comm_sendCMD(data, SYSWARNING, (uint8_t*)"NO MOTOR", 8);
		}
		break;
	case GET_ENCODER:
		if(RXBUF[RXCMD + 1] == ENCODER_L){
			decom.ui16[0] = EncoderL.pps;
			data->auxBuffer[0] = ENCODER_L;
			data->auxBuffer[1] = decom.ui8[0];
			data->auxBuffer[2] = decom.ui8[1];
			comm_sendCMD(data, GET_ENCODER, &data->auxBuffer[0], 3);
		}else if(RXBUF[RXCMD + 1] == ENCODER_R){
			decom.ui16[0] = EncoderR.pps;
			data->auxBuffer[0] = ENCODER_R;
			data->auxBuffer[1] = decom.ui8[0];
			data->auxBuffer[2] = decom.ui8[1];
			comm_sendCMD(data, GET_ENCODER, &data->auxBuffer[0], 3);
		}else{
			comm_sendCMD(data, SYSWARNING, (uint8_t*)"NO ENCODER", 10);
		}
		break;
	case MPUBLOCK:
		decom.i16[0] = MPU6050.Acc.x;
		data->auxBuffer[0] = decom.ui8[0];
		data->auxBuffer[1] = decom.ui8[1];
		decom.i16[0] = MPU6050.Acc.y;
		data->auxBuffer[2] = decom.ui8[0];
		data->auxBuffer[3] = decom.ui8[1];
		decom.i16[0] = MPU6050.Acc.z;
		data->auxBuffer[4] = decom.ui8[0];
		data->auxBuffer[5] = decom.ui8[1];
		decom.i16[0] = MPU6050.Gyro.x;
		data->auxBuffer[6] = decom.ui8[0];
		data->auxBuffer[7] = decom.ui8[1];
		decom.i16[0] = MPU6050.Gyro.y;
		data->auxBuffer[8] = decom.ui8[0];
		data->auxBuffer[9] = decom.ui8[1];
		decom.i16[0] = MPU6050.Gyro.z;
		data->auxBuffer[10] = decom.ui8[0];
		data->auxBuffer[11] = decom.ui8[1];
		comm_sendCMD(data, MPUBLOCK, data->auxBuffer, 12);
		break;
	default:
		comm_sendCMD(data, SYSWARNING, (uint8_t*)"NO CMD", 6);
		break;
	}
}

void onKeyChangeState(e_Estados value){

}

void onESP01ChangeState(_eESP01STATUS esp01State) {
   switch (esp01State) {
        case ESP01_NOT_INIT:
            sprintf((char*)USB.data.auxBuffer, "ESP01: No inicializado");
            break;
        case ESP01_WIFI_DISCONNECTED:
            sprintf((char*)USB.data.auxBuffer, "ESP01: WiFi desconectado");
            break;
        case ESP01_WIFI_NOT_SETED:
            sprintf((char*)USB.data.auxBuffer, "ESP01: WiFi no configurado");
            break;
        case ESP01_WIFI_CONNECTING_WIFI:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Conectando a WiFi...");
            break;
        case ESP01_WIFI_CONNECTED:
            sprintf((char*)USB.data.auxBuffer, "ESP01: WiFi conectado");
            break;
        case ESP01_WIFI_NEW_IP:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Nueva IP asignada");
            break;
        case ESP01_UDPTCP_DISCONNECTED:
            sprintf((char*)USB.data.auxBuffer, "ESP01: UDP/TCP desconectado");
            break;
        case ESP01_UDPTCP_CONNECTING:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Conectando UDP/TCP...");
            break;
        case ESP01_UDPTCP_CONNECTED:
            sprintf((char*)USB.data.auxBuffer, "ESP01: UDP/TCP conectado");
            break;
        case ESP01_SEND_BUSY:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Enviando datos...");
            break;
        case ESP01_SEND_READY:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Listo para enviar");
            break;
        case ESP01_SEND_OK:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Envío OK");
            break;
        case ESP01_SEND_ERROR:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Error al enviar");
            break;
        default:
            sprintf((char*)USB.data.auxBuffer, "ESP01: Estado desconocido");
            break;
    }

    // Cast explícito también aplicado aquí
    comm_sendCMD(&USB.data, USERTEXT, USB.data.auxBuffer, strlen((char*)USB.data.auxBuffer));
}

void onESP01Debug(const char *dbgStr) {
    comm_sendCMD(&USB.data, USERTEXT, (uint8_t *)dbgStr, strlen(dbgStr));
}

void task_10ms(){
	IS10MS = FALSE;

	is100ms1--;
	if(!is100ms1){
		is100ms1 = 10;
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		is1s--;
		if(!is1s){
			is1s = 10;
			Encoder_1s_Elapsed(&EncoderL);
			Encoder_1s_Elapsed(&EncoderR);

			/* ESTABILIZACIÓN DE PWM */
			Motor_Set_MaxValue(&MotorR, (3026/Analog.value[9]));//<! Usamos esto para independizar la salida PWM de la carga de las baterias
			Motor_Set_MaxValue(&MotorL, (3026/Analog.value[9]));
			/* END ESTABILIZACIÓN DE PWM */

			is20s--;
			if(!is20s){
				is20s = 10;

				//comm_sendCMD(&ESP.data, GETALIVE, NULL, 0);
			}
		}
	}

	Display.refreshCounter_10ms--;
	if(!Display.refreshCounter_10ms){ 							//Tasa de refresco variable
		Display.refreshCounter_10ms = Display.refreshRate_10ms;
		OLED_Print_Data_Task();
	}

	ESP01_Timeout10ms();

	Debouncer_Task();

	Motor_Break_Timeout(&MotorL);
	Motor_Break_Timeout(&MotorR);
	Encoder_Task(&EncoderL);
	Encoder_Task(&EncoderR);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  MPU6050.isInit = FALSE;
  Display.isInit = FALSE;
  Display.state = INIT;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* INICIALIZACIÓN DE PROTOCOLO MEDIANTE USB */
  Comm_Init(&USB.data, &decodeOn_USB, &writeOn_USB);
  CDC_Attach_Rx(&dataRxOn_USB);
  /* FIN INICIALIZACIÓN DE PROTOCOLO MEDIANTE USB */

  /* INICIALIZACIÓN DE USER KEY Y DEBOUNCE */
  Debounce_Init();
  key = Debounce_Add(&KEY_Read_Value, &onKeyChangeState);
  /* FIN INICIALIZACIÓN DE USER KEY Y DEBOUNCE */

  /* INICIALIZACIÓN DE MPU6050 */
  Init_MPU6050();
  /* FIN INICIALIZACIÓN DE MPU6050 */

  /* INICIALIZACIÓN DISPLAY*/
  Init_Display();
  /* FIN INICIALIZACIÓN DISPLAY */

  /* INICIALIZACIÓN DE MOTORES Y ENCODERS */
  Motor_Init(&MotorL, &Motor_Left_SetPWM , &Motor_Left_SetPins , htim3.Instance->ARR);
  Motor_Init(&MotorR, &Motor_Right_SetPWM, &Motor_Right_SetPins, htim3.Instance->ARR);

  Encoder_Init(&EncoderL, ENCODER_FASTPPS_COUNTER_10MS);
  Encoder_Init(&EncoderR, ENCODER_FASTPPS_COUNTER_10MS);
  /* FIN INICIALIZACIÓN DE MOTORES Y ENCODERS */

  /* ESP01 INITIALIZATION */
  Init_WiFi();
  /* END ESP01 INITIALIZATION */
  Car.state = IDLE;

  Init_Timing();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  /* USER TASK */
	Comm_Task(&USB.data);
	Comm_Task(&ESP.data);
	Display_UpdateScreen_Task();
	MPU6050_MAF(&MPU6050);
	ESP01_Task();
	/* END USER TASK */

	if(IS10MS){
		task_10ms();
	}

	switch(Car.state){
	case IDLE:

		break;
	case FOLLOW_LINE:

		break;
	case GO_FROM_TO:

		break;
	}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 9;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 23999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 59999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LedStatus_2_Pin|M2_IN2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, M1_IN1_Pin|M1_IN2_Pin|M2_IN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_Pin */
  GPIO_InitStruct.Pin = KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ESP_EN_Pin LedStatus_2_Pin M2_IN2_Pin */
  GPIO_InitStruct.Pin = ESP_EN_Pin|LedStatus_2_Pin|M2_IN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : M1_ENC_A_Pin */
  GPIO_InitStruct.Pin = M1_ENC_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(M1_ENC_A_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : M2_ENC_A_Pin */
  GPIO_InitStruct.Pin = M2_ENC_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(M2_ENC_A_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : M1_IN1_Pin M1_IN2_Pin M2_IN1_Pin */
  GPIO_InitStruct.Pin = M1_IN1_Pin|M1_IN2_Pin|M2_IN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/************************************ USER INIT FUNCTIONS ****************************************/
/* INICIALIZACIÓN DE TIMERS Y PWM*/
void Init_Timing(){
	  if(HAL_TIM_Base_Start_IT(&htim1) != HAL_OK){
		  comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"TIM1 INIT", 9);
	  }
	  if(HAL_TIM_Base_Start_IT(&htim3) != HAL_OK){
	  	  comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"TIM3 INIT BASE", 14);
	  }
	  if(HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK){
		  comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"TIM3 INIT PWM1", 14);
	  }
	  if(HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK){
		  comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"TIM3 INIT PWM2", 14);
	  }
}
/* FIN INICIALIZACIÓN DE TIMERS Y PWM*/
/* INICIALIZACIÓN DE MPU6050 */
void Init_MPU6050(){
	if(HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_ADDR, 1, 10000) != HAL_OK){
		comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"MPU6050 READY", 13);
	}else{
		MPU6050_Set_I2C_Communication(&I2C1_Mem_Write, &I2C1_Mem_Read);
		if(MPU6050_Init(&MPU6050) != SYS_OK){
			comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"MPU6050 INIT", 12);
		}else{
			MPU6050_Calibrate(&MPU6050);
		}
	}
}
/* FIN INICIALIZACIÓN DE MPU6050 */

/* INICIALIZACIÓN DISPLAY*/
void Init_Display(){
	Display.refreshCounter_10ms = DISPLAY_MEDIUM_REFRESH_RATE_10MS;
	Display.refreshRate_10ms = DISPLAY_MEDIUM_REFRESH_RATE_10MS;

	if(HAL_I2C_IsDeviceReady(&hi2c1, SSD1306_I2C_ADDR, 1, 10000) != HAL_OK){
		comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"OLED READY", 10);
	}else{
		Display_Set_I2C_Master_Transmit(&I2C1_DMA_Mem_Write, &I2C1_Master_Transmit);
		if(Display_Init() != SYS_OK){
			comm_sendCMD(&USB.data, SYSERROR, (uint8_t*)"OLED INIT", 9);
		}else{
			Display_DrawBitmap(0,0, uner_logo, 128, 64, 1);
			Display.isInit = TRUE;
			Display.timer = HAL_GetTick();
		}
	}
}
/* FIN INICIALIZACIÓN DISPLAY */

/* INICIALIZACIÓN WIFI */
void Init_WiFi(){
	ESP.password = 	"wlan412877";
	ESP.ssid = 		"InternetPlus_bed788";
	ESP.IP = 		"192.168.1.10";

	Comm_Init(&ESP.data, &decodeOn_USB, &writeOn_ESP);
	ESP.data.isESP01 = TRUE;
	HAL_UART_Receive_IT(&huart1, &ESP.AT_Rx_data, 1);

	ESP.Config.DoCHPD = setESP01_CHPD;
	ESP.Config.WriteUSARTByte = ESP01_UART_Transmit;
	ESP.Config.WriteByteToBufRX = ESP01_Data_Recived;

	ESP01_Init(&ESP.Config);
	ESP01_SetWIFI(ESP.ssid, ESP.password);
	ESP01_StartUDP("192.168.1.10", 30010, 30000);
	//ESP01_AttachChangeState(&onESP01ChangeState);
	ESP01_AttachDebugStr(&onESP01Debug);

	ESP01_setMode(CREATEWIFI);
}
/* END INICIALIZACIÓN WIFI */

/************************************ END USER INIT FUNCTIONS ****************************************/
/***************************************** HAL CALLBACKS *********************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){ //								1/4000s
	if(htim->Instance == TIM1){
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&Analog.raw, ADC_NUM_SENSORS);
		is5ms--;
		if(!is5ms){
			is5ms = 20;
			if(MPU6050.isInit){
				HAL_I2C_Mem_Read_DMA(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_REG, 1, MPU6050.bit_data, 14);
				Display_I2C_DMA_Ready(FALSE);
			}
		}
	}
	if(htim->Instance == TIM3){
		IS10MS = TRUE;
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c){
	if(hi2c->Devaddress == SSD1306_I2C_ADDR){
		if(!MPU6050.isInit){
			Display_I2C_DMA_Ready(TRUE);
		}
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c){
	if(hi2c->Devaddress == MPU6050_ADDR){
		MPU6050_I2C_DMA_Cplt(&MPU6050);
		Display_I2C_DMA_Ready(TRUE);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	ADC_Conversion_Cplt(Analog.raw, Analog.value);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if (GPIO_Pin == M1_ENC_A_Pin){
    	Encoder_Add_Pulse(&EncoderL);
    }
    if (GPIO_Pin == M2_ENC_A_Pin){
    	Encoder_Add_Pulse(&EncoderR);
	}
}
/**************************************** END HAL CALLBACKS ***************************************/

/******************************************** ESP ***********************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART1){
		ESP01_WriteRX(ESP.AT_Rx_data);
		HAL_UART_Receive_IT(&huart1, &ESP.AT_Rx_data, 1);
	}
}

int ESP01_UART_Transmit(uint8_t val){
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE)){
		USART1->DR = val;
		return 1;
	}
	return 0;
}

void ESP01_Data_Recived(uint8_t value){
	ESP.data.Rx.buffer[ESP.data.Rx.write++] = value;
}

void writeOn_ESP(s_commData *data){
	ESP.bytesToTx = data->Tx.write - data->Tx.read;
	if(ESP01_Send(data->Tx.buffer,  data->Tx.read,  ESP.bytesToTx,  RINGBUFFLENGTH) == ESP01_SEND_READY){
		data->Tx.read += ESP.bytesToTx;
	}
}
/******************************************** END ESP ***********************************************/

/*************************************** HARDWARE ABSTRACTION ************************************/
e_system I2C1_DMA_Mem_Write(uint16_t Dev_Address, uint8_t reg, uint8_t *p_Data, uint16_t _Size){
	return (e_system)HAL_I2C_Mem_Write_DMA(&hi2c1, Dev_Address, reg, 1, p_Data, _Size);
}

e_system I2C1_Master_Transmit(uint16_t Dev_Address, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout){
	return (e_system)HAL_I2C_Master_Transmit(&hi2c1, Dev_Address, p_Data, _Size, _Timeout);
}

e_system I2C1_Mem_Write(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout){
	return HAL_I2C_Mem_Write(&hi2c1, Dev_Address, Mem_Adress, Mem_AddSize, p_Data, _Size, _Timeout);
}

e_system I2C1_Mem_Read(uint16_t Dev_Address, uint8_t Mem_Adress, uint8_t Mem_AddSize, uint8_t *p_Data, uint16_t _Size, uint32_t _Timeout){
	return HAL_I2C_Mem_Read(&hi2c1, Dev_Address, Mem_Adress, Mem_AddSize, p_Data, _Size, _Timeout);
}

void setESP01_CHPD(uint8_t val){
	HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, val);
}

uint8_t KEY_Read_Value(){
	return HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
}

void writeOn_USB(s_commData *data){
	if(USB.data.Tx.write > USB.data.Tx.read){
		USB.bytesToTx = USB.data.Tx.write - USB.data.Tx.read;
	}else{
		USB.bytesToTx = RINGBUFFLENGTH - USB.data.Tx.read;
	}
	if(CDC_Transmit_FS(&USB.data.Tx.buffer[USB.data.Tx.read], USB.bytesToTx) == USBD_OK){
		USB.data.Tx.read += USB.bytesToTx;
	}
}

void dataRxOn_USB(uint8_t *buff, uint32_t len){
	if(buff != NULL){
		for(uint16_t i = 0; i < len; i++){
			USB.data.Rx.buffer[USB.data.Rx.write++] = buff[i];
		}
	}
}

void Motor_Left_SetPins(uint8_t pinA, uint8_t pinB){
	HAL_GPIO_WritePin(M1_IN1_GPIO_Port, M1_IN1_Pin, pinA);
	HAL_GPIO_WritePin(M1_IN2_GPIO_Port, M1_IN2_Pin, pinB);
}

void Motor_Right_SetPins(uint8_t pinA, uint8_t pinB){
	HAL_GPIO_WritePin(M2_IN1_GPIO_Port, M2_IN1_Pin, pinA);
	HAL_GPIO_WritePin(M2_IN2_GPIO_Port, M2_IN2_Pin, pinB);
}

void Motor_Left_SetPWM(uint16_t dCycle){
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, dCycle);
}

void Motor_Right_SetPWM(uint16_t dCycle){
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, dCycle);
}
/*************************************** END HARDWARE ABSTRACTION ************************************/
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
