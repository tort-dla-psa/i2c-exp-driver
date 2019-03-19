#include "pwm-exp.h"


fastPWMDriver::fastPWMDriver()
	:drv(PWM_I2C_DEVICE_NUM, PWM_I2C_DEVICE_ADDR)
{}
fastPWMDriver::~fastPWMDriver(){}

// initialize the pwm info struct
void fastPWMDriver::_initPwmSetup(struct pwmSetup *obj) {
	obj->driverNum 		= -1;
	obj->regOffset 		= 0;
	obj->timeStart		= 0;
	obj->timeEnd 		= 0;
	obj->prescale		= 0;
}

// return register offset for specified driver/channel
int fastPWMDriver::_getDriverRegisterOffset(int driverNum, int *addr) {
	// define array of register offsets
	int pwmDriverAddr[PWM_EXP_NUM_CHANNELS] = {
		PWM_EXP_REG_ADDR_DRIVER0,
		PWM_EXP_REG_ADDR_DRIVER1,
		PWM_EXP_REG_ADDR_DRIVER2,
		PWM_EXP_REG_ADDR_DRIVER3,
		PWM_EXP_REG_ADDR_DRIVER4,
		PWM_EXP_REG_ADDR_DRIVER5,
		PWM_EXP_REG_ADDR_DRIVER6,
		PWM_EXP_REG_ADDR_DRIVER7,
		PWM_EXP_REG_ADDR_DRIVER8,
		PWM_EXP_REG_ADDR_DRIVER9,
		PWM_EXP_REG_ADDR_DRIVER10,
		PWM_EXP_REG_ADDR_DRIVER11,
		PWM_EXP_REG_ADDR_DRIVER12,
		PWM_EXP_REG_ADDR_DRIVER13,
		PWM_EXP_REG_ADDR_DRIVER14,
		PWM_EXP_REG_ADDR_DRIVER15
	};

	// find the address
	if(driverNum < 0){
		*addr = PWM_EXP_REG_ADDR_DRIVER_ALL;
	}else if(driverNum < PWM_EXP_NUM_CHANNELS) {
		*addr = pwmDriverAddr[driverNum];
	}else{
		//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:: invalid driver selection, %d\n", driverNum);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

// find time count based on duty fastPWMDriver::fastPWMDriver::percentage(0-100)
// 	return value: the count
int fastPWMDriver::_dutyToCount(float duty) {
	int 	count;
	float 	dutyDecimal 	= duty / 100.0f;
	// convert duty to count
	count 	= (int)round(dutyDecimal * (float)PULSE_TOTAL_COUNT);
	// check for negatives
	if (count < 0) {
		count = 0;
	}
	return (count);
}

// split value into two bytes, and write to the L and H fastPWMDriver::fastPWMDriver::registers(via I2C)
int fastPWMDriver::_writeValue(int addr, int value) {
	int 	status, byte, wrAddr;
	// write first byte to L
	wrAddr 	= addr + REG_OFFSET_BYTE0;
	byte 	= (value & 0xff);
	//printf("Writing to addr: 0x%02x, data: 0x%02x\n", wrAddr, byte);
	drv.write(wrAddr, byte);
	// write second byte to H
	wrAddr 	= addr + REG_OFFSET_BYTE1;
	byte 	= ((value >> 8) & 0xff);
	//printf("Writing to addr: 0x%02x, data: 0x%02x\n", wrAddr, byte);
	drv.write(wrAddr, byte);
	return status;
}

// write ON and OFF time values
int fastPWMDriver::_pwmSetTime(struct pwmSetup *setup) {
	int 	status;
	// set the ON time
	status = _writeValue(setup->regOffset + REG_OFFSET_ON_BYTES, setup->timeStart);
	// set the OFF time
	status |= _writeValue(setup->regOffset + REG_OFFSET_OFF_BYTES, setup->timeEnd);
	return (status);
}

// calculate the ON and OFF time values
void fastPWMDriver::_pwmCalculate(float duty, float delay, struct pwmSetup *setup) {
	int 	countOn, countDelay;
	// to do: add case for 0 duty / 100 duty,
	// set the full OFF / full ON bits
	if (duty == 100) {
		// set LEDs to FULL_ON
		setup->timeStart 	= LED_FULL_VAL;
		setup->timeEnd 		= 0;
	} else if (duty == 0) {
		// set LEDs to FULL_OFF
		setup->timeStart 	= 0;
		setup->timeEnd 		= LED_FULL_VAL;
	} else {
		// find duty and delay in terms of numbers
		countOn 	= _dutyToCount(duty);	
		countDelay 	= _dutyToCount(delay);

		// calculate the time to assert and deassert the signal
		if (countDelay > 0) {
			// with delayed start
			setup->timeStart 	= countDelay - 1;
			setup->timeEnd		= setup->timeStart + countOn;
		} else {
			// no delay - start at 0
			setup->timeStart	= 0;
			setup->timeEnd 		= countOn - 1;
		}

		// take care of the case where delay + duty are more than 100
		if (setup->timeEnd > PULSE_TOTAL_COUNT) {
			setup->timeEnd 	-= PULSE_TOTAL_COUNT;
		}
	}
}

// i2c register writes to set sleep mode
int fastPWMDriver::fastPWMDriver::_pwmSetSleepMode(int bSleepMode) {
	int 	status;
	int 	addr, val;

	// read MODE1 register
	addr 	= PWM_EXP_REG_MODE1;
	drv.readByte(addr, &val);
	//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:_pwmSetSleepMode:: read MODE1 failed\n");

	// set desired sleep mode
	if (bSleepMode == 0) {
		// disable SLEEP mode
		val 	&= ~PWM_EXP_REG_MODE1_SLEEP;
	} else {
		// enable SLEEP mode
		val 	|= PWM_EXP_REG_MODE1_SLEEP; 
	} 

	// write to MODE1 register
	drv.write(addr, val);
	//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:_pwmSetSleepMode:: write to MODE1 failed\n");
	// wait for the oscillator
	usleep(1000);
	return EXIT_SUCCESS;
}

// i2c register read to find if chip (oscillator) is in sleep mode
int fastPWMDriver::_pwmGetSleepMode (int *bSleepMode) {
	int 	status, addr, val;

	// read MODE1 register
	addr 	= PWM_EXP_REG_MODE1;
	drv.readByte(addr, &val);
//	onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmCheckInit:: read MODE1 failed\n");
	// check if oscillator is in sleep mode
	*bSleepMode = ((val & PWM_EXP_REG_MODE1_SLEEP) == PWM_EXP_REG_MODE1_SLEEP) ? 1 : 0;
	return EXIT_SUCCESS;
}

// i2c register writes to set sw reset
int fastPWMDriver::fastPWMDriver::_pwmSetReset() {
	int 	status,	addr, val;

	// read MODE1 register
	addr 	= PWM_EXP_REG_MODE1;
	drv.readByte(addr, &val);
	//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:_pwmSetReset:: read MODE1 register failed\n");
	// enable reset
	val 	|= PWM_EXP_REG_MODE1_RESET; 
	drv.write(addr, val);
	//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:_pwmSetReset:: write to MODE1 register failed\n");
	// wait for the oscillator
	usleep(1000);
	return EXIT_SUCCESS;
}


// check if the oscillator is running
int fastPWMDriver::pwmCheckInit (int *bInitialized) {
	int 	status,	bSleepMode;

	// check if oscillator is in sleep mode
	status 	= _pwmGetSleepMode (&bSleepMode);
	if (status == EXIT_FAILURE) {
		*bInitialized 	= 0;
		return EXIT_FAILURE;
	}

	// check if oscillator is in sleep mode
	*bInitialized 	= (bSleepMode == 1 ? 0 : 1);
	//onionPrint(ONION_SEVERITY_DEBUG, "pwm-exp::pwmCheckInit:: sleep mode is %d, initialized is %d\n", bSleepMode, *bInitialized);
	return status;
}

// run the initial oscillator setup
int fastPWMDriver::pwmDriverInit () {
	int 	status, addr, val;

	//onionPrint(ONION_SEVERITY_INFO, "> Initializing PWM Expansion chip\n");
	// set all channels to 0
	pwmSetupDriver(-1, 0, 0);

	// set PWM drivers to totem pole
	addr 	= PWM_EXP_REG_MODE2;
	val 	= PWM_EXP_REG_MODE2_OUTDRV & 0xff;
	drv.write(addr, val);
//	onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:init:: write to MODE2 failed\n");

	// enable all call
	addr 	= PWM_EXP_REG_MODE1;
	val 	= PWM_EXP_REG_MODE1_ALLCALL & 0xff;
	drv.write(addr, val);
	//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:init:: write to MODE2 failed\n");

	// wait for oscillator
	usleep(1000);

	//// reset fastPWMDriver::fastPWMDriver::MODE1(without sleep)
	// disable SLEEP mode
	status 	= _pwmSetSleepMode(0);
	if (status == EXIT_FAILURE) {
	//	onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:init:: disabling SLEEP mode failed\n");
		return EXIT_FAILURE;
	}

	// enable the reset
	status 	= _pwmSetReset();
	if (status == EXIT_FAILURE) {
		//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:init:: reset failed\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// disable the chip - set oscillator to sleep
int fastPWMDriver::fastPWMDriver::pwmDisableChip() {
	int 	status;

	//onionPrint(ONION_SEVERITY_INFO, "> Oscillator going into sleep mode, PWM disabled\n");

	// enable oscillator sleep mode
	status  = _pwmSetSleepMode(1);
	if (status == EXIT_FAILURE) {
		//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: disabling SLEEP mode failed\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// program the prescale value for desired pwm frequency
int fastPWMDriver::pwmSetFrequency(float freq) {
	int 	status, prescale, addr, val;

	//// calculate the prescale value
	// prescale = round( osc_clk / pulse_count x update_rate ) - 1
	prescale = (int)round( (float)OSCILLATOR_CLOCK/((float)PULSE_TOTAL_COUNT * freq) ) - 1;

	// clamp the value
	if (prescale < PRESCALE_MIN_VALUE) {
		prescale = PRESCALE_MIN_VALUE;
	} else if (prescale > PRESCALE_MAX_VALUE) {
		prescale = PRESCALE_MAX_VALUE;
	}

	// read current prescale value
	addr 	= PWM_EXP_REG_ADDR_PRESCALE;
	drv.readByte(addr, &val);
	//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: read PRESCALE failed\n");

	// only program frequency if new frequency is required
	if (prescale != val) {
		//onionPrint(ONION_SEVERITY_INFO, "> Setting signal frequency to %0.2f Hz (prescale: 0x%02x)\n", freq, prescale);

		//// Go to sleep
		// read MODE1 register
		addr 	= PWM_EXP_REG_MODE1;
		drv.readByte(addr, &val);
		//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: read MODE1 failed\n");

		// enable sleep mode to disable the oscillator
		status  = _pwmSetSleepMode(1);
		if (status == EXIT_FAILURE) {
			//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: disabling SLEEP mode failed\n");
			return EXIT_FAILURE;
		}

		// set the prescale value
		addr 	= PWM_EXP_REG_ADDR_PRESCALE;
		drv.write(addr, prescale);
		//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: setting prescale value failed\n");

		//// wake up - reset sleep
		// disable sleep mode to enable the oscillator
		status  = _pwmSetSleepMode(0);
		if (status == EXIT_FAILURE) {
			//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: disabling SLEEP mode failed\n");
			return EXIT_FAILURE;
		}

		// reset 
		status 	= _pwmSetReset();
		if (status == EXIT_FAILURE) {
			//onionPrint(ONION_SEVERITY_FATAL, "pwm-exp:pwmSetFreq:: reset failed\n");
			return EXIT_FAILURE;
		}
	}

	return status;
}

// perform PWM driver setup based on duty and delay
int fastPWMDriver::pwmSetupDriver(int driverNum, float duty, float delay) {
	int status;
	struct pwmSetup 	setup;

	// initialize the setup structure
	_initPwmSetup(&setup);

	// find driver number and then register offset
	setup.driverNum 	= driverNum;
	status 	= _getDriverRegisterOffset( driverNum, &(setup.regOffset) );

	// find on and off times
	_pwmCalculate(duty, delay, &setup);

	//onionPrint(ONION_SEVERITY_INFO, "> Generating PWM signal with %0.2f%% duty cycle (%0.2f%% delay)\n", duty, delay);
	//printf("PWM: start: %d (0x%04x), stop: %d (0x%04x)\n\n", setup.timeStart, setup.timeStart, setup.timeEnd, setup.timeEnd);

	// write on and off times via i2c
	if (status == EXIT_SUCCESS) {
		status 	= _pwmSetTime(&setup);
	}

	return status;
}


