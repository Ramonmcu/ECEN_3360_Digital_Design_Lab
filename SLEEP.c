
/* This code is originally Silicon Labs and copy righted by Silicon Labs’ in 2015 and Silicon Labs’ grants
* permission to anyone to use the software for any purpose, including commercial applications, and to alter
* it, and redistribute it freely subject that the origins is not miss represented, altered source version must
* be plainly marked, and this notice cannot be altered or removed from any source distribution.
*
* Names of routines have been changed to confirm to the naming convention of the application developer.
*
* Routines include:
*
* void blockSleepMode(unsigned int EM);
* void unblockSleepMode(unsigned int EM);
* void enter_sleep(void);
*
*/
//***********************************************************************************
// Include files
//***********************************************************************************
#include "SLEEP.h"
#include "em_emu.h"
#include "em_core.h"
static uint32_t SLEEP_BLOCK_MODE[MAX_EM];

//***********************************************************************************
// functions
//***********************************************************************************
// function parameters:
// function takes one parameter EM of type uint32_t
// function return:
// function return nothing
// blockSleepMode blocks a sleep mode EM in the static array SLEEP_BLOCK_MODE that will be used in the enter sleep
// function to decide on energy mode
void blockSleepMode(uint32_t EM)
{
	CORE_ATOMIC_IRQ_DISABLE();
	SLEEP_BLOCK_MODE[EM]++;
	CORE_ATOMIC_IRQ_ENABLE();
}
// function parameters:
// function takes one parameter EM of type uint32_t
// function return:
// function return nothing
// unblockSleepMode unblocks a sleep mode EM in the static array SLEEP_BLOCK_MODE that will be used in the enter sleep
// function to decide on energy mode
void unblockSleepMode(uint32_t EM)
{
	CORE_ATOMIC_IRQ_DISABLE();
	if(SLEEP_BLOCK_MODE[EM] > 0)
	{
		SLEEP_BLOCK_MODE[EM]--;
	}
	CORE_ATOMIC_IRQ_ENABLE();
}
// function parameters:
// function takes no parameter
// function return:
// function return nothing
// enter_sleep function uses the static array SLEEP_BLOCK_MODE to decided the energy mode the application should be in
void enter_sleep()
{
	if(SLEEP_BLOCK_MODE[EM0] > 0)
	{
		return;
	}
	else if(SLEEP_BLOCK_MODE[EM1] > 0)
	{
		return;
	}
	else if(SLEEP_BLOCK_MODE[EM2] > 0)
	{
		EMU_EnterEM1();
		return;
	}
	else if(SLEEP_BLOCK_MODE[EM3] > 0)
	{
		EMU_EnterEM2(true);
		return;
	}
	else if(SLEEP_BLOCK_MODE[EM4] > 0)
	{
		EMU_EnterEM3(true);
		return;
	}
	else
	{
		EMU_EnterEM3(true);
		return;
	}
}

