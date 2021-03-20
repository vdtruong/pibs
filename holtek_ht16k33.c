/* Nov. 17, 2020
	States for holtek ht16k33.

	Adapted from Fabio Pereira.
	HCS08 Unleashed. 2008

	These functions write temps. to the displays.
*/

/* Function prototypes. *****************************************************************************************/
/* This function sends a single command write. */
unsigned char ht16k33_single_cmd_wr(unsigned char des_addr, unsigned char cmd, unsigned char iic_chnl);
/* This function sends a single display digit. */
void ht16k33_single_dat_wr(unsigned char des_addr, unsigned char cmd, unsigned char data, unsigned char iic_chnl);	
/* This function initializes all 7-seg displays. */
void initHt16k33(unsigned char iic_chnl);
/* This function sends all four digits to the display. */
// Inputs: address of 7-seg display, pointer to temp. data array, iic channel and sens_type.
unsigned char ht16k33_fsm(unsigned char des_addr, unsigned char *data, unsigned char iic_chnl, unsigned char sens_type);
/* This function tests out a display. */
void ht16k33_test (unsigned char des_addr, unsigned char iic_chnl);
/*****************************************************************************************************************/

/***** Function begins *******************************************************************************************/
/* This function sends a single command write. */
unsigned char ht16k33_single_cmd_wr(unsigned char des_addr, unsigned char cmd, unsigned char iic_chnl)
{
	// This function sends a single write command.
	// It uses iic2 for dev. board. 
	unsigned short int i2c_state = 1;							// Go to state 1.
	unsigned short int prev_st = 0;								// Previous state.
	unsigned char done_tx = 0;										// Finish transmitting command.
	
	/* Use iic2 for dev. board. */
	/* Need to use iic1 for main board. */
	while(!done_tx)
	{
		switch(i2c_state)
		{
			/***************************/
			// I2C in idle state.
			case 0:														// i2c_idl
				prev_st = 0;											// Set previous state.
				break;
			/***************************/
			// Send a start condition.
			case 1:														// i2c_start
				if(iic_chnl)
				{
					IIC2C1_TX = 1;										// Set for transmit.
					IIC2C1_MST = 1;									// Set for master transmit.
				}
				else
				{
					IIC1C1_TX = 1;										// Set for transmit.
					IIC1C1_MST = 1;									// Set for master transmit.
				}
				i2c_state = 2; 										// send dev. addr with wr bit.
				break;
			/***************************/
			// Send a device address and write bit.
			case 2:														// i2c_start
				if(iic_chnl)
				{
					while(!IIC2S_TCF);								// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D  = des_addr; 								// send addr. data; lsb is dir. of slave
				}
				else
				{	while(!IIC1S_TCF);								// Wait until transmission is done.  Wait for any transfer to complete.
					IIC1D  = des_addr; 								// send addr. data; lsb is dir. of slave
				}
				i2c_state = 5; 										// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Wait before sending stop bit.
			case 3:														// 
				delay(20);												// Delay (200 ~ 1 ms, 20 ~ 122 us)
				prev_st = 3;
				i2c_state = 8;											// Send stop bit.
				break;
			/***************************/
			// Send a start condition with receive set.
			case 4:														// i2c_start
				if(iic_chnl)
				{
					IIC2C1_TX = 1;										// Set for transmit.
					IIC2C1_MST = 1;									// Set for master transmit.
				}
				else
				{
					IIC1C1_TX = 1;										// Set for transmit.
					IIC1C1_MST = 1;									// Set for master transmit.
				}
				prev_st = 4;
				i2c_state = 12;										// send dev. addr with rd bit.
				break;
			/***************************/
			// Query for ACK response from slave.
			case 5: 														// I2C_ACK_QRY;
				if (prev_st == 0)										// If previous state is 0.
					i2c_state = 6;										// Go to command code.
				else if (prev_st == 6)
					i2c_state = 3;										// Go to wait.
				break;
			/***************************/
			// Send command code.
			case 6:												
				if(iic_chnl)
				{
					while(!IIC2S_TCF);								// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D  = cmd; 										// send addr. data; lsb is dir. of slave
				}
				else
				{	while(!IIC1S_TCF);								// Wait until transmission is done.  Wait for any transfer to complete.
					IIC1D  = cmd; 										// send addr. data; lsb is dir. of slave
				}
				prev_st = 6;
				i2c_state = 5; 										// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send a stop and go to slave mode.
			case 8:												 
				if(iic_chnl)
				{
					IIC2C1_MST = 0;									// Set for master transmit.
				}
				else
				{
					IIC1C1_MST = 0;									// Set for master transmit.
				}
				done_tx = 1;											// Finished.
				break;
			/**************************/
		}	// switch
	}		// done_tx
	return(done_tx);
}
/* This function sends a command and a data. */
void ht16k33_single_dat_wr(unsigned char des_addr, unsigned char cmd, unsigned char data, unsigned char iic_chnl)
{
	unsigned char done_sm = 0;							// Indicates if state machine is done.
	unsigned char i2c_state = 1;						// Next state machine index.
	unsigned char prev_st = 0;							// Previous state of state machine.
	
	/*	i2c_states:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	I2C_SND_DIG_ADDR
		4	I2C_SND_STOP_BIT_AND_QUIT					// Send stop bit and quits.
		5	I2C_ACK_QRY
		6	I2C_SND_DIG_DAT
		7	I2C_SND_STOP_BIT
		8	CHK_DIG_FIN										// Check if all digits are written.
	*/

	/* iic2 is for the dev board. */
	/* iic1 is for the main board. */
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
				if(iic_chnl)
				{
					IIC2C1_TX = 1;							// Set for transmit.
					IIC2C1_MST = 1;						// Set for master transmit.
				}
				else
				{
					IIC1C1_TX = 1;							// Set for transmit.
					IIC1C1_MST = 1;						// Set for master transmit.
				}
				i2c_state = 2;								// send dev. addr with wr bit.
				break;
			/***************************/
			// Send a device address and write bit.
			case 2:											// 
				if(iic_chnl)
				{
					while(!IIC2S_TCF);					// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D  = des_addr;					// send addr. data; lsb is dir. of slave
				}
				else
				{	while(!IIC1S_TCF);					// Wait until transmission is done.  Wait for any transfer to complete.
					IIC1D  = des_addr;					// send addr. data; lsb is dir. of slave
				}
				i2c_state = 5; 							// I2C_ACK_QRY;			// next state 
				break;
			/***************************/
			// Send command.
			case 3:											// 
				if(iic_chnl)
				{
					while(!IIC2S_TCF);					// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D  = cmd; 							// send addr. data; lsb is dir. of slave
				}
				else
				{	while(!IIC1S_TCF);					// Wait until transmission is done.  Wait for any transfer to complete.
					IIC1D  = cmd; 							// send addr. data; lsb is dir. of slave
				}
				prev_st = 3;
				i2c_state = 5; 							// go to ack query			
				break;
			/***************************/
			// Query for ACK response from slave.
			case 5: 											// I2C_ACK_QRY;
				if (prev_st == 0)							// If previous state is 0.
					i2c_state = 3;							// Go to command code.
				else if (prev_st == 3)
					i2c_state = 6;							// Send data.
				else if (prev_st == 6)
					i2c_state = 7;							// Go to wait.
				break;
			/***************************/
			// Send data.
			case 6:											// 
				if(iic_chnl)
				{
					while(!IIC2S_TCF);					// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D  = data; 						// send addr. data; lsb is dir. of slave
				}
				else
				{	while(!IIC1S_TCF);					// Wait until transmission is done.  Wait for any transfer to complete.
					IIC1D  = data;							// send addr. data; lsb is dir. of slave
				}
				prev_st = 6;								// 
				i2c_state = 5; 							// 
				break;
			/***************************/
			// Wait before sending stop bit.
			case 7:											// 
				delay(40);									// 
				i2c_state = 8;								// 
				break;
			/**************************/
			// Send a stop and go to slave mode.
			case 8:												 
				if(iic_chnl)
				{
					IIC2C1_MST = 0;						// Set for master transmit.
				}
				else
				{
					IIC1C1_MST = 0;						// Set for master transmit.
				}
				done_sm = 1;								// Finished.
				break;
			/**************************/
		}														// switch
	}															// while
}
/* This function initialize all holtek displays. */
/* We have eight 7-seg displays. */
void initHt16k33(unsigned char iic_chnl)
{
	/* Initialize all displays. 
	 * Send four init. commands to each 
	 * display.  The displays use iic1 for main board.
	 	It uses iic2 for dev. board. */
	unsigned char des_addr[8] = {0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee};	/* destination address with write bit */
	unsigned char cmd_codes[4] = {0x21, 0xa0, 0xe7, 0x80}; 								/* osc, row_output, dim, blink */
	unsigned char des_addr_cntr = 0;																/* There are eight displays. */
	unsigned char cmd_code_cntr = 0;																/* There are four command codes. */
	unsigned char done_des_addr = 0;																/* When done sending initialization for each display. */
	unsigned char done_cmd_code = 0;																/* When done sending init. command code. */
	unsigned short int i2c_state = 1;															// Go to state 1.
	unsigned short int prev_st = 0;																// Previous state.
	unsigned char done_tx = 0;																		// Finish transmitting command.
	unsigned char done_digits = 0;																// Done printing all four digits.
	unsigned char digit_addr_cntr = 0;															// Digit counter for each display.
	unsigned char digit_addr[4]=	{
												0x00,		// Digit 0 (com 0) 		
												0x02,		// Digit 1 (com 1)
												0x06,		// Digit 2 (com 3)
												0x08		// Digit 3 (com 4), F degree label
											};	
	unsigned char disp_dig_lut[13] =	{			// display look up table
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
	
	/* Use iic2 for dev. board. */
	/* Need to use iic1 for main board. */
	while(!done_des_addr)
	{
		while(!done_cmd_code)
		{
			ht16k33_single_cmd_wr(*(des_addr + des_addr_cntr), *(cmd_codes + cmd_code_cntr), iic_chnl);	// Set init. command.
			delay(20);
			cmd_code_cntr += 1;																									// Move to next command.
			if (cmd_code_cntr > 2)
				done_cmd_code = 1;																								// Done sending commands.
		}	// done_cmd_code
		des_addr_cntr += 1;																										// Move to next display.
		if (des_addr_cntr > 7)
			done_des_addr = 1;																									// Done with all displays.
		else
		{
			delay(40);																												// Wait before sending another command.
			done_cmd_code = 0;																									// Re-enter loop.
			cmd_code_cntr = 0;			
		}
	}		// done_des_addr
	delay(40);	

	done_des_addr = 0;	
	des_addr_cntr = 0;	
	
	/* Write index to all digits for each display. */
	while(!done_des_addr)
	{
		while(!done_digits)
		{	// Set des_addr, digit n, value m.
			ht16k33_single_dat_wr(*(des_addr + des_addr_cntr), *(digit_addr + digit_addr_cntr), *(disp_dig_lut + des_addr_cntr), iic_chnl);
			delay(20);
			// Decide if done.
			if (digit_addr_cntr == 3)	
				done_digits = 1;																									// done with digit
			else 
				digit_addr_cntr += 1;																							// Move to the next digit.
		}	
		ht16k33_single_cmd_wr(*(des_addr + des_addr_cntr), 0x81, iic_chnl);										// Turn on the display.
		delay(20);
			
		des_addr_cntr += 1;																										// Move to next display.
		if (des_addr_cntr > 7)
			done_des_addr = 1;																									// Done with all displays.
		else
		{
			delay(20);
			done_digits = 0;
			digit_addr_cntr = 0;
		}
	}
}
/* This function tests out the holtek display. */
void ht16k33_test(unsigned char des_addr, unsigned char iic_chnl)
{
	ht16k33_single_cmd_wr(des_addr, 0x21, iic_chnl);			// osc
	delay(20);
	ht16k33_single_cmd_wr(des_addr, 0xa0, iic_chnl);			// row_output
	delay(20);
	ht16k33_single_cmd_wr(des_addr, 0xe7, iic_chnl);			// dim
	delay(20);
	//ht16k33_single_cmd_wr(des_addr, 0x80, iic_chnl);			// blink
	delay(20);
	ht16k33_single_dat_wr(des_addr, 0x00, 0x6f, iic_chnl);	// Set des_addr, value.
	delay(20);
	ht16k33_single_dat_wr(des_addr, 0x02, 0x86, iic_chnl);	// Set des_addr, value .
	delay(20);
	ht16k33_single_dat_wr(des_addr, 0x06, 0x06, iic_chnl);	// Set des_addr, value .
	delay(20);
	ht16k33_single_dat_wr(des_addr, 0x08, 0x71, iic_chnl);	// Set des_addr, value .
	delay(20);
	ht16k33_single_cmd_wr(des_addr, 0x81, iic_chnl);			// Turn on the display.
}	
/* This is for the holtek ht16k33 7-seg display. */
/* It will display the four digits of each display. */
unsigned char ht16k33_fsm(unsigned char des_addr, unsigned char *data, unsigned char iic_chnl, unsigned char sens_type)
{
	unsigned char done_dspl = 0;						// Indicates done with display.
	unsigned char addr_indx = 0;						// Address of the com channel.
	unsigned char digit_data = 0;						// Data for each digit of the 7-seg display.
	unsigned char i2c_state = 1;						// Next state machine index.
	unsigned char prev_st = 0;							// Previous state of state machine.
	unsigned char b = 0;									// Start with digit 0.
	unsigned char done = 0;
	float a = 0.0, c = 0.0, d = 0.0,  diff = 0.0;
	unsigned char cntr = 0;
	unsigned char disp_dig_indx[3];					// Digits indexes for the display.
	unsigned char dspl_dig[4];							// Display digits array for 7-seg. 
	unsigned int temp_raw = 0;
	float temp_f = 0.0;									// temperature in farenheit.
	float temp_c = 0.0;									// temperature in celsius. 
	unsigned char high_temp = 0;						// Flag for temp_f equal or greater than 100 F.
	unsigned char disp_dig_lut[13] = 		{		// display look up table
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
	unsigned char disp_dig_lut_dp[13] = 	{		// with decimal point
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
	unsigned char digit_addr[4]= 				{
														0x00,	// Digit 0 (com 0) 		
														0x02,	// Digit 1 (com 1)
														0x06,	// Digit 2 (com 3)
														0x08	// Digit 3 (com 4), F degree label
														};	
	
	// Find temperature base on which sensor type.
	switch (sens_type)									// Which sensor type.
	{
		case 0x00:											// sens_type = 0 (shtc3)
			temp_raw = *(data + 4);						// temp msb
			temp_raw <<= 8;								// shift to front of number
			temp_raw |= *(data + 5);					// temp lsb
			// Calculate temp. for Farenheit. 
			temp_f = (-81.0 + 315.0*((float)temp_raw / 65536.0)) + 32.0;
			break;
		case 0x01:											// sens_type = 1 (sht3x)
			temp_raw = *(data + 1);						// temp msb
			temp_raw <<= 8;								// shift to front of number
			temp_raw |= *(data + 2);					// temp lsb
			// Calculate temp. for Farenheit. 
			temp_f = (-49.0 + 315.0*((float)temp_raw / (65536.0 - 1)));
			break;
		case 0x02:																// sens_type = 2
			temp_raw = *(data + 4);						// temp msb
			temp_raw <<= 8;								// shift to front of number
			temp_raw |= *(data + 5);					// temp lsb
			// Calculate temp. for Farenheit. 
			temp_f = (-81.0 + 315.0*((float)temp_raw / 65536.0)) + 32.0;
			break;
		default:
			temp_raw = *(data + 4);						// temp msb
			temp_raw <<= 8;								// shift to front of number
			temp_raw |= *(data + 5);					// temp lsb
			// Calculate temp. for Farenheit. 
			temp_f = (-81.0 + 315.0*((float)temp_raw / 65536.0)) + 32.0;
			break;
	}
	
	/* Calculate temp. for Celsius. */
	//	temp_c = -45 + 175*(temp_raw / 65536);
	
	/* Extract each temperature value as an integer. 
	  	Use this integer as an index to the displ_dig array. 
	 	b is the integer. */
	if (temp_f < 100)
		a = temp_f/10.0;
	else
	{
		a = temp_f/100.0;
		high_temp = 1;
	}
	
	while (!done)
	{
		diff = a - b;
		if (diff < 1.0)
		{
			if (!high_temp)																										// If under 100 F.
			{
				if (cntr == 1)
					*(dspl_dig + cntr) = *(disp_dig_lut_dp + b);  
				else
					*(dspl_dig + cntr) = *(disp_dig_lut + b);  	
			}
			else																														// 100 F or higher.
			{
				if (cntr == 2)
					*(dspl_dig + cntr) = *(disp_dig_lut_dp + b);  
				else
					*(dspl_dig + cntr) = *(disp_dig_lut + b);  	
			}
			cntr += 1;
			if (!high_temp)
			{
				if (cntr > 2)
				{
					done = 1;
					*(dspl_dig + cntr) = *(disp_dig_lut + 12);															// Unit F digit.  
					cntr = 0;
					b = 0;
				}
				else
				{
					c = a - b;
					d = c*10.0;
					a = d;
					b = 0;
				}
			}
			else																														// 100 F or higher.
			{
				if (cntr > 3)
				{
					b = 0;
					done = 1;
				}
				else
				{
					c = a - b;
					d = c*10.0;
					a = d;
					b = 0;
				}
			}	
		}
		else																															// if diff > 1
			b += 1;
	}
	
	/* Now send the display digit. */
	while(!done_dspl)
	{
		ht16k33_single_dat_wr(des_addr, *(digit_addr + addr_indx), *(dspl_dig + addr_indx), iic_chnl);	// Set des_2, digit n, value m.
		delay(40);
		// Decide if done.
		if (addr_indx == 3)	
			done_dspl = 1;																											// done with digit
		else 
			addr_indx += 1;																										// Move to the next digit.
	}
	/*******************************/
	
	return (ht16k33_single_cmd_wr(des_addr, 0x81, iic_chnl));														// Turn on the display.
}
