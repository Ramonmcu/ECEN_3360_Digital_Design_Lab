//***********************************************************************************
// Include files
//***********************************************************************************
#include "LETIMER.h"
#include "I2C_DRIVER.h"
#include "SLEEP.h"
#include "em_core.h"
#include "gpio.h"
#include "si7021.h"
#include "stdlib.h"
#include <String.h>
//***********************************************************************************
// functions
//***********************************************************************************
//***********************************************************************************
// function parameters:
// function takes one parameter, freq, of type uint32_t that is used to calculate the comp0 and comp1 values
// function return:
// function return nothing
// My_LETIMER_INIT initializes the letimer0 peripheral as well as enables the IRQ for the letimer
//***********************************************************************************
void MY_LETIMER_INIT(uint32_t freq)
{
	uint32_t Vcomp0;
	uint32_t Vcomp1;


	LETIMER_Init_TypeDef MY_LETIMER0;

	/* LETIMER initialization */
	MY_LETIMER0.enable = false;
	MY_LETIMER0.debugRun = false;
	MY_LETIMER0.comp0Top = true;
	MY_LETIMER0.bufTop = false;
	MY_LETIMER0.out0Pol = 0;
	MY_LETIMER0.out1Pol = 0;
	MY_LETIMER0.ufoa0 = letimerUFOANone;
	MY_LETIMER0.ufoa1 = letimerUFOANone;
	MY_LETIMER0.repMode = letimerRepeatFree;
	MY_LETIMER0.topValue = 0;


	Vcomp0 = DESIRED_PERIOD*freq; //led on
	Vcomp1 = Vcomp0-(LED_ON_TIME*freq); // led off
	MY_LETIMER0.topValue = Vcomp0;

	LETIMER_Init(LETIMER0, &MY_LETIMER0); // initialize LETIMER

	LETIMER_CompareSet(LETIMER0, 0,Vcomp0); //	set LETIMER compare register 0
	LETIMER_CompareSet(LETIMER0, 1, Vcomp1); // set LETIMER compare register 1

	 NVIC_EnableIRQ(LETIMER0_IRQn); // enable processor to see interrupt

	 LETIMER0->IFC = LETIMER_IFC_COMP0 | LETIMER_IFC_COMP1; // clear interrupt flag values
	 LETIMER0->IEN = LETIMER_IEN_COMP0 | LETIMER_IEN_COMP1; // enables interrupts
}
// function parameters:
// function takes one parameter, temp, of type float that is used to convert the decimal value of temp to acii text
// function return:
// function return nothing
// ACII_Convert converts temperature value obtained from the si7021 to an ascii values to be transmitted to the bluetooth module
char* ASCII_Convert(float temp)
{
	uint32_t	temp_init;
	uint32_t	piece;
	uint32_t	Hundreds, Tens, Ones, Tenths, period, sign,unit;


	temp = temp* 10;			//Multiply by 10 to make 123.4 --> 1234

	temp_init = abs(temp);

	piece = temp_init / 1000;			// 1234 / 1000 == 1.234 rounded to 1

	if(piece == 0)
		Hundreds = 0x20;
	else
		Hundreds = piece + 0x30;				//Corresponding ASCII value

	Hundreds = Hundreds;			//Shifts to make it the left most value in the string

	temp_init = temp_init % 1000;		// 1234 % 1000 == 234

	piece = temp_init / 100;			// 234 / 100 == 2.34 rounded to 2

	if(piece == 0)
		Tens = 0x20;
	else
		Tens = piece + 0x30;				//Corresponding ASCII value

	Tens = Tens;				//Shifts to make in the correct position

	temp_init = temp_init % 100;		// 234 % 100 == 34

	piece = temp_init / 10;				// 34 / 10 == 3.4 rounded to 3

	Ones = piece + 0x30;				//Corresponding ASCII value

	Ones = Ones;				//Shifts to make in the correct position

	temp_init = temp_init % 10;			// 34 % 10 == 4

	Tenths= temp_init + 0x30;				//Corresponding ASCII value

	period = 0x2e;

	if(temp_unit == 1)
	{
		unit =  0x43;
	}
	else if(temp_unit == 0)
	{
		unit =  0x46;
	}

	if(temp > 0)
		sign = 0x2b;
	else if(temp < 0)
		sign = 0x2d;

	//Leading zeroes are translated to spaces
	//Temp_Array  = {sign, TEMPV_1, TEMPV_2, TEMPV_3, period, TEMPV_4};
	Temp_Array[0] = sign;
	Temp_Array[1] = Hundreds;
	Temp_Array[2] = Tens;
	Temp_Array[3] = Ones;
	Temp_Array[4] = period;
	Temp_Array[5] = Tenths;
	Temp_Array[6] = unit;

	return Temp_Array;

}
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// LETIMER0_IRQHandler obtains interrupt cause and then set scheduler event to proper event to be handled later outside of the ISR
void LETIMER0_IRQHandler()
{
	CORE_ATOMIC_IRQ_DISABLE();

	uint32_t interrupt_cause;

	interrupt_cause = LETIMER0->IF; // get interrupt flag
	LETIMER0->IFC = interrupt_cause; // clear interrupt flag register

	if(interrupt_cause & LETIMER_IF_COMP0) // checks to see if COMP0 caused interrupt
	{

		Scheduler |= Comp0_event;
	}
	else if (interrupt_cause & LETIMER_IF_COMP1) // checks to see if COMP1 caused interrupt
	{
		Scheduler |= Comp1_event;
	}
	CORE_ATOMIC_IRQ_ENABLE();
}
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// comp0_event turns on the si7021 using GPIO_PinModeSet
void comp0_event()
{
	uint32_t SENSE_ENABLE = 1;

	GPIO_PinModeSet(SENSENAB_port, SENSENAB_pin, gpioModePushPull, SENSE_ENABLE); // enable temp sensor

}
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// comp1_event changes energy levels and turns on the the bus for the I2C protocol and then uses I2C to get a temperature reading from the Si7021
// after receiving the temperature reading the program will start the LDMA RX transfer
// and will use the ACII_Convert function to convert the reading to ascii
// then the bus is turned off and the energy levels are restored
void comp1_event()
{
	unblockSleepMode(EM3);

	blockSleepMode(EM2); // change to energy mode EM1

	// power up bus
	GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeWiredAnd , 1);
	GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeWiredAnd , 1);

	// reset i2c for temp sensor
	for(int i=0;i<9;i++)
	{
		GPIO_PinOutClear(SCL_port, SCL_pin);
		GPIO_PinOutSet(SCL_port, SCL_pin);
	}

	// reset i2c for pearl gecko
	I2C0->CMD = I2C_CMD_ABORT;

	uint16_t read_data;

	float temp = 0;

	read_data = Read_Temp_Code(); // reads 16bit representation of temp

	temp  = Temp_Value(read_data); // converts temp code to decimal number

	temp = Temp_Convert(temp);


	if(temp < COMPARE_TEMP) // compares temp to 15c
	{
		GPIO_PinOutSet(LED0_port, LED0_pin);
	}
	else
	{
		GPIO_PinOutClear(LED0_port, LED0_pin);
	}

	LDMA_RX(temp_string, strlen(temp_string));

	Trans_T = ASCII_Convert(temp);


	// turns off bus
	GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeDisabled , 0);
	GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeDisabled , 0);

	//turns off tmep sensor
	GPIO_PinModeSet(SENSENAB_port, SENSENAB_pin, gpioModePushPull, 0);

	//returns program back to EM2
	unblockSleepMode(EM2);

	blockSleepMode(EM3);
}


