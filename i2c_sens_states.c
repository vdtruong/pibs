/* Nov. 29, 2019
	States for sensor i2c states.

	Adapted from Fabio Pereira.
	HCS08 Unleashed. 2008
*/

#include "i2c_sens_states.h"
#include "define.h"
#include "headers.h"
//#include "mc9s08qe128.h"
//#include "structures.h"
//#include "misc.c"

/* Function prototype(s). */
//enum ei2c_states i2c_fsm(char new_state);
unsigned char *i2c_fsm(unsigned char strt);					// Function returns the pointer to the buffer array.
/**************************/
struct Shtc3Outputs i2c_fsm_shtc3(unsigned char strt);	// Function returns the structure. */
/**************************/

/***** Function begins ****/
//This is for the Sensirion SHT20 sensor.

unsigned char *i2c_fsm(unsigned char strt)
{
	static unsigned char i2c_state;
	static unsigned char rd_temp = 1;				/* read temp = 1, else read r.h. */
	static unsigned char snd_cmd = 1;				// 1 is for sending a command after ACK.
	static unsigned char rpt_strt = 0;				// Send a repeated start command.
	static unsigned char snd_ack_cntr = 0;			// Send ack counter.
	static unsigned char rd_byte_cntr =0;			// Byte read counter.
	static unsigned char cmd_byte = 0;				// Read command byte.
	static unsigned char i2c_buffer[10]= {
														0x80,	// Send addr. write. 0	rd_byte_cntr
														0xE3,	// Read temp. cmd.	1
														0xE5,	// Read r.h. cmd.		2
														0x00,	// Read dummy data.	3 		0
														0x00,	// read temp. data.	4		1
														0x00,	// read temp. data.	5		2
														0x00, // read temp. crc		6		3
														0x00,	// read r.h. data.	7		4
														0x00,	// read r.h. data.	8		5
														0x00 	// read r.h. crc		9		6
														};		

	/*	i2c_state:
		
		0	I2C_IDLE
		1	I2C_START
		2	I2C_REPEATED_START
		3	I2C_ACK_QRY
		4	I2C_SND_RD_CMD
		5	I2C_DUMMY_READ
		6	I2C_RD_BYTE
		7	I2C_SND_ACK
		8	I2C_SND_NAK
		9	I2C_STOP
	*/

	if (strt)
		i2c_state = 1;	// go to start state
	else
		i2c_state = 0;	// stay at idle state.

	switch(i2c_state)
	{
		/***************************/
		// I2C in idle state.
		case 0:	// i2c_idle
			break;
		/***************************/
		// Send a start :condition.
		case 1:											// i2c_start
			IIC1C1 = 0xb0;								// Send the start bit. {1011 0000} {iic enb, int enb, mst, tx/rx}
			IIC1D = *(i2c_buffer + 0);				// Send the addr. field with WR bit set (R/W = WR).
			delay(20);									// Delay 20 ms.
			while(!IIC1S_TCF);						// Wait until transmission is done.
			snd_cmd = 1;								// Indicates after ACK, send a command.
			i2c_state = 3; 							// I2C_ACK_QRY;			// next state
			break;
		/***************************/
		// Send a repeated start condition.
		case 2: 										// I2C_REPEATED_START:
			IIC1C1 = 0xb4;							// Send repeated start.
			IIC1D = *(i2c_buffer + 0)|RD;		// Send the addr. field plus the read bit (R/W = RD).
			delay(20);								// Delay 20 ms.
			while(!IIC1S_TCF);					// Wait until transmission is done.
			snd_cmd = 0;							// Indicates the next send is not a command.
			rpt_strt = 0;							// Reset.
			i2c_state = 3;							// I2C_ACK_QRY;		// next state
			break;
		/***************************/
		// Query for ACK response from slave.
		case 3: 										// I2C_ACK_QRY;
			if (IIC1S_RXAK)						/*	If NAK from slave. */
			{
				i2c_state = 0;						//I2C_IDLE;
			}
			else 				
			{
				if (snd_cmd)						// Send a read command.
				{
					if (rd_temp)					// If read temperature data.
					{	
						rd_temp = 0;				// Next time read r.h..
						cmd_byte = *(i2c_buffer + 1);
					}
					else								// Else, read relative humility data.
					{
						cmd_byte = *(i2c_buffer + 2);	
					}
					i2c_state = 4;					// I2C_SND_RD_CMD;
				}
				else if (rpt_strt)
				{
					i2c_state = 2;					// I2C_REPEATED_START;
				}
				else
				{
					i2c_state = 5;					// I2C_DUMMY_READ;// Change direction to read.
				}
			}
			break;
		/***************************/
		// Send the read command packet.
		case 4:													// I2C_SND_RD_CMD;
			IIC1D = cmd_byte;									// Send the read command.
			delay(20); 											// Delay for 20 ms.
			while(!IIC1S_TCF);								// Wait until transmission is done.
			snd_cmd = 0;										// Do not send a read command next.
			rpt_strt = 1;										// Send a repeated start next.
			i2c_state = 3;										// I2C_ACK_QRY;// Next state, query for ACK.
			break;
		/***************************/
		// Do a dummy read to change direction and cause a delay.
		case 5:													// I2C_DUMMY_READ:
			IIC1C_TX = 0;										// Change to read mode.
			*(i2c_buffer + 3) = IIC1D;						// Do a dummy read.
			delay(20);											// Wait 20 ms.
			while(!IIC1S_TCF);								// Wait until transmission is done.
			i2c_state = 6;										// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
			break;
		/***************************/
		// Read one byte of data from sensor.
		case 6:													// I2C_RD_BYTE:
			rd_byte_cntr =+ 1;								// Increment counter.
			*(i2c_buffer + rd_byte_cntr + 3) = IIC1D;	// Read one byte of data from sensor.
			delay(20);											// Wait 20 ms.
			while(!IIC1S_TCF);								// Wait until transmission is done.
			if(rd_byte_cntr == 3 | rd_byte_cntr == 6) // Send NAK for these two counts.
			{				
				i2c_state = 8;									// I2C_SND_NAK;					// Send NACK.
			}
			else 
			{
				i2c_state = 7;									// I2C_SND_ACK;					// Send ACK.
			}
			break;
		/***************************/
		// Send ACK after byte read.
		case 7:													// I2C_SND_ACK:
			IIC1C_TXAK = 0;									// Send ACK.
			delay(10);											// Wait 10 ms.
			i2c_state = 6;										// I2C_RD_BYTE;
			break;
		/***************************/
		// Send NAK after CRC byte read.
		case 8:													// I2C_SND_NAK:
			IIC1C_TXAK = 1;									// Send NAK.
			delay(10);											// Wait 10 ms.
			i2c_state = 9;										// I2C_STOP;
			break;
		/***************************/
		// Send a stop and go to slave mode.
		case 9:													// I2C_STOP:
			IIC1C_MST = 0;										// Send a stop (go to slave mode)
			rd_byte_cntr = 0;									// Reset.
			snd_ack_cntr = 0;									// Reset.
			rd_byte_cntr =0;									// Reset.	
			snd_cmd = 1;										// Reset.
			i2c_state = 0;										// I2C_IDLE;	// Next state.
			break;
		/**************************/
		// Wait before making measurement.
		//case I2C_RD_WAIT:
		//	delay(20);					// Wait 20 ms before taking measurement.
		//	i2c_state = I2C_RD_BYTE;// Go to read one byte of data.
		//	break;
		/***************************/
	}
	return (i2c_buffer);
}

