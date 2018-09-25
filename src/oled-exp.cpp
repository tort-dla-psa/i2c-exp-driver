#include "../include/onion-i2c.h"
#include "../include/oled-exp.h"

fastOledDriver::fastOledDriver()
	:i2c_driver(OLED_EXP_DEVICE_NUM, OLED_EXP_ADDR)
{
	_bufsize = OLED_EXP_WIDTH * OLED_EXP_PAGES;
	_buffer = new uint8_t[_bufsize];
}


fastOledDriver::fastOledDriver(fastDebuger debuger)
	:i2c_driver(OLED_EXP_DEVICE_NUM, OLED_EXP_ADDR),debuger(debuger)
{
	_bufsize = OLED_EXP_WIDTH * OLED_EXP_PAGES;
	_buffer = new uint8_t[_bufsize];
}

fastOledDriver::fastOledDriver(fastI2CDriver i2c_driver)
	:i2c_driver(OLED_EXP_DEVICE_NUM, OLED_EXP_ADDR)
{
	_bufsize = OLED_EXP_WIDTH * OLED_EXP_PAGES;
	_buffer = new uint8_t[_bufsize];
}

fastOledDriver::fastOledDriver(fastI2CDriver i2c_driver, fastDebuger debuger)
	:i2c_driver(i2c_driver),debuger(debuger)
{
	_bufsize = OLED_EXP_WIDTH * OLED_EXP_PAGES;
	_buffer = new uint8_t[_bufsize];
}

fastOledDriver::~fastOledDriver() {
	delete[] _buffer;
}

void fastOledDriver::setDebuger(fastDebuger debuger) {}

fastDebuger fastOledDriver::getDebuger() const {
	return fastDebuger();
}

void fastOledDriver::init() {
	debuger.print(ONION_SEVERITY_INFO, "> initializing display\n");

	memset(_buffer, 0, _bufsize);	// initialize the buffer
	_cursorRow = 0;					// initialize the cursor
	_cursorChar = 0;				// initialize the row cursor

	// set defaults
	_vccState = OLED_EXP_SWITCH_CAP_VCC;

	// initialize the screen for 128x64
	sendCommand(OLED_EXP_DISPLAY_OFF);

	sendCommand(OLED_EXP_SET_DISPLAY_CLOCK_DIV);
	sendCommand(0x80); 								// The suggested ratio 0x80
	sendCommand(OLED_EXP_SET_MULTIPLEX);
	sendCommand(0x3F);
	sendCommand(OLED_EXP_SET_DISPLAY_OFFSET);
	sendCommand(0x00);								// no offset
	sendCommand(OLED_EXP_SET_START_LINE | 0x00); 	// Set start line to line #0
	sendCommand(OLED_EXP_CHARGE_PUMP);
	if (_vccState == OLED_EXP_EXTERNAL_VCC) {
		sendCommand(0x10);
	} else {
		sendCommand(0x14);
	}

	sendCommand(OLED_EXP_MEMORY_MODE);
	sendCommand(OLED_EXP_MEM_HORIZONTAL_ADDR_MODE);
	sendCommand(OLED_EXP_SEG_REMAP | 0x01);
	sendCommand(OLED_EXP_COM_SCAN_DEC);
	sendCommand(OLED_EXP_SET_COM_PINS);
	sendCommand(0x12);
	sendCommand(OLED_EXP_SET_CONTRAST);
	if (_vccState == OLED_EXP_EXTERNAL_VCC) {
		sendCommand(OLED_EXP_DEF_CONTRAST_EXTERNAL_VCC);
	} else {
		sendCommand(OLED_EXP_DEF_CONTRAST_SWITCH_CAP_VCC);
	}

	sendCommand(OLED_EXP_SET_PRECHARGE);
	if (_vccState == OLED_EXP_EXTERNAL_VCC) {
		sendCommand(0x22);
	} else {
		sendCommand(0xF1);
	}
	sendCommand(OLED_EXP_SET_VCOM_DETECT);
	sendCommand(0x40);
	sendCommand(OLED_EXP_DISPLAY_ALL_ON_RESUME);
	sendCommand(OLED_EXP_NORMAL_DISPLAY);
	sendCommand(OLED_EXP_SEG_REMAP);
	sendCommand(OLED_EXP_COM_SCAN_INC);
	sendCommand(OLED_EXP_DISPLAY_ON);
	clear();
}

