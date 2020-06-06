//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"
#include "LETIMER.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
//***********************************************************************************
// Function return a 32 bit integer value that will be the used as the frequency of the clock for the letimer0 interrupt case
// Function has one input parameter that is used to determine what oscillator will be used for the letimer0
// Function sets up and enables clocks for all peripherals used in the program
//***********************************************************************************
uint32_t  cmu_init(uint32_t EM){

		uint32_t	temp_freq = 0;
		uint32_t 	pre_scalar = 1;


		CMU_HFXOAutostartEnable(true,false,false);
		CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
		CMU_ClockSelectSet(cmuClock_HFPER, cmuSelect_HFRCO);
		CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, LFRCO is enabled
		CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);	// using LFXO or ULFRCO (disables LFRO)
		// Route LF clock to the LF clock tree
		CMU_ClockEnable(cmuClock_HFPER, true);
		CMU_ClockSelectSet(cmuClock_HFPER,cmuSelect_HFRCO);
		//CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

		if(EM  >  3)
		{
			temp_freq = 1000;
			CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
			CMU_OscillatorEnable(cmuOsc_LFXO, false, false);		// enable LFXO
			CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);	// routing clock to LFA
		}
		else
		{
			temp_freq = 32768;
			CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
			CMU_OscillatorEnable(cmuOsc_LFXO, true, true);		// enable LFXO
			CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);// routing clock to LFA

			CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
			CMU_ClockEnable(cmuClock_LFB, true);
			CMU_ClockEnable(cmuClock_LEUART0, true);
		}

		if(DESIRED_PERIOD*temp_freq < LETIMER_MAX) // checks to see if prescalar needs to be greater than 0
		{
			pre_scalar = 0;
			CMU_ClockPrescSet(cmuClock_LETIMER0, pre_scalar);
		}
		else
		{

			while(DESIRED_PERIOD*temp_freq > LETIMER_MAX)
			{
				temp_freq = temp_freq >> 1;
				pre_scalar=pre_scalar << 1;
			}
			CMU_ClockPrescSet(cmuClock_LETIMER0, pre_scalar);
		}


		CMU_ClockEnable(cmuClock_LFA, true);
		CMU_ClockEnable(cmuClock_CORELE, true);
		CMU_ClockEnable(cmuClock_LDMA, true); //??

		CMU_ClockSelectSet(cmuOsc_ULFRCO, cmuSelect_ULFRCO);
		// Peripheral clocks enabled
		CMU_ClockEnable(cmuClock_LETIMER0, true);
		CMU_ClockEnable(cmuClock_GPIO, true);
		CMU_ClockEnable(cmuClock_I2C0, true);
		CMU_ClockEnable(cmuClock_CRYOTIMER, true);


		return temp_freq;

}

