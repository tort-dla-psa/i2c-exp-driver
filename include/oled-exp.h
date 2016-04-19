#ifndef _OLED_EXP_H_
#define _OLED_EXP_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include <onion-i2c.h>

// Constants
#define OLED_EXP_ADDR 					0x3C
#define OLED_EXP_DEVICE_NUM 			(I2C_DEFAULT_ADAPTER)
#define OLED_EXP_WIDTH 					128
#define OLED_EXP_HEIGHT 				64
#define OLED_EXP_PAGES 					8
#define OLED_EXP_CHAR_LENGTH 			6
#define OLED_EXP_NUM_CHARS				96

#define OLED_EXP_CHAR_COLUMNS			21
#define OLED_EXP_CHAR_ROWS				8
#define OLED_EXP_CHAR_COLUMN_PIXELS		(OLED_EXP_CHAR_COLUMNS * OLED_EXP_CHAR_LENGTH)

#define OLED_EXP_CONTRAST_MIN			0
#define OLED_EXP_CONTRAST_MAX			255

#define OLED_EXP_DEF_CONTRAST_EXTERNAL_VCC 		0x9f
#define OLED_EXP_DEF_CONTRAST_SWITCH_CAP_VCC 	0xcf

// Registers
#define OLED_EXP_REG_DATA					0x40
#define OLED_EXP_REG_COMMAND				0x80


// Addresses
#define OLED_EXP_ADDR_BASE_PAGE_START		0xB0

// Command Constants
#define OLED_EXP_SET_CONTRAST 				0x81
#define OLED_EXP_DISPLAY_ALL_ON_RESUME 		0xA4
#define OLED_EXP_DISPLAY_ALL_ON 			0xA5
#define OLED_EXP_NORMAL_DISPLAY 			0xA6
#define OLED_EXP_INVERT_DISPLAY 			0xA7
#define OLED_EXP_DISPLAY_OFF 				0xAE
#define OLED_EXP_DISPLAY_ON 				0xAF
#define OLED_EXP_SET_DISPLAY_OFFSET 		0xD3
#define OLED_EXP_SET_COM_PINS 				0xDA
#define OLED_EXP_SET_VCOM_DETECT 			0xDB
#define OLED_EXP_SET_DISPLAY_CLOCK_DIV 		0xD5
#define OLED_EXP_SET_PRECHARGE 				0xD9
#define OLED_EXP_SET_MULTIPLEX 				0xA8
#define OLED_EXP_SET_LOW_COLUMN 			0x00
#define OLED_EXP_SET_HIGH_COLUMN 			0x10
#define OLED_EXP_SET_START_LINE 			0x40
#define OLED_EXP_MEMORY_MODE 				0x20

typedef enum e_OledExpMemoryMode {
	OLED_EXP_MEM_HORIZONTAL_ADDR_MODE 	= 0x00,
	OLED_EXP_MEM_VERTICAL_ADDR_MODE 	= 0x01,
	OLED_EXP_MEM_PAGE_ADDR_MODE	 		= 0x02,
	OLED_EXP_MEM_NUM_MODES 				= 3
} eOledExpMemoryMode;

#define OLED_EXP_COLUMN_ADDR				0x21
#define OLED_EXP_PAGE_ADDR 					0x22
#define OLED_EXP_COM_SCAN_INC 				0xC0
#define OLED_EXP_COM_SCAN_DEC 				0xC8
#define OLED_EXP_SEG_REMAP 					0xA0
#define OLED_EXP_CHARGE_PUMP 				0x8D
#define OLED_EXP_EXTERNAL_VCC 				0x01
#define OLED_EXP_SWITCH_CAP_VCC 			0x02

// Scrolling Constants
#define OLED_EXP_ACTIVATE_SCROLL 						0x2F
#define OLED_EXP_DEACTIVATE_SCROLL 						0x2E
#define OLED_EXP_SET_VERTICAL_SCROLL_AREA 				0xA3
#define OLED_EXP_RIGHT_HORIZONTAL_SCROLL 				0x26
#define OLED_EXP_LEFT_HORIZONTAL_SCROLL 				0x27
#define OLED_EXP_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 	0x29
#define OLED_EXP_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 	0x2A

