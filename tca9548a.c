/* Nov. 5, 2020
	States for tca9548a.

	Adapted from Fabio Pereira.
	HCS08 Unleashed. 2008
*/

/* Need include for make. */
//#include "mc9s08qe128.h"

/* Function prototype(s). */
unsigned char tca9548a_fsm(unsigned char cntrl_reg);	// Change which i2c channel to use.
/**************************/

/***** Function begins ****/
/* This is for the tca9548a iic expander. */
unsigned char tca9548a_fsm(unsigned char cntrl_reg)
{
	static unsigned char cntrl_reg_label = 0x00;
	static unsigned char done = 0, strt = 1, i2c_state = 0, prev_st = 0;
	
	/*unsigned char i2c_buffer[10]= {
						238,	// Send addr. and write. 	0	
						53,	// Wakeup command msb		1
						23,	// Wakeup command lsb		2
						92,	// Meas. command msb			3 		0
						36,	// Meas. command lsb			4		1
						225,	// Send addr. read			5		2
						176, 	// Sleep command msb			6		3
						152,	// Sleep command lsb			7		4
						0,		// read r.h. data.			8		5
						0 		// read r.h. crc				9		6
						};		*/
	static unsigned char i2c_buffer[10]= 	{
														0xEE,	// Send addr. and write. 	0	
														0x35,	// Wakeup command msb		1
														0x17,	// Wakeup command lsb		2
														0x5C,	// Meas. command msb			3 		0
														0x24,	// Meas. command lsb			4		1
														0xE1,	// Send addr. read			5		2
														0xB0, // Sleep command msb			6		3
														0x98,	// Sleep command lsb			7		4
														0x00,	// read r.h. data.			8		5
														0x00 	// read r.h. crc				9		6
														};	
	
	/* Choose tca channel. */	
	if (cntrl_reg == 0)
		cntrl_reg_label = 0x01;
	else if (cntrl_reg == 1)
		cntrl_reg_label = 0x02;
	else if (cntrl_reg == 2)
		cntrl_reg_label = 0x04;
	else if (cntrl_reg == 3)
		cntrl_reg_label = 0x08;
	else if (cntrl_reg == 4)
		cntrl_reg_label = 0x10;
	else if (cntrl_reg == 5)
		cntrl_reg_label = 0x20;
	else if (cntrl_reg == 6)
		cntrl_reg_label = 0x40;
	else if (cntrl_reg == 7)
		cntrl_reg_label = 0x80;
										
	/*		
		i2c_states:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	Not used.
		4	Not used.
		5	I2C_ACK_QRY
		6	Send iic channel.
		7	I2C_SND_STOP_BIT
	*/

	while(!done)
	{
		if (strt)
			i2c_state = 1;									// go to start state
		else
			i2c_state = 0;									// stay at idle state.

		switch(i2c_state)
		{
			/***************************/
			// I2C in idle state.
			case 0:											// i2c_idl
				prev_st = 0;								// Set previous state.
				break;
			/***************************/
			// Send a start condition.
			case 1:											// i2c_start
				IIC2C = 0xb0;								// Send the start bit.
				if (prev_st == 0 || prev_st == 9 || prev_st == 16)
					i2c_state = 2; 						// send dev. addr with wr bit.
				else if (prev_st == 11)
					i2c_state = 12;						// send dev. addr with rd bit.
				break;
			/***************************/
			// Send a device address and write bit.
			case 2:											// 
				while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
				IIC2D = *(i2c_buffer + 0);				// Send the addr. field with WR bit set (R/W = WR).
				prev_st = 2;
				i2c_state = 5; 							// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Query for ACK response from slave.
			case 5: 											// I2C_ACK_QRY;
				if (IIC2S_RXAK)							/*	If NAK from slave. */
					i2c_state = 0;							//I2C_IDLE;
				else 											// If ACK.
				{
					if (prev_st == 2)						// If previous command is send device address.
						i2c_state = 6;						// Go to send iic channel.	
				}
				break;
			/***************************/
			// Send iic channel.
			case 6:											// 
				while(!IIC2S_TCF);						// Wait until transmission is done.
				IIC2D = cntrl_reg_label;				// Send the tca9548a iic channel..
				//Delay(20);									// Delay 20 ms.
				//snd_cmd = 1;								// Indicates after ACK, send a command.
				prev_st = 6;
				i2c_state = 7; 							// stop bit;			// next state
				break;
			/***************************/
			// Send a stop and go to slave mode.
			case 7:											// 
				IIC2C_MST = 0;								// Send a stop (go to slave mode)
				i2c_state = 0;								// idle
				done = 1;									// Finished
				break;
			/**************************/
		}
	}
	return(done);
}
