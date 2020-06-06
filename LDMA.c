//***********************************************************************************
// Include files
//***********************************************************************************
#include "LDMA.h"
#include "SLEEP.h"
#include <String.h>
//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************
LDMA_Descriptor_t LDMA_TX_DESC;
LDMA_TransferCfg_t LDMA_TX_CONFIG;
LDMA_Descriptor_t LDMA_RX_DESC;
LDMA_TransferCfg_t LDMA_RX_CONFIG;
//***********************************************************************************
// function prototypes
//***********************************************************************************
//***********************************************************************************
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// My_LDMA initializes the LDMA peripheral
// when initializng the LDMA peripheral the we used the default setting
//***********************************************************************************
void MY_LDMA(void)
{
	LDMA_Init_t MY_LDMA;
	MY_LDMA.ldmaInitCtrlNumFixed = LDMA_CTRL_NUMFIXED_DEFAULT;
	MY_LDMA.ldmaInitCtrlSyncPrsClrEn = 0;
	MY_LDMA.ldmaInitCtrlSyncPrsSetEn = 0;
	MY_LDMA.ldmaInitIrqPriority =3;
	LDMA_Init(&MY_LDMA);

	NVIC_EnableIRQ(LDMA_IRQn);

}
//***********************************************************************************
// function parameters:
// function takes two parameters. The first parameter is the string of text to be transmitted using LEUART. The second parameter is the total number of bytes to be transmitted
// function return:
// function return nothing
// LDMA_Initialize  blocks sleep mode down to EM2 and configures the Descriptor and Configuration for the LDMA channel for transmission.
// After the LDMA channel has been configured, the interrupt for channel zero is enabled and the LDMA starts the transfer
// For LDMA transmission will be using the LEUART peripheral so we enable the TXDMAWU and the TXC interrupt
//***********************************************************************************
void LDMA_Initialize(char* temp, uint8_t total_byte)
{
	blockSleepMode(EM3);

	LEUART0-> CTRL |= (LEUART_CTRL_TXDMAWU);
	while(LEUART0->SYNCBUSY);

	LEUART0-> IEN  |= (LEUART_IEN_TXC);
	while(LEUART0->SYNCBUSY);

	LEUART0-> IEN  &= ~LEUART_IEN_TXBL;


	//Configuration Function
	LDMA_TX_CONFIG = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_LEUART0_TXBL );
	//Descriptor Function
	LDMA_TX_DESC = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(temp,&(LEUART0->TXDATA),total_byte);
	LDMA->IFC = (1<< TX_CH);
	LDMA->IEN = (1<< TX_CH);
	while(LEUART0->SYNCBUSY);

	//LDMA_StartTransfer(RX_CH, &LDMA_RX_CONFIG, &LDMA_RX_DESC);
	LDMA_StartTransfer(TX_CH, &LDMA_TX_CONFIG, &LDMA_TX_DESC);

}
//***********************************************************************************
// function parameters:
// function takes two parameters. The first parameter is the string of text to be be received from the BLE.
// The second parameter is the total number of bytes to be received
// function return:
// function return nothing
// LDMA_RX  blocks enables RX wake-up for the DMA transfer and blocks the RX buffer
// Function also  configures the Descriptor and the Configuration for the RX channel
// After the LDMA channel has been configured, the interrupt for channel one is enabled and the LDMA starts the transfer
// For LDMA receive will be using the LEUART peripheral so we enable the RXDMAWU and unblocks the RX buffer
//***********************************************************************************
void LDMA_RX(char* conv, uint16_t total_byte)
{
	//blockSleepMode(EM3);

	LEUART0-> CTRL |= (LEUART_CTRL_RXDMAWU);
	while(LEUART0->SYNCBUSY);

	LEUART0->CMD |= LEUART_CMD_RXBLOCKDIS;
	while(LEUART0->SYNCBUSY);

	LEUART0->IEN &= ~LEUART_IEN_RXDATAV;
	while(LEUART0->SYNCBUSY);

	LDMA_RX_CONFIG = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_LEUART0_RXDATAV );

	LDMA_RX_DESC = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(LEUART0->RXDATA),conv,strlen(conv));

	LDMA->IFC = (1<< RX_CH);
	LDMA->IEN = (1<< RX_CH);
	while(LEUART0->SYNCBUSY);

	LDMA_StartTransfer(RX_CH, &LDMA_RX_CONFIG, &LDMA_RX_DESC);
}
//***********************************************************************************
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// IRQ Handler hands the interrupts for the LDMA.
// The only interrupts being handled is the interrupt for channel zero and channel 1.
// If the interrupt is caused by either channel zero or 1, the DMAWU is turned off for
// RX and TX respectively
//***********************************************************************************
void LDMA_IRQHandler(void)
{
	CORE_ATOMIC_IRQ_DISABLE();
	//gets interrupt
	uint32_t interupt_cause;
	interupt_cause = LDMA->IF & LDMA->IEN;
	LDMA->IFC = interupt_cause;

	if(interupt_cause & (1<<TX_CH) )
	{
		LEUART0-> CTRL &= ~LEUART_CTRL_TXDMAWU;
		while(LEUART0->SYNCBUSY);
	}
	else if(interupt_cause &(1<<RX_CH))
	{
		LEUART0-> CTRL &= ~LEUART_CTRL_RXDMAWU;
		while(LEUART0->SYNCBUSY);
	}
	CORE_ATOMIC_IRQ_ENABLE();
}