/***** Function begins ****/
/* This is for the shtc3 sensor. */
struct Shtc3Outputs i2c_fsm_shtc3(unsigned char strt)
{
	static unsigned char i2c_state;
	static unsigned char rd_temp = 1;				/* read temp = 1, else read r.h. */
	static unsigned char snd_cmd = 1;				// 1 is for sending a command after ACK.
	static unsigned char rpt_strt = 0;				// Send a repeated start command.
	static unsigned char snd_ack_cntr = 0;			// Send ack counter.
	static unsigned char rd_byte_cntr =0;			// Byte read counter.
	static unsigned char cmd_byte = 0;				// Read command byte.
	static unsigned int	prev_st = 0;				// Previous state.
	static unsigned char i2c_buffer[10]= 	{
														0xE0,	// Send addr. write. 	0	
														0x35,	// Wakeup command msb	1
														0x17,	// Wakeup command lsb	2
														0x5C,	// Meas. command msb		3 		0
														0x24,	// Meas. command lsb		4		1
														0xE1,	// Send addr. read		5		2
														0xB0, // Sleep command msb		6		3
														0x98,	// Sleep command lsb		7		4
														0x00,	// read r.h. data.		8		5
														0x00 	// read r.h. crc			9		6
														};		
	static struct Shtc3Outputs ste_out;				// state machine outputs
	