void fastOledDriver::draw() {
	sendCommand(OLED_EXP_COLUMN_ADDR);
	sendCommand(0x00);								// Column start address (0x00 = reset)
	sendCommand(OLED_EXP_WIDTH - 1);				// Column end address
	sendCommand(OLED_EXP_PAGE_ADDR);
	sendCommand(0x00);								// Page start address (0x00 = reset)
	sendCommand(OLED_EXP_PAGES - 1);				// Page end address
	int controlRegister = 0x40;
	for (unsigned int i = 0; i < _bufsize; i += I2C_BUFFER_SIZE) {
		i2c_driver.write(controlRegister, _buffer + i, I2C_BUFFER_SIZE);
	}
}

void fastOledDriver::setDisplayMode(bool bInvert) {
	debuger.print((bInvert ? ONION_SEVERITY_INFO : ONION_SEVERITY_DEBUG),
		"> Setting display mode to %s\n",
		(!bInvert ? "inverted" : "normal")
	);
	sendCommand(bInvert ? OLED_EXP_NORMAL_DISPLAY : OLED_EXP_INVERT_DISPLAY);
}

inline void fastOledDriver::sendCommand(uint8_t command) {
	i2c_driver.write(OLED_EXP_REG_COMMAND, command);
}

// clear the OLED screen
void fastOledDriver::clear() {
	int 	charRow, pixelCol;
	debuger.print(ONION_SEVERITY_DEBUG, "> clearing display\n");
	// set the column addressing for the full width
	setImageColumns();
	// display off
	sendCommand(OLED_EXP_DISPLAY_OFF);
	// write a blank space to each character
	uint8_t empty_row[OLED_EXP_WIDTH];
	memset(empty_row, 0x00, OLED_EXP_WIDTH);
	for (charRow = 0; charRow < OLED_EXP_CHAR_ROWS; charRow++) {
		setCursor(charRow, 0);
		i2c_driver.write(OLED_EXP_REG_DATA, &empty_row[0], OLED_EXP_WIDTH);
	}
	// display on
	sendCommand(OLED_EXP_DISPLAY_ON);
	// reset the cursor to (0, 0)
	setCursor(0, 0);
}

void fastOledDriver::setCursor(unsigned int row, unsigned int column) {
	debuger.print(ONION_SEVERITY_DEBUG, "> Setting cursor to (%d, %d)\n", row, column);
	// check the inputs
	if (row >= OLED_EXP_CHAR_ROWS) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Attempting to set cursor to invalid row '%d'\n", row);
		return;
	}
	if (column >= OLED_EXP_CHAR_COLUMNS) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Attempting to set cursor to invalid column '%d'\n", column);
		return;
	}
	// set page address
	sendCommand(OLED_EXP_ADDR_BASE_PAGE_START + row);
	// set column lower address
	sendCommand(OLED_EXP_SET_LOW_COLUMN + (OLED_EXP_CHAR_LENGTH * column & 0x0F));
	// set column higher address
	sendCommand(OLED_EXP_SET_HIGH_COLUMN + ((OLED_EXP_CHAR_LENGTH * column >> 4) & 0x0F));
}

inline void fastOledDriver::sendData(uint8_t data) {
	i2c_driver.write(OLED_EXP_REG_DATA, data);
}

void fastOledDriver::setDisplayPower(bool bPowerOn) {
	uint8_t	cmd;
	debuger.print(ONION_SEVERITY_INFO, "> Setting display to %s\n", (bPowerOn == 1 ? "ON" : "OFF"));
	// set the command code
	if (bPowerOn) {
		cmd = OLED_EXP_DISPLAY_ON;
	} else {
		cmd = OLED_EXP_DISPLAY_OFF;
	}
	// send the command code
	sendCommand(cmd);
}

void fastOledDriver::setBrightness(unsigned int brightness) {
	// clamp the brightness to the lower and upper limits
	if (brightness < OLED_EXP_CONTRAST_MIN) {
		brightness = OLED_EXP_CONTRAST_MIN;
	}
	if (brightness > OLED_EXP_CONTRAST_MAX) {
		brightness = OLED_EXP_CONTRAST_MAX;
	}
	// send the command
	debuger.print(ONION_SEVERITY_DEBUG, "> Setting display brightness to %d/%d\n", brightness, OLED_EXP_CONTRAST_MAX);
	sendCommand(OLED_EXP_SET_CONTRAST);
	sendCommand(brightness);
}

