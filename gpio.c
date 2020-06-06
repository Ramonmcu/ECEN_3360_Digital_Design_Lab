//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

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
// functions
//***********************************************************************************
void gpio_init(void){

	// Set LED ports to be standard output drive with default off (cleared)
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);

	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);

	//turn on si7021
	GPIO_DriveStrengthSet(SENSENAB_port,gpioDriveStrengthWeakAlternateWeak);
    GPIO_PinModeSet(SENSENAB_port, SENSENAB_pin, gpioModePushPull, SENSENAB_default);

	//turn on SCL and SDA for I2C
	GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeDisabled , SCL_Default);
	GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeDisabled , SDA_Default);

	//set LEUART ports
	GPIO_DriveStrengthSet(UART_RX_port, gpioDriveStrengthWeakAlternateStrong);
	GPIO_DriveStrengthSet(UART_TX_port, gpioDriveStrengthWeakAlternateStrong);
	GPIO_PinModeSet(UART_RX_port, UART_RX_pin, gpioModePushPull, UART_RX_default);
	GPIO_PinModeSet(UART_TX_port, UART_TX_pin, gpioModePushPull, UART_TX_default);
}