typedef enum e_OledExpScrollSpeed {
	OLED_EXP_SCROLL_SPEED_5_FRAMES		= 0x00,
	OLED_EXP_SCROLL_SPEED_64_FRAMES		= 0x01,
	OLED_EXP_SCROLL_SPEED_128_FRAMES	= 0x02,
	OLED_EXP_SCROLL_SPEED_256_FRAMES	= 0x03,
	OLED_EXP_SCROLL_SPEED_3_FRAMES		= 0x04,
	OLED_EXP_SCROLL_SPEED_4_FRAMES		= 0x05,
	OLED_EXP_SCROLL_SPEED_25_FRAMES		= 0x06,
	OLED_EXP_SCROLL_SPEED_2_FRAMES		= 0x07,
	OLED_EXP_SCROLL_SPEED_NUM
} eOledExpScrollSpeed;


#define OLED_EXP_READ_LCD_STRING_OPT0					"0x%02x,"
#define OLED_EXP_READ_LCD_STRING_OPT1					"%2x"
#define OLED_EXP_READ_LCD_DATA_IDENTIFIER				"data:"


// Ascii Table
static const uint8_t asciiTable[][OLED_EXP_CHAR_LENGTH] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // SPACE

	{0x00, 0x00, 0x4F, 0x00, 0x00, 0x00}, // !
	{0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
	{0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00}, // #
	{0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00}, // $
	{0x23, 0x13, 0x08, 0x64, 0x62, 0x00}, // %
	{0x36, 0x49, 0x55, 0x22, 0x50, 0x00}, // &
	{0x00, 0x05, 0x03, 0x00, 0x00, 0x00}, // '
	{0x00, 0x1C, 0x22, 0x41, 0x00, 0x00}, // (
	{0x00, 0x41, 0x22, 0x1C, 0x00, 0x00}, // )
	{0x14, 0x08, 0x3E, 0x08, 0x14, 0x00}, // *
	{0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // +
	{0x00, 0x50, 0x30, 0x00, 0x00, 0x00}, // ,
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
	{0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // .
	{0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // /

	{0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00}, // 0
	{0x00, 0x42, 0x7F, 0x40, 0x00, 0x00}, // 1
	{0x42, 0x61, 0x51, 0x49, 0x46, 0x00}, // 2
	{0x21, 0x41, 0x45, 0x4B, 0x31, 0x00}, // 3
	{0x18, 0x14, 0x12, 0x7F, 0x10, 0x00}, // 4
	{0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 5
	{0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00}, // 6
	{0x01, 0x71, 0x09, 0x05, 0x03, 0x00}, // 7
	{0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
	{0x06, 0x49, 0x49, 0x29, 0x1E, 0x00}, // 9

	{0x36, 0x36, 0x00, 0x00, 0x00, 0x00}, // :
	{0x56, 0x36, 0x00, 0x00, 0x00, 0x00}, // ;
	{0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, // <
	{0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // =
	{0x00, 0x41, 0x22, 0x14, 0x08, 0x00}, // >
	{0x02, 0x01, 0x51, 0x09, 0x06, 0x00}, // ?
	{0x30, 0x49, 0x79, 0x41, 0x3E, 0x00}, // @

	{0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00}, // A
	{0x7F, 0x49, 0x49, 0x49, 0x36, 0x00}, // B
	{0x3E, 0x41, 0x41, 0x41, 0x22, 0x00}, // C
	{0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00}, // D
	{0x7F, 0x49, 0x49, 0x49, 0x41, 0x00}, // E
	{0x7F, 0x09, 0x09, 0x09, 0x01, 0x00}, // F
	{0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00}, // G
	{0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00}, // H
	{0x00, 0x41, 0x7F, 0x41, 0x00, 0x00}, // I
	{0x20, 0x40, 0x41, 0x3F, 0x01, 0x00}, // J
	{0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // K
	{0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // L
	{0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00}, // M
	{0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00}, // N
	{0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // O
	{0x7F, 0x09, 0x09, 0x09, 0x06, 0x00}, // P
	{0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00}, // Q
	{0x7F, 0x09, 0x19, 0x29, 0x46, 0x00}, // R
	{0x46, 0x49, 0x49, 0x49, 0x31, 0x00}, // S
	{0x01, 0x01, 0x7F, 0x01, 0x01, 0x00}, // T
	{0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00}, // U
	{0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00}, // V
	{0x3F, 0x40, 0x30, 0x40, 0x3F, 0x00}, // W
	{0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // X
	{0x07, 0x08, 0x70, 0x08, 0x07, 0x00}, // Y
	{0x61, 0x51, 0x49, 0x45, 0x43, 0x00}, // Z

	{0x00, 0x7F, 0x41, 0x41, 0x00, 0x00}, // [
	{0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // backslash
	{0x00, 0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
	{0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
	{0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // _
	{0x00, 0x01, 0x02, 0x04, 0x00, 0x00}, // `

	{0x20, 0x54, 0x54, 0x54, 0x78, 0x00}, // a
	{0x7F, 0x50, 0x48, 0x48, 0x30, 0x00}, // b
	{0x38, 0x44, 0x44, 0x44, 0x20, 0x00}, // c
	{0x38, 0x44, 0x44, 0x48, 0x7F, 0x00}, // d
	{0x38, 0x54, 0x54, 0x54, 0x18, 0x00}, // e
	{0x08, 0x7E, 0x09, 0x01, 0x02, 0x00}, // f
	{0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00}, // g
	{0x7F, 0x08, 0x04, 0x04, 0x78, 0x00}, // h
	{0x00, 0x44, 0x7D, 0x40, 0x00, 0x00}, // i
	{0x20, 0x40, 0x44, 0x3D, 0x00, 0x00}, // j
	{0x7F, 0x10, 0x28, 0x44, 0x00, 0x00}, // k
	{0x00, 0x41, 0x7F, 0x40, 0x00, 0x00}, // l
	{0x78, 0x04, 0x78, 0x04, 0x78, 0x00}, // m
	{0x7C, 0x08, 0x04, 0x04, 0x78, 0x00}, // n
	{0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, // o
	{0x7C, 0x14, 0x14, 0x14, 0x08, 0x00}, // p
	{0x08, 0x14, 0x14, 0x18, 0x7C, 0x00}, // q
	{0x7C, 0x08, 0x04, 0x04, 0x08, 0x00}, // r
	{0x48, 0x54, 0x54, 0x54, 0x20, 0x00}, // s
	{0x04, 0x3F, 0x44, 0x40, 0x20, 0x00}, // t
	{0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00}, // u
	{0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00}, // v
	{0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00}, // w
	{0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, // x
	{0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00}, // y
	{0x44, 0x64, 0x54, 0x4C, 0x44, 0x00}, // z

	{0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // {
	{0x00, 0x00, 0x7F, 0x00, 0x00, 0x00}, // |
	{0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // }
	{0x0C, 0x02, 0x0C, 0x10, 0x0C, 0x00}, // ~
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

// Variables
int 	_vccState;
int 	_memoryMode;

int 	_buffer[OLED_EXP_WIDTH * OLED_EXP_PAGES];
int 	_cursor;

int 	_cursorInRow;
int 	_bColumnsSetForText;
#ifdef __cplusplus
extern "C"{
#endif

//// Functions
int 		_oledSendCommand 			(int command);
int 		_oledSendData				(int data);

// initialization and clearing
int 		oledDriverInit 				();
int 		oledCheckInit 				();
int 		oledClear 					();

// configuration
int 		oledSetDisplayPower			(int bPowerOn);
int 		oledSetDisplayMode			(int bInvert);
int 		oledSetBrightness 			(int brightness);
int 		oledSetDim 					(int dim);
int 		oledSetMemoryMode			(int mode);

int 		oledSetCursor				(int row, int column);
int 		oledSetCursorByPixel 		(int row, int pixel);

int 		oledSetColumnAddressing 	(int startPixel, int endPixel);
int 		oledSetTextColumns			();
int 		oledSetImageColumns 		();

// writing to the display
int 		oledWriteChar 				(char c);
int 		oledWrite 					(char *msg);
int 		oledWriteByte				(int byte);

int 		oledDraw 					(uint8_t *buffer, int bytes);

// scroll the display
int 		oledScroll 					(int direction, int scrollSpeed, int startPage, int stopPage);
int 		oledScrollDiagonal 			(int direction, int scrollSpeed, int fixedRows, int scrollRows, int verticalOffset, int startPage, int stopPage);
int 		oledScrollStop 				();

// reading lcd data
int 		oledReadLcdFile				(char* file, uint8_t *buffer);
int 		oledReadLcdData				(char* data, uint8_t *buffer);

// writing to the buffer
int 		oledDisplay 				();
int 		oledPrintChar 				(char c);

int 		oledLineScroll 				();
int 		oledNewLine 				();
int 		oledPrintLine 				();

#ifdef __cplusplus
}
#endif
#endif // _OLED_EXP_H_