	ste_out.done = 0;

	/*		
		i2c_state:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	Not used.
		4	Not used.
		5	I2C_ACK_QRY
		6	I2C_SND_WKUP_CMD_MSB
		7	I2C_SND_WKUP_CMD_LSB
		8	I2C_SND_STOP_BIT
		9	I2C_WAIT_FOR_WKUP
		10	I2C_SND_MEAS_CMD_MSB
		11	I2C_SND_MEAS_CMD_LSB
		12	I2C_SND_DEV_ADDR_RD
		13	I2C_WAIT_MEAS
		14	Not used.
		15	I2C_RD_DATA
		16	I2C_SND_ACK
		17	I2C_SND_NACK
		18	I2C_SND_SLEEP_CMD_MSB
		19	I2C_SND_SLEEP_CMD_LSB
	*/

	while(!ste_out.done)
	{
		if (strt)
			i2c_state = 1;										// go to start state
		else	
			i2c_state = 0;										// stay at idle state	

		// If the TCF flag is not set, do not process the state machine
		// and return now with the current state.
		//if (!IICS_TCF) return (i2c_state);
		//IICS_TCF = 1;	// Clear TCF flag if it is set.
		switch(i2c_state)
		{
			/***************************/
			// I2C in idle state.
			case 0:												// i2c_idl
				prev_st = 0;									// Set previous state.
				break;
			/***************************/
			// Send a start condition.
			case 1:												// i2c_start
				IIC1C = 0xb0;									// Send the start bit.
				if (prev_st == 0 || prev_st == 9 || prev_st == 16)
					i2c_state = 2; 							// send dev. addr with wr bit.
				else if (prev_st == 11)
					i2c_state = 12;							// send dev. addr with rd bit.
				break;
			/***************************/
			// Send a device address and write bit before wake command.
			case 2:												// i2c_start
				while(!IIC1S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
				IIC1D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send a device address and write bit before meas. command.
			case 3:												// i2c_start
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
				//delay(20);									// Delay 20 ms.
				prev_st = 3;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Query for ACK response from slave.
			case 5: 												// I2C_ACK_QRY;
				if (IIC1S_RXAK)								/*	If NAK from slave. */
					i2c_state = 0;								// I2C_IDLE
				else 												// If ACK.
				{
					if (prev_st == 0)							// If previous command is write before wakeup msb command.
						i2c_state = 6;							// Go to wakeup cmd msb.
					else if (prev_st == 6)
						i2c_state = 7;							// Go to wakeup cmd lsb
					else if (prev_st == 7 || prev_st == 11 || prev_st == 19)
						i2c_state = 8;							// Send stop bit state.
					else if (prev_st == 9)
						i2c_state = 10;						// send meas. command msb
					else if (prev_st == 10)
						i2c_state = 11;						// send meas. command lsb
					else if (prev_st == 12)
						i2c_state = 13;						// wait for read to start 
					else if (prev_st == 16)
						i2c_state = 18;						// send sleep command msb
					else if (prev_st == 18)
						i2c_state = 19;						// send sleep command lsb
				}
				break;
			/***************************/
			// Send wakeup msb command.
			case 6:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 1);					// Send the wakeup msb command.
				//delay(20);									// Delay 20 ms.
				//snd_cmd = 1;									// Indicates after ACK, send a command.
				prev_st = 6;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send wakeup lsb command.
			case 7:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 2);					// Send the wakeup msb command.
				prev_st = 7;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send a stop and go to slave mode.
			case 8:												// 
				IIC1C_MST = 0;									// Send a stop (go to slave mode)
				if (prev_st == 7)
					i2c_state = 9;								// Wait for device to wake up.
				else if (prev_st == 11 || prev_st == 16)
					i2c_state = 1;								// send start bit 
				else
					{
						i2c_state = 0;							// idle
						rd_byte_cntr = 0;						// reset
						ste_out.done = 1;						// Finish state machine
					}
				break;
			/**************************/
			// Wait for device to wake up.
			case 9:												// 
				delay(2);										// Delay 2 ms.
				prev_st = 9;
				i2c_state = 1; 								// Start bit state.
				break;
			/***************************/
			// Send the meas. msb command.
			case 10:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 3);					// Send the meas. msb command.
				prev_st = 10;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send the meas. lsb command.
			case 11:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 4);					// Send the meas. lsb command.
				prev_st = 11;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send device addr. rd command.
			case 12:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 5);					// Send the device address and W/R bit sets to read.
				prev_st = 12;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Wait for device to finish meas.
			case 13:												// 
				delay(13);										// Delay 13 ms.
				prev_st = 13;
				i2c_state = 15; 								// Start to read data.
				break;
			/***************************/
			// Send the read command packet.
			case 4:												// I2C_SND_RD_CMD;
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = cmd_byte;								// Send the read command.
				delay(20); 										// Delay for 20 ms.
				snd_cmd = 0;									// Do not send a read command next.
				rpt_strt = 1;									// Send a repeated start next.
				i2c_state = 3;									// I2C_ACK_QRY;// Next state, query for ACK.
				break;
			/***************************/
			// Do a dummy read to change direction and cause a delay.
			case 14:												// I2C_DUMMY_READ:
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1C_TX = 0;									// Change to read mode.
				*(i2c_buffer + 8) = IIC1D;					// Do a dummy read.
				delay(20);										// Wait 20 ms.
				i2c_state = 6;									// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
				break;
			/***************************/
			// Read one byte of data from sensor.
			case 15:												// I2C_RD_BYTE:
				while(!IIC1S_TCF);							// Wait until ready.
				/* When transmission is done, ready to do some thing. */
				*(ste_out.data + rd_byte_cntr) = IIC1D;// Read one byte of data from sensor.
				rd_byte_cntr =+ 1;							// Increment counter.
				delay(20);										// Wait 20 ms.
				if (rd_byte_cntr >5)	
					i2c_state = 1;								// Send Start bit.			
				else
					i2c_state = 16;							// Send ACK.
				prev_st = 15;
				break;
			/***************************/
			// Send ACK after byte read.
			case 16:												// I2C_SND_ACK:
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1C_TXAK = 0;								// Send ACK.
				delay(10);										// Wait 10 ms.
				if (rd_byte_cntr < 6)						// If has not read 6 bytes yet.
					i2c_state = 15;
				else
					i2c_state = 8;								// Send stop bit;
				prev_st = 16;
				break;
			//***************************/
			// Send NAK after CRC byte read.
			case 17:												// I2C_SND_NAK:
				IIC1C_TXAK = 1;								// Send NAK.
				delay(10);										// Wait 10 ms.
				i2c_state = 9;									// I2C_STOP;
				break;
			/***************************/
			// Send the sleep command msb.
			case 18:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 6);					// Send the sleep msb command.
				prev_st = 18;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
			// Send the sleep lsb command.
			case 19:												// 
				while(!IIC1S_TCF);							// Wait until transmission is done.
				IIC1D = *(i2c_buffer + 7);					// Send the meas. lsb command.
				prev_st = 19;
				i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				break;
			/***************************/
		} 	/* switch */
	}		/* while */
	return (ste_out);
}
