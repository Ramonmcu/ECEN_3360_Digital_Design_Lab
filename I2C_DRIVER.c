//***********************************************************************************
// Include files
//***********************************************************************************
#include "I2C_DRIVER.h"
#include "gpio.h"
#include "em_i2c.h"
#include "em_core.h"

//***********************************************************************************
// functions
//***********************************************************************************
// function parameters:
// function takes no parameters
// function return:
// function return nothing
// My_I2C_Init initializes the I2C peripheral
// when initializing the I2C peripheral we set up the frequency of the peripheral and set the pearl gecko to be the master
// the states of the mater and slave needed to be reset before enable the ACK and RXDATAV interrupts
void MY_I2C_Init()
{
	I2C_Init_TypeDef MY_I2C0;

	MY_I2C0.enable = false;
	MY_I2C0.freq = I2C_FREQ_FAST_MAX ;
	MY_I2C0.master = true;
	MY_I2C0.refFreq = 0;
	MY_I2C0.clhr = i2cClockHLRAsymetric;

	I2C_Init(I2C0, &MY_I2C0); //Initialize I2C0

	I2C0->CMD = I2C_CMD_ABORT;

	I2C0->ROUTELOC0 = I2C_ROUTELOC0_SDALOC_LOC15 | I2C_ROUTELOC0_SCLLOC_LOC15; // routing connection

	I2C0->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN; //routing connection

	for(int i=0;i<9;i++) //resets state of SCL
	{
		GPIO_PinOutClear(SCL_port, SCL_pin);
		GPIO_PinOutSet(SCL_port, SCL_pin);
	}

	I2C0->CMD = I2C_CMD_ABORT;

	I2C0->IEN |=(I2C_IF_ACK | I2C_IF_RXDATAV);
}
// function parameters:
// function takes one parameter tx_data of type uint8_t
// function return:
// function return nothing
// MY_I2C_Write using polling to transmit tx_data using the I2C protocol
void MY_I2C_Write(uint8_t tx_data)
{

	I2C0->CMD = I2C_CMD_START; // send start

	I2C0->TXDATA = ((SLAVE_ADD << 1) | WRITE); // send slave address and write

	while( !(I2C0->IF & I2C_IF_ACK))//polling waiting for ack
	{
	}

	I2C0->IFC = I2C_IFC_ACK; // clears ack

	I2C0->TXDATA =  REG_WRITE_CMD;

	while( !(I2C0->IF & I2C_IF_ACK))//polling waiting for ack
	{
	}

	I2C0->IFC = I2C_IFC_ACK; // clears ack

	I2C0->TXDATA = tx_data; // sends data

	I2C0->CMD = I2C_CMD_ACK; // sends ack

	I2C0->CMD = I2C_CMD_STOP; // sends stop

	I2C0->IFC = I2C_IFC_ACK;

}
// function parameters:
// function takes no parameters
// function return:
// function return an uint8_t
// MY_I2C_Read uses polling to read incoming data from the I2C protocol
uint8_t MY_I2C_Read()
{

	uint8_t rx_data;

	I2C0->CMD = I2C_CMD_START; // send start

	I2C0->TXDATA = ((SLAVE_ADD << 1) | WRITE); // send slave address and write

	while( !(I2C0->IF & I2C_IF_ACK))//polling waiting for ack
	{
	}

	I2C0->IFC = I2C_IFC_ACK; // clears ack

	I2C0->TXDATA = REG_READ_CMD;

	while( !(I2C0->IF & I2C_IF_ACK))//polling waiting for ack
	{
	}

	I2C0->IFC = I2C_IFC_ACK; // clears ack

	I2C0->CMD = I2C_CMD_START; // send start

	I2C0->TXDATA = ((SLAVE_ADD << 1) | READ); // sends slave add and read

	while( !(I2C0->IF & I2C_IF_ACK))//polling waiting for ack
	{
	}

	I2C0->IFC = I2C_IFC_ACK; // clears ack

	while( !(I2C0->IF & I2C_IF_RXDATAV))//polling waiting for valid data
	{
	}

	rx_data = I2C0->RXDATA; // retrieves data

	I2C0->CMD = I2C_CMD_NACK; // sends NACK

	I2C0->CMD = I2C_CMD_STOP; // sends stop

	return rx_data;
}

