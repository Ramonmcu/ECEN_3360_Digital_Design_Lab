//***********************************************************************************
// Include files
//***********************************************************************************
#include "Si7021.h"
#include "I2C_DRIVER.h"
#include "em_i2c.h"
#include "LEUART.h"
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
// Function returns a 16 bit integer value that can later be converted to actual temperature reading
// Function has no input parameters
// Function follows a Hold Master Mode sequence to read temperature from si7021
// Function returns a 16 bit integer value to be later converted to an actual temperature reading
//***********************************************************************************
uint16_t Read_Temp_Code()
{
		uint16_t MSB = 0;
		uint8_t LSB = 0;
		uint16_t DATA = 0;

		I2C0->CMD = I2C_CMD_START; // send start

		I2C0->TXDATA = ((SLAVE_ADD << 1) | WRITE); // send slave address and write

		while( !(I2C0->IF & I2C_IF_ACK))//polling waiting for ack
		{
		}

		I2C0->IFC = I2C_IFC_ACK; // clears ack

		I2C0->TXDATA = MEASURE_CMD;

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

		MSB = I2C0->RXDATA; // reads MSB

		I2C0 ->CMD = I2C_CMD_ACK; // sends ACK

		while( !(I2C0->IF & I2C_IF_RXDATAV))//polling waiting for valid data
		{
		}


		LSB = I2C0->RXDATA; // read LSB

		I2C0->CMD = I2C_CMD_NACK; // sends NACK

		I2C0->CMD = I2C_CMD_STOP; // sends stop

		MSB= MSB << 8; // shift msb to upper 8 byte

		DATA = MSB | LSB; // gets full 16 bit representation of temp code

		return DATA;
}

//***********************************************************************************
// Function returns a float value that is temperature reading from si7021
// Function has one input parameter that is the temperature code read from the si7021
// Function converts temperature based of conversion given in the si7021 data sheet
// Function returns temperature value reported in celsius that will be sent to the bluetooth
//***********************************************************************************
float Temp_Value(uint16_t Temp_Code)
{
	float Temp_Val = 0;

	Temp_Val = ((175.72 * (Temp_Code))/65536) - 46.85;

	return Temp_Val;
}
//***********************************************************************************
// Function returns a float value that is temperature reading from si7021 either in Celsius or converted to Fahrenheit
// Function has one input parameter that is the temperature reading from the si7021
// Function converts temperature based on a parameter temp_unit. If temp_unit is one output temperature is reported in Celsius
// If temp_unit is zero the temperature is reported in Fahrenheit.
// Function returns the fianl_temp value
//***********************************************************************************
float Temp_Convert(float temp)
{
	float final_temp;
	if(temp_unit == 0)
	{
		final_temp = temp*(9.00/5.00)+32.00;
	}
	else
	{
		final_temp = temp;
	}
	return final_temp;
}

