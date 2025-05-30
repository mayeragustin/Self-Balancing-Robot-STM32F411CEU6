 /*
 * display.c
 *
 *  Created on: May 19, 2025
 *      Author: agust
 */

#include "I2C/OLED/display.h"

/* Write command */
#define SSD1306_WRITECOMMAND(command)      ssd1306_I2C_Write(SSD1306_I2C_ADDR, 0x00, (command))
/* Write data */
#define SSD1306_WRITEDATA(data)            ssd1306_I2C_Write(SSD1306_I2C_ADDR, 0x40, (data))
/* Absolute value */
#define ABS(x)   ((x) > 0 ? (x) : -(x))



typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;



/* Private variable */
static SSD1306_t SSD1306;
//static uint8_t actualYPosition = 0;

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A
#define SSD1306_DEACTIVATE_SCROLL                    0x2E  // Stop scroll
#define SSD1306_ACTIVATE_SCROLL                      0x2F  // Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3  // Set scroll range

#define SSD1306_NORMALDISPLAY     					 0xA6
#define SSD1306_INVERTDISPLAY       				 0xA7

static e_system (*I2C_Master_Transmit)(uint16_t DevAddress, uint8_t reg, uint8_t *pData, uint16_t Size);
static e_system (*I2C_Master_Transmit_Blocking)(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//uint8_t *SSD1306_Buffer;
//uint16_t Buf_size;
uint8_t OLED_DMA_BUFFER[OLED_DMA_BUFFER_SIZE];

typedef enum{
	PAGE,
	DATA
}e_state;

struct{
	uint8_t Ready_To_Refresh;
	uint8_t Ready_DMA;
	uint8_t Ready_MPU_data;
	uint8_t Page;
	e_state state;
	uint8_t PageData[6];
}Update;

void Display_Set_I2C_Master_Transmit(
		e_system (*Master_Transmit)(uint16_t DevAddress, uint8_t reg, uint8_t *pData, uint16_t Size),
		e_system (*Master_Transmit_Blocking)(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)){
	I2C_Master_Transmit = Master_Transmit;
	I2C_Master_Transmit_Blocking = Master_Transmit_Blocking;
}

e_system Display_Init(){
	uint8_t status = 0;

    status += SSD1306_WRITECOMMAND(0xAE);   // Display off
    status += SSD1306_WRITECOMMAND(0x20);   // Set Memory Addressing Mode
    status += SSD1306_WRITECOMMAND(0x10);   // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    status += SSD1306_WRITECOMMAND(0xB0);   // Set Page Start Address for Page Addressing Mode,0-7
    status += SSD1306_WRITECOMMAND(0xC8);   // Set COM Output Scan Direction
    status += SSD1306_WRITECOMMAND(0x00);   // Set low column address
    status += SSD1306_WRITECOMMAND(0x10);   // Set high column address
    status += SSD1306_WRITECOMMAND(0x40);   // Set start line address
    status += SSD1306_WRITECOMMAND(0x81);   // set contrast control register
    status += SSD1306_WRITECOMMAND(0xFF);
    status += SSD1306_WRITECOMMAND(0xA1);   // Set segment re-map 0 to 127
    status += SSD1306_WRITECOMMAND(0xA6);   // Set normal display

    status += SSD1306_WRITECOMMAND(0xA8);   // Set multiplex ratio(1 to 64)
    status += SSD1306_WRITECOMMAND(SSD1306_HEIGHT - 1);

    status += SSD1306_WRITECOMMAND(0xA4);   // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    status += SSD1306_WRITECOMMAND(0xD3);   // Set display offset
    status += SSD1306_WRITECOMMAND(0x00);   // No offset
    status += SSD1306_WRITECOMMAND(0xD5);   // Set display clock divide ratio/oscillator frequency
    status += SSD1306_WRITECOMMAND(0xF0);   // Set divide ratio
    status += SSD1306_WRITECOMMAND(0xD9);   // Set pre-charge period
    status += SSD1306_WRITECOMMAND(0x22);

    status += SSD1306_WRITECOMMAND(0xDA);   // Set com pins hardware configuration
    status += SSD1306_WRITECOMMAND(Display_COM_LR_REMAP << 5 | Display_COM_ALTERNATIVE_PIN_CONFIG << 4 | 0x02);

    status += SSD1306_WRITECOMMAND(0xDB);   // Set vcomh
    status += SSD1306_WRITECOMMAND(0x20);   // 0x20,0.77xVcc
    status += SSD1306_WRITECOMMAND(0x8D);   // Set DC-DC enable
    status += SSD1306_WRITECOMMAND(0x14);   //
    status += SSD1306_WRITECOMMAND(0xAF);   // Turn on Display panel

	status += SSD1306_WRITECOMMAND(SSD1306_DEACTIVATE_SCROLL);

	/* Clear screen */
	Display_Fill(SSD1306_COLOR_BLACK);

	/* Set default values */
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;

	/* Initialized OK */
	SSD1306.Initialized = 1;

	Update.state = PAGE;
	Update.Page = 0;
	Update.Ready_DMA = FALSE;
	Update.Ready_MPU_data = FALSE;
	Update.Ready_To_Refresh = FALSE;

	Update.PageData[0] = 0xB0 + Update.Page;
	Update.PageData[1] = 0x00; //< acá seleccionamos Lower start
	Update.PageData[2] = 0x10; //< y acá upper start

	/* Return OK */
	return status;
}

e_system ssd1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data){
	uint8_t dt[2];
	dt[0] = reg;
	dt[1] = data;
	return I2C_Master_Transmit_Blocking(address, dt, 2, 10);
}

