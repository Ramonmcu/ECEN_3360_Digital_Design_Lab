//***********************************************************************************
// Include files
//***********************************************************************************
#include "LEUART.h"
#include <String.h>
#include "em_core.h"
#include <stdbool.h>
#include "SLEEP.h"
#include "LDMA.h"


LDMA_Descriptor_t LDMA_RX_DESC;
LDMA_TransferCfg_t LDMA_RX_CONFIG;

int i =0;
int length =0;
//***********************************************************************************
// functions
//***********************************************************************************
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// My_LEUART initializes the leuart peripheral as well as enables the IRQ for the leuart
// When initializing the UART peripheral the function sets the Baud Rate of 9600
// It also specifies the number of bits that will be sent, the parity bit and the number of stop bits
// After initializing the UART peripheral, the RX and TX pins are routed and the start frame and sig frame
// are set to be "?" and "#" respectively
void MY_LEUART()
{
	temp_unit = 1;
	LEUART_Init_TypeDef MY_LEUART0;

	MY_LEUART0.baudrate = BAUD_RATE;
	MY_LEUART0.databits = leuartDatabits8;
	MY_LEUART0.enable = leuartDisable;
	MY_LEUART0.parity = leuartNoParity;
	MY_LEUART0.refFreq =0;
	MY_LEUART0.stopbits = leuartStopbits1;

	LEUART_Init(LEUART0, &MY_LEUART0);

	//LEUART0->CTRL |= LEUART_CTRL_LOOPBK;
	while(LEUART0->SYNCBUSY);

	LEUART0->ROUTELOC0 =LEUART_ROUTELOC0_RXLOC_LOC18 |LEUART_ROUTELOC0_TXLOC_LOC18;

	LEUART0->ROUTEPEN |= (LEUART_ROUTEPEN_RXPEN | LEUART_ROUTEPEN_TXPEN);

	//LEUART0 -> IEN |= (LEUART_IEN_TXBL | LEUART_IEN_TXC | LEUART_IEN_STARTF );
	LEUART0->STARTFRAME = 0x3f;

	LEUART0->SIGFRAME = 0x23;
	LEUART0 -> IEN |= LEUART_IEN_SIGF ;
	//LEUART0->CMD = LEUART_CMD_RXBLOCKEN;

	//LEUART0->CTRL |= LEUART_CTRL_SFUBRX;


	NVIC_EnableIRQ(LEUART0_IRQn);


}
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// LEUART0_IRQHandler obtains interrupt cause and then set scheduler event to proper event to be handled later outside of the ISR
// the interrupt handler deals with the TXBL interrupt for the data transfer and the RXDATAVinterrupt that will be used for receiving data
// If the RXDATAV interrupt is seen another function called read_data is called and will read the data stored in the RX buffer
// If the TXBL interrupt is seen the program goes into energy mode 3 and then set ups the scheduler to  transmit data
// The only other interrupt actually being used in the program is the sigf interrupt which lets our program know when the end of the string
// has occurred when this happens the RX buffer is blocked to ensure nothing else goes into the buffer and sets up the scheduler to handle the
// RX event
void LEUART0_IRQHandler()
{
	CORE_ATOMIC_IRQ_DISABLE();

	uint32_t interrupt_cause;

	interrupt_cause = LEUART0->IF & LEUART0->IEN;

	LEUART0->IFC = interrupt_cause;

	if(interrupt_cause & LEUART_IF_RXDATAV)
	{
		temp_string[i] = read_data();
		i++;

	}
	if((interrupt_cause & LEUART_IF_TXBL))
	{
		LEUART0->IEN &= ~(LEUART_IEN_TXBL);
		blockSleepMode(EM3);
		Scheduler |= Tran_event;

	}
	if(interrupt_cause & LEUART_IF_TXC)
	{
		unblockSleepMode(EM3);
	}
	if(interrupt_cause & LEUART_IF_STARTF)
	{

	}
	if(interrupt_cause & LEUART_IF_SIGF)
	{
		Scheduler |= Rec_event;
		unblockSleepMode(EM3);
		LEUART0->IEN &= ~( LEUART_IEN_RXDATAV);
		LEUART0->CMD = LEUART_CMD_RXBLOCKEN;
		length = i;
		i=0;
	}

	CORE_ATOMIC_IRQ_ENABLE();
}
// function parameters:
// function takes one parameter called string which is a pointer to a string of data the will be transmited.
// function return:
// function return nothing
// transmit function reads the length of the string we wish to transmit and then enables the interrupt for TXC.
// transmit will transmit a character to the bluetooth module using the send_char function and then enter sleep until
// the TXC interrupt wakes it up to transmit again
void transmit(char* string)
{
	uint32_t length = strlen(string);
	LEUART0->IEN |= LEUART_IEN_TXC;
	LEUART0->IEN &= ~LEUART_IEN_TXBL;
	for(int i =0; i<length;i++)
	{
		while(!(LEUART0->IF & LEUART_IF_TXBL));
		send_char(string[i]);
		enter_sleep();
	}
}
// function parameters:
// function takes one parameter named trans of type char that is a single character that the program wants to send
// function return:
// function return nothing
// send_char disabled all interrupt and then transmits the char trans if the buffer is not full.
void send_char(char trans)
{
	CORE_ATOMIC_IRQ_DISABLE();
	while(!(LEUART0->IF & LEUART_IF_TXBL));
	LEUART0->TXDATA = trans;
	while(LEUART0->SYNCBUSY);
	CORE_ATOMIC_IRQ_ENABLE();
}
// function parameters:
// function takes no parameters
// function return:
// function return a character
// read_data reads data being received from the bluetooth module
char read_data()
{
	return LEUART0->RXDATA;
}
// function parameters:
// function takes one parameter that is a pointer to the array that stores the received message
// function return:
// function has no return
// function operations:
// function  looks through the array that holds the received message and looks for a sub string of DF,dF,Df,df or Dc,DC,dC,dc
// depending on the sub string seen it will set the variable temp_unit that will be used in the convert function.
// the variable temp_unit is a global variable that is used by other function in the program to decide temperature scaling.
// In this function depending on the type of sub string is contained in the received string of data  temp unit will be either
// a 1 or a zero specifying whether the temperature reading should be given in fahrenheit or celsius.
void decode_msg(char* msg)
{
	for(int j= 0; j< (strlen(msg)-1); j++)
	{
		if(msg[j]== 'd' || msg[j] == 'D')
		{
			if(msg[j+1] == 'C' || msg[j+1] == 'c')
			{
				temp_unit = 1;
			}
			if(msg[j+1] == 'F' || msg[j+1] == 'f')
			{
				temp_unit = 0;
			}
		}
	}
}