inline unsigned int fastOledDriver::getBrightness() const {
	return brightness;
}

void fastOledDriver::setDim(bool dim) {
	int 	brightness;
	// set the brightness based on the dimness setting
	if (dim) {
		// dim 
		brightness = OLED_EXP_CONTRAST_MIN;
		debuger.print(ONION_SEVERITY_INFO, "> Dimming display\n");
	} else {
		// normal
		brightness = OLED_EXP_DEF_CONTRAST_SWITCH_CAP_VCC;
		if (_vccState == OLED_EXP_EXTERNAL_VCC) {
			brightness = OLED_EXP_DEF_CONTRAST_EXTERNAL_VCC;
		}
		debuger.print(ONION_SEVERITY_INFO, "> Setting normal display brightness\n");
	}
	// send the command
	this->dim = dim;
	setBrightness(brightness);
}
bool fastOledDriver::getDim() const {
	return false;
}
// Check if i2c transmissions to the display work
bool fastOledDriver::checkInit() {
	int data;
	return i2c_driver.readByte(OLED_EXP_REG_COMMAND, &data);
}

// set the display's memory mode
void fastOledDriver::setMemoryMode(int mode) {
	// check the input
	if (mode != OLED_EXP_MEM_HORIZONTAL_ADDR_MODE &&
		mode != OLED_EXP_MEM_VERTICAL_ADDR_MODE &&
		mode != OLED_EXP_MEM_PAGE_ADDR_MODE
		) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Attempting to set invalid memory mode (0x%02x)\n", mode);
		return;
	}
	// send the command
	sendCommand(OLED_EXP_MEMORY_MODE);
	sendCommand(mode);
	// store the memory mode
	_memoryMode = mode;
}

int fastOledDriver::getMemoryMode() const {
	return 0;
}

// set the OLED's cursor (according to character rows and dislay pixels)
void fastOledDriver::setCursorByPixel(unsigned int row, unsigned int pixel) {
	debuger.print(ONION_SEVERITY_DEBUG, "> Setting cursor to row %d, pixel %d)\n", row, pixel);

	// check the inputs
	if (row >= OLED_EXP_CHAR_ROWS) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Attempting to set cursor to invalid row '%d'\n", row);
		return;
	}
	if (pixel >= OLED_EXP_WIDTH) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Attempting to set cursor to invalid pixel '%d'\n", pixel);
		return;
	}
	//// set the cursor
	// set page address
	sendCommand(OLED_EXP_ADDR_BASE_PAGE_START + row);
	// set pixel lower address
	sendCommand(OLED_EXP_SET_LOW_COLUMN + (pixel & 0x0F));
	// set pixel higher address
	sendCommand(OLED_EXP_SET_HIGH_COLUMN + ((pixel >> 4) & 0x0F));
	return;
}



// set the horizontal addressing
void fastOledDriver::setColumnAddressing(unsigned int startPixel, unsigned int endPixel) {
	// check the inputs
	if (startPixel >= OLED_EXP_WIDTH || startPixel >= endPixel) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Invalid start pixel (%d) for column address setup\n", startPixel);
		return;
	}
	if (endPixel >= OLED_EXP_WIDTH) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: Invalid end pixel (%d) for column address setup\n", endPixel);
		return;
	}

	// set column addressing
	sendCommand(OLED_EXP_COLUMN_ADDR);
	sendCommand(startPixel);	// start pixel setup
	sendCommand(endPixel);		// end pixel setup
}

// set the horizontal addressing for text (fit 6-pixel wide characters onto 128 pixel line)
void fastOledDriver::setTextColumns() {
	// set the column addressing for text mode
	setColumnAddressing(0, OLED_EXP_CHAR_COLUMN_PIXELS - 1);
	_bColumnsSetForText = 1;
}

// set the horizontal addressing for images (rows go from pixel 0 to 127, full width)
void fastOledDriver::setImageColumns() {
	// set the column addressing to full width
	setColumnAddressing(0, OLED_EXP_WIDTH - 1);
	_bColumnsSetForText = 0;
}

