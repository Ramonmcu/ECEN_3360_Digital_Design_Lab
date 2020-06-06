//***********************************************************************************
// Include files
//***********************************************************************************
#include "main.h"
#include "em_letimer.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define DESIRED_PERIOD  3	//period of operation
#define LED_ON_TIME		.08	//time for led to be on
#define FREQUENCY 		32768
#define LETIMER_MAX	    65535 //max count of LETIMER
#define RX_DATA 		0x3B
#define COMPARE_TEMP    15

static char Temp_Array[7];
//***********************************************************************************
// function prototypes
//***********************************************************************************
void MY_LETIMER_INIT(uint32_t);
void LETIMER0_IRQHandler(void);
void comp0_event(void);
void comp1_event(void);
char* ASCII_Convert(float);
