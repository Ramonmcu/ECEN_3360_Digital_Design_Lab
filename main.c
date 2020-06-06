/***************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for SLSTK3402A
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <String.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_core.h"
#include "bsp.h"
#include "main.h"
#include "gpio.h"
#include "cmu.h"
#include "em_letimer.h"
#include "em_cryotimer.h"
#include "em_i2c.h"
#include "LETIMER.h"
#include "SLEEP.h"
#include "I2C_DRIVER.h"
#include "LEUART.h"
#include "LDMA.h"
#include "capsense.h"
#include "Cryotimer.h"
#include "I2C_DRIVER.h"

//start set up time = .6 us
// 80 ms = full range
// 25 ms = 25c


int main(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
  CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;

  /* Chip errata */
  CHIP_Init();

  /* Init DCDC regulator and HFXO with kit specific parameters */
  EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;

  EMU_DCDCInit(&dcdcInit);
  em23Init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  EMU_EM23Init(&em23Init);
  CMU_HFXOInit(&hfxoInit);

  /* Switch HFCLK to HFXO and disable HFRCO */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

  Scheduler = 0;
  Comp0_event =1;
  Comp1_event =2;
  Tran_event = 4;
  Rec_event = 8;
  Cryo_event = 16;

  uint8_t BUTTON_ZERO =0;

  bool he_touched_me = false;

  uint8_t temp_toggle = 0;

  uint32_t freq= 0;

  uint32_t LETIMER0_EM = EM3;

  /* Initialize clocks */
  freq = cmu_init(LETIMER0_EM);

  /* Initialize LETIMER */
  MY_LETIMER_INIT(freq);

  /* Initialize GPIO */
  gpio_init();

  /*Initialize I2C */
  MY_I2C_Init();

  cryotimer_initialize();

  /*block sleep mode*/
  blockSleepMode(LETIMER0_EM);

  /* Infinite blink loop */
  LETIMER_Enable(LETIMER0,true);
  I2C_Enable(I2C0, true);
  CAPSENSE_Init();

  MY_LEUART();

  LEUART_Enable(LEUART0, leuartEnable);

  //transmit name

  MY_LDMA();



  CRYOTIMER_Enable(true);
  /* Infinite blink loop */
  Scheduler &= ~Tran_event;
  while (1) {

	  if(Scheduler ==0)
	  {
		  enter_sleep();
	  }
	  if(Scheduler & Comp0_event)
	  {
		  CORE_ATOMIC_IRQ_DISABLE();
		  comp0_event();
		  Scheduler &= ~Comp0_event;
		  CORE_ATOMIC_IRQ_ENABLE();
	  }
	  if(Scheduler & Comp1_event)
	  {
		  CORE_ATOMIC_IRQ_DISABLE();
		  comp1_event();
		  LDMA_Initialize(Trans_T, strlen(Trans_T));
		  //LDMA_RX(temp_string, strlen(temp_string));
		  Scheduler &= ~Comp1_event;
		  CORE_ATOMIC_IRQ_ENABLE();
	  }
	  if(Scheduler & Tran_event)
	  {
		  CORE_ATOMIC_IRQ_DISABLE();
		  Scheduler &= ~Tran_event;
		  CORE_ATOMIC_IRQ_ENABLE();
	  }

	  if(Scheduler & Rec_event)
	  {
		  CORE_ATOMIC_IRQ_DISABLE();
		  decode_msg(temp_string);
		  Scheduler &= ~Rec_event;
		  CORE_ATOMIC_IRQ_ENABLE();
	  }
	  if(Scheduler & Cryo_event)
	  {
		  CAPSENSE_Sense();
		  if(CAPSENSE_getPressed(BUTTON_ZERO))
		  {
			  if(he_touched_me)
			  {

			  }
			  else
			  {
				  he_touched_me = true;
				  if(temp_toggle == 0)
				  {
					  temp_toggle = 1;
					  GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeDisabled , false);
					  GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeDisabled , false);
					  GPIO_PinModeSet(SENSENAB_port, SENSENAB_pin, gpioModePushPull, false);
					  LETIMER_Enable(LETIMER0, false);
					  LETIMER0->CNT =0;
					  while(LEUART0->SYNCBUSY);
				  }
				  else if(temp_toggle ==1)
				  {
					  temp_toggle = 0;
					  LETIMER0->CNT =0;
					  while(LETIMER0->SYNCBUSY);
					  GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeWiredAnd , true);
					  GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeWiredAnd , true);
					  for(int i=0;i<9;i++)
					  {
						 GPIO_PinOutClear(SCL_port, SCL_pin);
						 GPIO_PinOutSet(SCL_port, SCL_pin);
					  }
					  GPIO_PinModeSet(SENSENAB_port, SENSENAB_pin, gpioModePushPull, true);
					  LETIMER_Enable(LETIMER0,true);
				  }
			  }
		  }
		  else
		  {
			  he_touched_me = false;
		  }
		  Scheduler &= ~Cryo_event;
	  }
  }
}