//// writing functions ////
// write a character directly to the OLED display (at the OLED cursor's current position)
void fastOledDriver::writeChar(const char c) {
	int 	idx;
	int 	charIndex = (int)c - 32;
	// ensure character is in the table
	if (charIndex >= 0 && charIndex < sizeof(asciiTable) / sizeof(asciiTable[0])) {
		// write the data for the character
		i2c_driver.write(OLED_EXP_REG_DATA, &asciiTable[charIndex][0], OLED_EXP_CHAR_LENGTH);
		debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "\twriting '%c' to column %d\n", c, _cursorChar);
		// increment row cursor
		_cursorChar = (_cursorChar == OLED_EXP_CHAR_COLUMNS - 1) ? 0 : _cursorChar+1;
	}
}

// write a string message directly to the display
void fastOledDriver::write(const char *msg) {
	int 	status;
	int 	idx, i;
	debuger.print(ONION_SEVERITY_INFO, "> Writing '%s' to display\n", msg);
	// set addressing mode to page
	//setMemoryMode(OLED_EXP_MEM_PAGE_ADDR_MODE);	// want automatic newlines enabled
	// set column addressing to fit 126 characters that are 6 pixels wide
	if (_bColumnsSetForText == 0) {
		setTextColumns();
	}
	size_t length = strlen(msg);
	// write each character
	for (idx = 0; idx < length; idx++) {
		// check for newline character
		if (msg[idx] == '\\') {
			if (idx + 1 < length && msg[idx + 1] == 'n' && msg[idx - 1] != '\\') {
				// move the cursor to the next row
				for (i = _cursorChar; i < OLED_EXP_CHAR_COLUMNS; i++) {
					writeChar(' ');
				}
				// increment past this newline character (skip next index)
				idx++;
			} else if (msg[idx - 1] != '\\'){
				writeChar(msg[idx]);
			}
		} else {
			writeChar(msg[idx]);
		}
	}
	// reset the column addressing
	setImageColumns();
}

// write a buffer directly to the display
void fastOledDriver::draw(uint8_t *buffer, int bytes) {
	int 	idx;
	debuger.print(ONION_SEVERITY_INFO, "> Writing buffer data to display\n");
	// set the column addressing for the full width
	setImageColumns();
	// set addressing mode to horizontal (automatic newline at the end of each line)
	setMemoryMode(OLED_EXP_MEM_HORIZONTAL_ADDR_MODE);
	// write each byte
	/*
	for (idx = 0; idx < bytes; idx++) {
		debuger.print(ONION_SEVERITY_DEBUG_EXTRA, ">> writing byte %d 0x%02x\n", idx, buffer[idx]);
		sendData(buffer[idx]);
	}*/
	//write buffer
	i2c_driver.write(OLED_EXP_REG_DATA, buffer, bytes);
	/*
	unsigned short size = 63;
	for (unsigned short i = 0; i < _bufsize; i += size) {
		i2c_writeBuffer();
	}*/
	//i2c_writeBuffer
}


//// scrolling functions ////
// horizontal scrolling
//	direction 	scrolling
//	0 			left
//	1 			right
void fastOledDriver::scroll(bool direction, int scrollSpeed, int startPage, int stopPage) {
	int 	scrollMode;
	debuger.print(ONION_SEVERITY_INFO, "> Enabling horizontal scrolling to the %s\n", (direction == 1 ? "right" : "left"));
	// read the direction
	if (direction) {
		scrollMode = OLED_EXP_RIGHT_HORIZONTAL_SCROLL;
	} else {
		scrollMode = OLED_EXP_LEFT_HORIZONTAL_SCROLL;
	}
	// send the commands
	sendCommand(scrollMode);
	sendCommand(0x00);			// dummy byte
	sendCommand(startPage);		// start page addr (0 - 7)
	sendCommand(scrollSpeed);	// time interval between frames
	sendCommand(stopPage);		// end page addr (must be greater than start)
	sendCommand(0x00);			// dummy byte (must be 0x00)
	sendCommand(0xff);			// dummy byte (must be 0xff)
	sendCommand(OLED_EXP_ACTIVATE_SCROLL);
}