e_system Display_UpdateScreen_Task() {
	if(Update.Ready_To_Refresh && Update.Ready_DMA && SSD1306.Initialized){

		switch(Update.state){
		case PAGE:
			Update.PageData[0] = 0xB0 + Update.Page;     // Set Page
			Update.PageData[1] = 0x00;                   // Lower Column
			Update.PageData[2] = 0x10;                   // Upper Column
			if(I2C_Master_Transmit(SSD1306_I2C_ADDR, 0x00, Update.PageData, 3)		== SYS_OK){
				Update.state = DATA;
				Update.Ready_DMA = FALSE;
			}else{
				return SYS_ERROR;
			}
			break;
		case DATA:
			if(I2C_Master_Transmit(	SSD1306_I2C_ADDR, 0x40,
									&OLED_DMA_BUFFER[SSD1306_WIDTH * Update.Page],
									SSD1306_WIDTH) 									== SYS_OK){
				Update.Page++;
				Update.state = PAGE;
				Update.Ready_DMA = FALSE;
			}else{
				return SYS_ERROR;
			}
			break;
		default:
			Update.Page = 8; //!< fuerzo el if de abajo para reiniciar el sistema
		}
		if(Update.Page > 7){
			Update.Page = 0;
			Update.Ready_To_Refresh = FALSE;
		}
	}

	return SYS_OK;
}

void Display_I2C_DMA_Ready(uint8_t val){
	Update.Ready_DMA = val;
}

void Display_I2C_Refresh_Ready(uint8_t val){
	Update.Ready_To_Refresh = val;
}

void SSD1306_ON(void){
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x14);
	SSD1306_WRITECOMMAND(0xAF);
}

void SSD1306_OFF(void){
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x10);
	SSD1306_WRITECOMMAND(0xAE);
}

/**
 * SET CURSOR POSITION
 */
void Display_SetCursor(uint8_t x, uint8_t y)
{
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

void Display_Fill(SSD1306_COLOR_t color) {
	/* Set memory */
	//memset(OLED_DMA_BUFFER, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, 1024);
	uint32_t i;

	for(i = 0; i < sizeof(OLED_DMA_BUFFER); i++)	// Fill screenbuffer with a constant value (color)
	{
		OLED_DMA_BUFFER[i] = (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF;
	}
}

void Display_Clear (void){
	Display_Fill (0);
}

void Display_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Error */
		return;
	}

	/* Check if pixels are inverted */
	if (SSD1306.Inverted) {
		color = (SSD1306_COLOR_t)!color;
	}

	/* Set color */
	if (color == SSD1306_COLOR_WHITE) {
		OLED_DMA_BUFFER[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		OLED_DMA_BUFFER[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

void Display_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color)
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    //memcpy(&OLED_DMA_BUFFER[OFFSET_DATA], bitmap, 1024);
	/*for(uint16_t i = 0; i<1024; i++){
		OLED_DMA_BUFFER[i] = bitmap[i];
	}*/
    for(int16_t j=0; j<h; j++, y++)
    {
        for(int16_t i=0; i<w; i++)
        {
            if(i & 7)
            {
               byte <<= 1;
            }
            else
            {
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if(byte & 0x80) Display_DrawPixel(x+i, y, color);
        }
    }
}

void Display_ShowBitmap(const unsigned char bitmap[])
{
	Display_Clear();
	Display_DrawBitmap(0, 0, bitmap, 128, 64, 1);
	Display_UpdateScreen_Task();
}

/**
 * WRITE A CHAR INTO THE DISPLAY BUFFER
 */
char Display_WriteChar(char ch, FontDef_t Font, SSD1306_COLOR_t color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
    		SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {

        return 0;	// Not enough space on current line
    }

    for (i = 0; i < Font.FontHeight; i++)	// Translate font to screenbuffer
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
                Display_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t) color);
            else
                Display_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
        }
    }
    SSD1306.CurrentX += Font.FontWidth;	// The current space is now taken

    return ch;	// Return written char for validation
}

/**
 * WRITE STRING INTO THE DISPLAY BUFFER
 */
char Display_WriteString(char* str, FontDef_t Font, SSD1306_COLOR_t color)
{
    while (*str){
        if (Display_WriteChar(*str, Font, color) != *str)	// Write until null-byte
        	return *str;	// Char could not be written

        str++;	// Next char
    }
    return *str;	// Everything ok
}

void Display_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c) {
	int16_t dx, dy, i, tmp;

	if(x0 != x1 && y0 != y1)
		return;

	/* Check for overflow */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}

	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			Display_DrawPixel(x0, i, c);
		}

		/* Return from function */
		return;
	}

	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			Display_DrawPixel(i, y0, c);
		}
	}
	return;
}

void Display_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}

	/* Draw 4 lines */
	Display_DrawLine(x, y, x + w, y, c);         /* Top line */
	Display_DrawLine(x, y + h, x + w, y + h, c); /* Bottom line */
	Display_DrawLine(x, y, x, y + h, c);         /* Left line */
	Display_DrawLine(x + w, y, x + w, y + h, c); /* Right line */
}

void Display_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	uint8_t i;

	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}

	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}

	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		Display_DrawLine(x, y + i, x + w, y + i, c);
	}
}
