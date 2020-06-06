//***********************************************************************************
// Include files
//***********************************************************************************
#include "main.h"
#include "em_gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// LED0 pin is
#define	LED0_port		gpioPortF
#define LED0_pin		4
#define LED0_default	false 	// off
// LED1 pin is
#define LED1_port		gpioPortF
#define LED1_pin		5
#define LED1_default	false	// off

#define SCL_port		gpioPortC
#define SCL_pin			11
#define SCL_Default 	0

#define SDA_port		gpioPortC
#define SDA_pin			10
#define SDA_Default 	0

#define SENSENAB_port	gpioPortB
#define SENSENAB_pin	10
#define SENSENAB_default 0

#define	UART_RX_port		gpioPortD
#define UART_RX_pin			11
#define UART_RX_default		true

#define UART_TX_port		gpioPortD
#define UART_TX_pin			10
#define UART_TX_default		true

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_init(void);

