/* Nov. 17, 2020
	States for holtek ht16k33.

	Adapted from Fabio Pereira.
	HCS08 Unleashed. 2008

	This function writes each temperature
	to the 7-seg display.
*/

//#include "mc9s08qe128.h"

/* Function prototype(s). */
// Inputs: address of 7-seg display, pointer to temp. data array
unsigned char ht16k33_fsm(unsigned char slav_addr, unsigned char *data);	
/**************************/

/***** Function begins ****/
/* This is for the holtek ht16k33 7-seg display. */
/* The data array contains data from the shtc3 sensor.
 	The data is in this order [rh_msb, rh_lsb, rh_crc, t_msb, t_lsb, t_crc]. */
unsigned char ht16k33_fsm(unsigned char slav_addr, unsigned char *data)
{
	static unsigned char done_sm = 0;				// Indicates if state machine is done.
	static unsigned char addr_indx = 0;				// Address of the com channel.
	static unsigned char digit_data = 0;			// Data for each digit of the 7-seg display.
	static unsigned char i2c_state = 0;				// State machine index.
	static unsigned char prev_st = 0;				// Previous state of state machine.
	static unsigned char b = 0;
	static unsigned char done = 0;
	static float a = 0.0, c = 0.0, d = 0.0,  diff = 0.0;
	static unsigned char cntr = 0;
	static unsigned char disp_dig_indx[3];			/* Digits indexes for the display. */
	static unsigned char dspl_dig[4];				// Display digits array for 7-seg. 
	static unsigned int temp_raw = 0;
	static float temp_f = 0.0;							// temperature in farenheit.
	static float temp_c = 0.0;							// temperature in celsius. 
	static unsigned char i2c_buffer[10]= 	{
														0xEE,	// Send addr. and write. 	0	
														0x35,	// Wakeup command msb		1
														0x17,	// Wakeup command lsb		2
														0x5C,	// Meas. command msb			3 	0
														0x24,	// Meas. command lsb			4	1
														0xE1,	// Send addr. read			5	2
														0xB0, // Sleep command msb			6	3
														0x98,	// Sleep command lsb			7	4
														0x00,	// read r.h. data.			8	5
														0x00 	// read r.h. crc				9	6
														};
	static unsigned char disp_dig_lut[13] = {		// display look up table
			  											0x3f,	// 0
														0x06,	// 1
														0x5b,	// 2
														0x4f,	// 3
														0x66, // 4
														0x6d, //	5
														0x7d,	// 6
														0x07, // 7
														0x7f, // 8
														0x6f, // 9
														0x39, // c
														0x79, // e
														0x71 	// f
														};	
	static unsigned char disp_dig_lut_dp[13] = {	// with decimal point
			  											0xbf,	// 0
														0x86,	// 1
														0xdb,	// 2
														0xcf,	// 3
														0xe6, // 4
														0xed, //	5
														0xfd,	// 6
														0x87, // 7
														0xff, // 8
														0xef, // 9
														0xb9, // c
														0xf9, // e
														0xf1 	// f
														};	
	/* The COM address of each digit.  COM2 is for the colen.  Not used here. */
	static unsigned char digit_addr[4]= 	{
														0x00,	// Digit 0 (com 0) 		
														0x02,	// Digit 1 (com 1)
														0x06,	// Digit 2 (com 3)
														0x08	// Digit 3 (com 4), F degree label
														};	
	temp_raw = *(data + 3);
	temp_raw <<= 8;
	temp_raw |= *(data + 4);
	
	/* Calculate temp. for Celsius. */
	//	temp_c = -45 + 175*(temp_raw / 65536);
	/* Calculate temp. for Farenheit. */
	temp_f = (-81.0 + 315.0*(temp_raw / 65536)) + 32.0;

	/* Extract each temperature value as an integer. 
	 * Use this integer as an index to the displ_dig array. */
	
	a = temp_f/10.0;
	while (!done)
	{
		diff = a - b;
		if (diff < 1.0)
		{
			if (cntr == 0)
			{
				//*(disp_dig_indx + 0) = b;
				*(disp_dig_indx + cntr) = b;
				*(dspl_dig + cntr) = *(disp_dig_lut + *(disp_dig_indx + cntr));  
				b = 0;
			}
			else if (cntr == 1)
			{
				//*(dig_digit_indx + 1) = b;
				*(disp_dig_indx + cntr) = b;
				*(dspl_dig + cntr) = *(disp_dig_lut + *(disp_dig_indx + cntr));  
				//*(dspl_dig + cntr) = *(disp_dig_lut_dp + disp_dig_indx);  
				b = 0;
			}
			else 
			{
				//*(dig_digit_indx + cntr) = b;
				*(disp_dig_indx + cntr) = b;
				*(dspl_dig + cntr) = *(disp_dig_lut + *(disp_dig_indx + cntr));  
				//*(dspl_dig + cntr) = *(disp_dig_lut + disp_dig_indx);  
				b = 0;
			}
			cntr += 1;
			if (cntr > 2)
			{
				done = 1;
				*(dspl_dig + 3) = *(disp_dig_lut + 12);	// Unit F digit.  
			}
			else
			{
				c = a - b;
				d = c*10.0;
				a = d;
			}
		}
		else
			b += 1;
	}
	
	/*		
		i2c_states:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	I2C_SND_DIG_ADDR
		4	I2C_SND_STOP_BIT_AND_QUIT				// Send stop bit and quits.
		5	I2C_ACK_QRY
		6	I2C_SND_DIG_DAT
		7	I2C_SND_STOP_BIT
		8	CHK_DIG_FIN									// Check if all digits are written.
	*/

	/* Now send the display digit. */
	while(!done_sm)
	{
		switch(i2c_state)
		{
			/***************************/
			// I2C in idle state.
			case 0:											// i2c_idle
				prev_st = 0;								// Set previous state.
				done_sm = 1;								// exit state machine
				break;
			/***************************/
			// Send a start condition.
			case 1:											// i2c_start
				IIC1C = 0xb0;								// Send the start bit.
				i2c_state = 2;								// send dev. addr with wr bit.
				break;
			/***************************/
			// Send a device address and write bit.
			case 2:											// 
				while(!IIC1S_TCF);						// Wait until transmission is done.  Wait for any transfer to complete.
				IIC1D = slav_addr;						// Send the addr. field with WR bit set (R/W = WR).
				prev_st = 2;
				i2c_state = 5; 							// I2C_ACK_QRY;			// next state
				delay(5);									// delay 5 ms.
				break;
			/***************************/
			// Send digit address.
			case 3:											// 
				while(!IIC1S_TCF);						// Wait until transmission is done.
				IIC1D = *(digit_addr + addr_indx);	// Send the digital address (com address).
				prev_st = 3;
				i2c_state = 5; 							// go to ack query
				delay(5);									// delay 5 ms.			
				break;
			/***************************/
			// Send a stop bit and quits.
			case 4:											// 
				IIC1C_MST = 0;								// Send a stop (go to slave mode)
				done_sm = 1;								// 
				break;
			/**************************/
			// Query for ACK response from slave.
			case 5: 											// I2C_ACK_QRY;
				if (IIC1S_RXAK)	{						/*	If NAK from slave. */
					i2c_state = 4;}						// I2C_snd_stop_bit and quits;
				else 											// If ACK.
				{
					if (prev_st == 2){					// If previous command is send device address.
						i2c_state = 3;	}					// Go to send digit address.
					else if (prev_st == 3){				// If previous command is send digit address.
						i2c_state = 6;	}					// Go to send digit data.
				}
				break;
			/***************************/
			// Send digital data.
			case 6:											// 
				while(!IIC1S_TCF);						// Wait until transmission is done.
				IIC1D = *(dspl_dig + addr_indx);		// Send the digit data
				prev_st = 6;								// 
				i2c_state = 7; 							// stop bit;			// next state
				delay(5);									// delay 5 ms.			
				break;
			/***************************/
			// Send a stop and go to slave mode.
			case 7:											// 
				IIC1C_MST = 0;								// Send a stop (go to slave mode)
				i2c_state = 8;								// 
				break;
			/**************************/
			// Decide if done with state machine.
			case 8:											// 
				if (addr_indx == 3){
					done_sm = 1;	}						// done with digit
				else {
					addr_indx += 1;
					i2c_state = 1;							// go to start bit state
				}
				break;
			/**************************/
		}														// switch
	}															// while

	return (done_sm);
}