// diagonal scrolling
//	direction 	scrolling
//	0 			left
//	1 			right
void fastOledDriver::scrollDiagonal(bool direction, int scrollSpeed, int fixedRows, int scrollRows, int verticalOffset, int startPage, int stopPage) {
	int 	scrollMode;
	debuger.print(ONION_SEVERITY_INFO, "> Enabling diagonal scrolling to the %s\n", (direction == 1 ? "right" : "left"));
	// read the direction
	if (direction) {
		scrollMode = OLED_EXP_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL;
	} else {
		scrollMode = OLED_EXP_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL;
	}
	//// send the commands
	// setup the vertical scrolling
	sendCommand(OLED_EXP_SET_VERTICAL_SCROLL_AREA);
	sendCommand(fixedRows);		// number of fixed rows
	sendCommand(scrollRows);	// number of rows in scroll area
	// setup the horizontal scrolling
	sendCommand(scrollMode);
	sendCommand(0x00);			// dummy byte
	sendCommand(startPage);		// start page addr (0 - 7)
	sendCommand(scrollSpeed);	// time interval between frames
	sendCommand(stopPage);		// end page addr (must be greater than start)
	sendCommand(verticalOffset);// number of rows to scroll by
	sendCommand(OLED_EXP_ACTIVATE_SCROLL);
}

// disable scrolling
void fastOledDriver::scrollStop() {
	debuger.print(ONION_SEVERITY_INFO, "> Disabling scrolling\n");
	// send the command
	sendCommand(OLED_EXP_DEACTIVATE_SCROLL);
}

//// reading lcd data ////
// read a file with hex data 
void fastOledDriver::readLcdFile(char* file, uint8_t *buffer) {
	int 	idx;
	FILE 	*fp;
	unsigned int	val;
	// open the file
	fp = fopen(file, "r");
	if (fp == NULL) {
		debuger.print(ONION_SEVERITY_FATAL, "ERROR: cannot open file '%s'\n", file);
		return;
	}
	// read each byte, add to the buffer
	idx = 0;
	while (fscanf(fp, OLED_EXP_READ_LCD_STRING_OPT1, &val) > 0) {
		buffer[idx] = (uint8_t)val;
		idx++;
	}
	// close the file
	fclose(fp);
}

// read hex data from a string
void fastOledDriver::readLcdData(char* data, uint8_t *buffer) {
	int 	idx, i;
	unsigned int	val;
	//DBG
	debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "\n\n\ndata is of length %d, data:\n%s\n", strlen(data), data);
	// read each byte, add to the buffer
	idx = 0;
	while (sscanf(data, OLED_EXP_READ_LCD_STRING_OPT1, &val) > 0) {
		debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "idx: %d, val: 0x%02x\n", idx, val);
		buffer[idx] = (uint8_t)val;
		// advance the buffer index
		idx++;
		// advance the string by 2 characters
		memmove(data, data + 2, strlen(data));
	}
	//DBG
	debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "buffer: idx is %d, buffer: \n", idx);
	for (i = 0; i < idx; i++) {
		debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "idx: %d, 0x%02x\n", i, buffer[i]);
	}
	debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "\n");
}

// write a character to the buffer
void fastOledDriver::printChar(char c) {
	int 	charIndex = (int)c - 32;
	if (c == '\n') {
	} else if (charIndex >= 0 && charIndex < sizeof(asciiTable) / sizeof(asciiTable[0])) {
		// At the end of the screen
		if (_cursor >= OLED_EXP_WIDTH * OLED_EXP_PAGES - 2) {
			lineScroll();
		}
		_cursor++;
		if (_cursor % OLED_EXP_WIDTH >= 126) {
			_cursor += 2;
		}
		memcpy(_buffer + _cursor, asciiTable[charIndex], OLED_EXP_CHAR_LENGTH);
		_cursor += OLED_EXP_CHAR_LENGTH;
	}
}

void fastOledDriver::lineScroll() {
	memmove(_buffer, _buffer + (OLED_EXP_WIDTH * OLED_EXP_CHAR_LENGTH), OLED_EXP_WIDTH * (OLED_EXP_PAGES - 1) * OLED_EXP_CHAR_LENGTH);
	_cursor = OLED_EXP_WIDTH * (OLED_EXP_PAGES - 1);
}