/* Nov. 29, 2019
	States for sensor i2c states.

	Adapted from Fabio Pereira.
	HCS08 Unleashed. 2008
*/

#include "i2c_sens_states.h"
#include "define.h"
#include "headers.h"

/* Function prototype(s). **************************************************************************************/
unsigned char *i2c_fsm(unsigned char strt);													// Function returns the pointer to the buffer array.
struct Shtc3Outputs i2c_fsm_shtc3(unsigned char strt);									// Function returns the structure. 
struct Shtc3Outputs i2c_fsm_sht3x(unsigned char strt);									// Function returns the structure. 
struct Shtc3Outputs i2c_fsm_d6t_1a_01(unsigned char strt);								// Function returns the structure. 
// This is for the ST HTS221 sensor.
//struct Shtc3Outputs i2c_fsm_hts221(unsigned char strt);
struct Shtc3Outputs i2c_fsm_hts221(unsigned char strt, unsigned char strt_addr);
struct Shtc3Outputs i2c_fsm_aht20(unsigned char strt);									// Function returns the structure. 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/***** Function begins *****************************************************************************************/
//This is for the Sensirion SHT20 sensor.

unsigned char *i2c_fsm(unsigned char strt)
{
	unsigned char i2c_state;
	unsigned char rd_temp = 1;				// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;				// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;			// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;		// Send ack counter.
	unsigned char rd_byte_cntr =0;		// Byte read counter.
	unsigned char cmd_byte = 0;			// Read command byte.
	unsigned char i2c_buffer[10]= {
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
// This is for the Sensirion shtc3 sensor. 
struct Shtc3Outputs i2c_fsm_shtc3(unsigned char strt)
{
	unsigned short int i2c_state = 1;		// Go to state 1.
	unsigned short int prev_st = 0;			// Previous state.
	unsigned char rd_temp = 1;					// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;					// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;				// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;			// Send ack counter.
	unsigned char rd_byte_cntr =0;			// Byte read counter.
	unsigned char cmd_byte = 0;				// Read command byte.
	unsigned char i2c_buffer[10]= 	{
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
	struct Shtc3Outputs ste_out;				// state machine outputs
	unsigned char done = 0;

	ste_out.done = 0;

	/*		
		i2c_state:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	Wait before measurement.
		4	Same as 1.
		5	I2C_ACK_QRY
		6	I2C_SND_WKUP_CMD_MSB
		7	I2C_SND_WKUP_CMD_LSB
		8	I2C_SND_STOP_BIT
		9	I2C_WAIT_FOR_WKUP
		10	I2C_SND_MEAS_CMD_MSB
		11	I2C_SND_MEAS_CMD_LSB
		12	I2C_SND_DEV_ADDR_RD
		13	Change mode to receive.
		14	Wait after sleep command lsb.
		15	I2C_RD_DATA
		16	I2C_SND_ACK
		17	I2C_SND_NACK
		18	I2C_SND_SLEEP_CMD_MSB
		19	I2C_SND_SLEEP_CMD_LSB
	*/

	if (strt) {
		while(!done)
		{
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
					//IIC2C = 0xb0;									// Send the start bit.
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					if (prev_st == 0 || prev_st == 9 || prev_st == 16)
						i2c_state = 2; 							// send dev. addr with wr bit.
					else if (prev_st == 11)
						i2c_state = 12;							// send dev. addr with rd bit.
					break;
				/***************************/
				// Send a device address and write bit before wake command.
				case 2:												// i2c_start
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Wait before meas. command.
				case 3:												// i2c_start
					delay(2400);									// Delay
					prev_st = 3;
					i2c_state = 8;									// Send stop bit.
					break;
				/***************************/
				// Send a start condition with receive set.
				case 4:												// i2c_start
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					prev_st = 4;
					i2c_state = 12;								// send dev. addr with rd bit.
					break;
				/***************************/
				// Query for ACK response from slave.
				case 5: 												// I2C_ACK_QRY;
					if (prev_st == 0)								// If previous command is write before wakeup msb command.
						i2c_state = 6;								// Go to wakeup cmd msb.
					else if (prev_st == 6)
						i2c_state = 7;								// Go to wakeup cmd lsb
					else if (prev_st == 7)
						i2c_state = 9;								// Wait for wake up.
					else if (prev_st == 9)
						i2c_state = 10;							// send meas. command msb
					else if (prev_st == 10)
						i2c_state = 11;							// send meas. command lsb
					else if (prev_st == 11) 
						i2c_state = 3;								// Wait for measurement.
					else if (prev_st == 12)
						i2c_state = 13;							// wait for read to start 
					else if (prev_st == 16)
						i2c_state = 18;							// send sleep command msb
					else if (prev_st == 18)
						i2c_state = 19;							// send sleep command lsb
					else if (prev_st == 19)
						i2c_state = 14;							// Send stop bit state.
					break;
				/***************************/
				// Send wakeup msb command.
				case 6:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 1);					// Send the wakeup msb command.
					prev_st = 6;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send wakeup lsb command.
				case 7:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 2);					// Send the wakeup lsb command.
					prev_st = 7;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send a stop and go to slave mode.
				case 8:												// 
					IIC2C1_MST = 0;								// Send a stop (go to slave mode)
					if (prev_st == 3)
						i2c_state = 4;								// Send start bit with receive mode.
					else if (prev_st == 9)
						i2c_state = 1;								// Send start bit.
					else if (prev_st == 16)
						i2c_state = 1;								// Send start bit with tx mode.
					else if (prev_st == 19)
					{
						ste_out.done = 1;							// Finish.
						done = 1;
					}
					break;
				/**************************/
				// Wait for device to wake up.
				case 9:												// 
					delay(60);										// Delay.
					prev_st = 9;
					i2c_state = 8;
					break;
				/***************************/
				// Send the meas. msb command.
				case 10:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 3);					// Send the meas. msb command.
					prev_st = 10;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the meas. lsb command.
				case 11:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 4);					// Send the meas. lsb command.
					prev_st = 11;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send device addr. rd command.
				case 12:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 5);					// Send the device address and W/R bit sets to read.
					prev_st = 12;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Change mode to receive.
				case 13:												// 
					IIC2C1_TX = 0;									// Change to read mode.
					prev_st = 13;
					i2c_state = 15; 								// Start to read data.
					break;
				/***************************/
				// Wait before sending a stop bit.
				case 14:												// I2C_DUMMY_READ:
					delay(200);										// Wait.
					i2c_state = 8;									// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
					break;
				/***************************/
				// Read one byte of data from sensor.
				case 15:												// I2C_RD_BYTE:
					while(!IIC2S_TCF);							// Wait until ready.
					/* When transmission is done, ready to do some thing. */
					*(ste_out.data + rd_byte_cntr) = IIC2D;// Read one byte of data from sensor.
					rd_byte_cntr += 1;							// Increment counter.
					i2c_state = 16;								// Send ACK.
					prev_st = 15;
					break;
				/***************************/
				// Send ACK after byte read.
				case 16:												// I2C_SND_ACK:
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2C1_TXAK = 0;								// Send ACK.
					if (rd_byte_cntr < 7)						// If has not read 7 bytes yet.
						i2c_state = 15;
					else
						i2c_state = 8;								// Send stop bit;
					prev_st = 16;
					break;
				//***************************/
				// Send NAK after CRC byte read.
				case 17:												// I2C_SND_NAK:
					IIC2C1_TXAK = 1;								// Send NAK.
					//delay(10);										// Wait 10 ms.
					i2c_state = 9;									// I2C_STOP;
					break;
				/***************************/
				// Send the sleep command msb.
				case 18:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 6);					// Send the sleep msb command.
					prev_st = 18;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the sleep lsb command.
				case 19:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 7);					// Send the meas. lsb command.
					prev_st = 19;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					//done = 1;
					break;
				/***************************/
			} 	/* switch */
		}		/* while */
	}			/* strt */
	return (ste_out);
}
/* This is for the Sensirion sht3x sensor. */
struct Shtc3Outputs i2c_fsm_sht3x(unsigned char strt)
{
	unsigned short int i2c_state = 1;		// Go to state 1.
	unsigned short int prev_st = 0;			// Previous state.
	unsigned char rd_temp = 1;					// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;					// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;				// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;			// Send ack counter.
	unsigned char rd_byte_cntr =0;			// Byte read counter.
	unsigned char cmd_byte = 0;				// Read command byte.
	unsigned char i2c_buffer[10]= 	{
												0x88,	// Send addr. write. 	0	
												0x35,	// Wakeup command msb	1
												0x17,	// Wakeup command lsb	2
												0x2C,	// Meas. command msb		3 		0
												0x06,	// Meas. command lsb		4		1
												0x89,	// Send addr. read		5		2
												0xB0, // Sleep command msb		6		3
												0x98,	// Sleep command lsb		7		4
												0x00,	// read r.h. data.		8		5
												0x00 	// read r.h. crc			9		6
												};		
	struct Shtc3Outputs ste_out;				// state machine outputs
	unsigned char done = 0;

	ste_out.done = 0;

	/*		
		i2c_state:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	Wait before measurement.
		4	Same as 1.
		5	I2C_ACK_QRY
		6	I2C_SND_WKUP_CMD_MSB
		7	I2C_SND_WKUP_CMD_LSB
		8	I2C_SND_STOP_BIT
		9	I2C_WAIT_FOR_WKUP
		10	I2C_SND_MEAS_CMD_MSB
		11	I2C_SND_MEAS_CMD_LSB
		12	I2C_SND_DEV_ADDR_RD
		13	Change mode to receive.
		14	Wait after sleep command lsb.
		15	I2C_RD_DATA
		16	I2C_SND_ACK
		17	I2C_SND_NACK
		18	I2C_SND_SLEEP_CMD_MSB
		19	I2C_SND_SLEEP_CMD_LSB
	*/

	if (strt) {
		while(!done)
		{
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
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					if (prev_st == 0 || prev_st == 9 || prev_st == 16)
						i2c_state = 2; 							// send dev. addr with wr bit.
					else if (prev_st == 11)
						i2c_state = 12;							// send dev. addr with rd bit.
					break;
				/***************************/
				// Send a device address and write bit before wake command.
				case 2:												// i2c_start
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Wait before meas. command.
				case 3:												// i2c_start
					delay(2400);									// Delay
					prev_st = 3;
					i2c_state = 8;									// Send stop bit.
					break;
				/***************************/
				// Send a start condition with receive set.
				case 4:												// i2c_start
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					prev_st = 4;
					i2c_state = 12;								// send dev. addr with rd bit.
					break;
				/***************************/
				// Query for ACK response from slave.
				// The hardware works without ACK query here.
				case 5: 												// I2C_ACK_QRY;
					if (prev_st == 0)								// If previous command is write before wakeup msb command.
						i2c_state = 10;							// Go to meas. cmd msb.
					else if (prev_st == 6)
						i2c_state = 7;								// Go to wakeup cmd lsb
					else if (prev_st == 7)
						i2c_state = 9;								// Wait for wake up.
					else if (prev_st == 9)
						i2c_state = 10;							// send meas. command msb
					else if (prev_st == 10)
						i2c_state = 11;							// send meas. command lsb
					else if (prev_st == 11) 
						i2c_state = 3;								// Wait for measurement.
					else if (prev_st == 12)
						i2c_state = 13;							// wait for read to start 
					else if (prev_st == 16)
						i2c_state = 18;							// send sleep command msb
					else if (prev_st == 18)
						i2c_state = 19;							// send sleep command lsb
					else if (prev_st == 19)
						i2c_state = 14;							// Send stop bit state.
					break;
				/***************************/
				// Send wakeup msb command.
				case 6:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 1);					// Send the wakeup msb command.
					prev_st = 6;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send wakeup lsb command.
				case 7:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 2);					// Send the wakeup lsb command.
					prev_st = 7;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send a stop and go to slave mode.
				case 8:												// 
					IIC2C1_MST = 0;								// Send a stop (go to slave mode)
					if (prev_st == 3)
						i2c_state = 4;								// Send start bit with receive mode.
					else if (prev_st == 9)
						i2c_state = 1;								// Send start bit.
					else if (prev_st == 16)
					{
						ste_out.done = 1;							// Finish.
						done = 1;
					}
					break;
				/**************************/
				// Wait for device to wake up.
				case 9:												// 
					delay(60);										// Delay.
					prev_st = 9;
					i2c_state = 8;
					break;
				/***************************/
				// Send the meas. msb command.
				case 10:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 3);					// Send the meas. msb command.
					prev_st = 10;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the meas. lsb command.
				case 11:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 4);					// Send the meas. lsb command.
					prev_st = 11;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send device addr. rd command.
				case 12:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 5);					// Send the device address and W/R bit sets to read.
					prev_st = 12;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Change mode to receive.
				case 13:												// 
					IIC2C1_TX = 0;									// Change to read mode.
					prev_st = 13;
					i2c_state = 15; 								// Start to read data.
					break;
				/***************************/
				// Wait before sending a stop bit.
				case 14:												// I2C_DUMMY_READ:
					delay(200);										// Wait.
					i2c_state = 8;									// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
					break;
				/***************************/
				// Read one byte of data from sensor.
				case 15:												// I2C_RD_BYTE:
					while(!IIC2S_TCF);							// Wait until ready.
					/* When transmission is done, ready to do some thing. */
					*(ste_out.data + rd_byte_cntr) = IIC2D;// Read one byte of data from sensor.
					rd_byte_cntr += 1;							// Increment counter.
					i2c_state = 16;								// Send ACK.
					prev_st = 15;
					break;
				/***************************/
				// Send ACK after byte read.
				case 16:												// I2C_SND_ACK:
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2C1_TXAK = 0;								// Send ACK.
					if (rd_byte_cntr < 7)						// If has not read 7 bytes yet.  Include extra byte in front.
						i2c_state = 15;
					else
						i2c_state = 8;								// Send stop bit;
					prev_st = 16;
					break;
				//***************************/
				// Send NAK after CRC byte read.
				case 17:												// I2C_SND_NAK:
					IIC2C1_TXAK = 1;								// Send NAK.
					//delay(10);										// Wait 10 ms.
					i2c_state = 9;									// I2C_STOP;
					break;
				/***************************/
				// Send the sleep command msb.
				case 18:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 6);					// Send the sleep msb command.
					prev_st = 18;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the sleep lsb command.
				case 19:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 7);					// Send the meas. lsb command.
					prev_st = 19;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					//done = 1;
					break;
				/***************************/
			} 	/* switch */
		}		/* while */
	}			/* strt */
	return (ste_out);
}
/* This is for the Omron D6T_1A_01/02 sensor. */
struct Shtc3Outputs i2c_fsm_d6t_1a_01(unsigned char strt)
{
	unsigned short int i2c_state = 1;		// Go to state 1.
	unsigned short int prev_st = 0;			// Previous state.
	unsigned char rd_temp = 1;					// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;					// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;				// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;			// Send ack counter.
	unsigned char rd_byte_cntr =0;			// Byte read counter.
	unsigned char cmd_byte = 0;				// Read command byte.
	unsigned char i2c_buffer[10]= 	{
												0x14,	// Send addr. write. 	0	
												0x35,	// Wakeup command msb	1
												0x17,	// Wakeup command lsb	2
												0x4C,	// Meas. command msb		3 		0
												0x06,	// Meas. command lsb		4		1
												0x15,	// Send addr. read		5		2
												0xB0, // Sleep command msb		6		3
												0x98,	// Sleep command lsb		7		4
												0x00,	// read r.h. data.		8		5
												0x00 	// read r.h. crc			9		6
												};		
	struct Shtc3Outputs ste_out;				// state machine outputs
	unsigned char done = 0;

	ste_out.done = 0;

	/*		
		i2c_state:
		
		0	I2C_IDLE
		1	I2C_SND_STRT_BIT
		2	I2C_SND_DEV_ADDR_WR
		3	Wait before measurement.
		4	Same as 1.
		5	I2C_ACK_QRY
		6	I2C_SND_WKUP_CMD_MSB
		7	I2C_SND_WKUP_CMD_LSB
		8	I2C_SND_STOP_BIT
		9	I2C_WAIT_FOR_WKUP
		10	I2C_SND_MEAS_CMD_MSB
		11	I2C_SND_MEAS_CMD_LSB
		12	I2C_SND_DEV_ADDR_RD
		13	Change mode to receive.
		14	Wait after sleep command lsb.
		15	I2C_RD_DATA
		16	I2C_SND_ACK
		17	I2C_SND_NACK
		18	I2C_SND_SLEEP_CMD_MSB
		19	I2C_SND_SLEEP_CMD_LSB
	*/

	if (strt) {
		while(!done)
		{
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
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					if (prev_st == 0 || prev_st == 9 || prev_st == 16)
						i2c_state = 2; 							// send dev. addr with wr bit.
					else if (prev_st == 11)
						i2c_state = 12;							// send dev. addr with rd bit.
					break;
				/***************************/
				// Send a device address and write bit before wake command.
				case 2:												// i2c_start
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Wait before meas. command.
				case 3:												// i2c_start
					delay(2400);									// Delay
					prev_st = 3;
					i2c_state = 8;									// Send stop bit.
					break;
				/***************************/
				// Send a start condition with receive set.
				case 4:												// i2c_start
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					prev_st = 4;
					i2c_state = 12;								// send dev. addr with rd bit.
					break;
				/***************************/
				// Query for ACK response from slave.
				// The hardware works without ACK query here.
				case 5: 												// I2C_ACK_QRY;
					if (prev_st == 0)								// If previous command is write before wakeup msb command.
						i2c_state = 10;							// Go to meas. cmd msb.
					else if (prev_st == 6)
						i2c_state = 7;								// 
					else if (prev_st == 7)
						i2c_state = 9;								// 
					else if (prev_st == 9)
						i2c_state = 10;							// send meas. command msb
					else if (prev_st == 10)
						i2c_state = 4;								// start
					else if (prev_st == 11) 
						i2c_state = 3;								// 
					else if (prev_st == 12)
						i2c_state = 15;							// take data 
					else if (prev_st == 16)
						i2c_state = 18;							// 
					else if (prev_st == 18)
						i2c_state = 19;							// 
					else if (prev_st == 19)
						i2c_state = 14;							// 
					break;
				/***************************/
				// Send wakeup msb command.
				case 6:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 1);					// Send the wakeup msb command.
					prev_st = 6;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send wakeup lsb command.
				case 7:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 2);					// Send the wakeup lsb command.
					prev_st = 7;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send a stop and go to slave mode.
				case 8:												// 
					IIC2C1_MST = 0;								// Send a stop (go to slave mode)
					if (prev_st == 3)
						i2c_state = 4;								// Send start bit with receive mode.
					else if (prev_st == 9)
						i2c_state = 1;								// Send start bit.
					else if (prev_st == 16)
					{
						ste_out.done = 1;							// Finish.
						done = 1;
					}
					break;
				/**************************/
				// Wait for device to wake up.
				case 9:												// 
					delay(60);										// Delay.
					prev_st = 9;
					i2c_state = 8;
					break;
				/***************************/
				// Send the meas. msb command.
				case 10:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 3);					// Send the meas. msb command.
					prev_st = 10;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the meas. lsb command.
				case 11:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 4);					// Send the meas. lsb command.
					prev_st = 11;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send device addr. rd command.
				case 12:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 5);					// Send the device address and W/R bit sets to read.
					prev_st = 12;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Change mode to receive.
				case 13:												// 
					IIC2C1_TX = 0;									// Change to read mode.
					prev_st = 13;
					i2c_state = 15; 								// Start to read data.
					break;
				/***************************/
				// Wait before sending a stop bit.
				case 14:												// I2C_DUMMY_READ:
					delay(200);										// Wait.
					i2c_state = 8;									// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
					break;
				/***************************/
				// Read one byte of data from sensor.
				case 15:												// I2C_RD_BYTE:
					while(!IIC2S_TCF);							// Wait until ready.
					/* When transmission is done, ready to do some thing. */
					*(ste_out.data + rd_byte_cntr) = IIC2D;// Read one byte of data from sensor.
					rd_byte_cntr += 1;							// Increment counter.
					i2c_state = 16;								// Send ACK.
					prev_st = 15;
					break;
				/***************************/
				// Send ACK after byte read.
				case 16:												// I2C_SND_ACK:
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2C1_TXAK = 0;								// Send ACK.
					if (rd_byte_cntr < 6)						// If has not read 7 bytes yet.  Include extra byte in front.
						i2c_state = 15;
					else
						i2c_state = 8;								// Send stop bit;
					prev_st = 16;
					break;
				//***************************/
				// Send NAK after CRC byte read.
				case 17:												// I2C_SND_NAK:
					IIC2C1_TXAK = 1;								// Send NAK.
					//delay(10);										// Wait 10 ms.
					i2c_state = 9;									// I2C_STOP;
					break;
				/***************************/
				// Send the sleep command msb.
				case 18:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 6);					// Send the sleep msb command.
					prev_st = 18;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the sleep lsb command.
				case 19:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 7);					// Send the meas. lsb command.
					prev_st = 19;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					//done = 1;
					break;
				/***************************/
			} 	/* switch */
		}		/* while */
	}			/* strt */
	return (ste_out);
}
// This is for the ST HTS221 sensor. 
struct Shtc3Outputs i2c_fsm_hts221(unsigned char strt, unsigned char strt_addr)
{
	unsigned short int i2c_state = 1;		// Go to state 1.
	unsigned short int prev_st = 0;			// Previous state.
	unsigned char rd_temp = 1;					// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;					// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;				// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;			// Send ack counter.
	unsigned char rd_byte_cntr = 0;			// Byte read counter.
	unsigned char cmd_byte = 0;				// Read command byte.
	unsigned char i2c_buffer[10] = 	{		// Not all elements are used.
												0xBE,	// Send addr. write. 				0	
												0xA8,	// Read addr with multiple flag.	1
												0x17,	// Wakeup command lsb				2
												0x5C,	// Meas. command msb					3 		0
												0x24,	// Meas. command lsb					4		1
												0xBF,	// Send addr. read					5		2
												0xB0, // Sleep command msb					6		3
												0x98,	// Sleep command lsb					7		4
												0x00,	// read r.h. data.					8		5
												0x00 	// read r.h. crc						9		6
												};		
	struct Shtc3Outputs ste_out;				// state machine outputs
	unsigned char done = 0;
	//unsigned char tot_bytes_rd = 20;			// Total bytes to read.
	unsigned char tot_bytes_rd = 24;			// Total bytes to read.  Only see 10 bytes. Repeat. No NAK.
	//unsigned char tot_bytes_rd = 11;			// Total bytes to read. Stops at 10 bytes, no repeat.  Has nak.
	//unsigned char tot_bytes_rd = 7;			// Total bytes to read.

	ste_out.done = 0;
	
	if (strt) {
		while(!done)
		{
			switch(i2c_state)
			{
				// I2C in idle state.
				case 0:												// i2c_idl
					prev_st = 0;									// Set previous state.
					break;
				// Send a start condition.
				case 1:												// i2c_start
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					i2c_state = 2; 							// send dev. addr with wr bit.
					break;
				// Send a device address and write bit before wake command.
				case 2:												// i2c_start
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
					prev_st = 2;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Wait before meas. command.
				case 3:												// i2c_start
					//delay(2400);									// Delay
					delay(2000);									// Delay
					prev_st = 3;
					i2c_state = 13;									// Send stop bit.
					break;
				case 4:												// i2c_repeated_start
					//IIC2C1_TX = 1;									// Set for transmit.
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2C1_RSTA = 1;								// Send repeated start.
					prev_st = 4;
					i2c_state = 12;								// send dev. addr with rd bit.
					break;
				// Query for ACK response from slave.
				case 5: 												// I2C_ACK_QRY;
					if (!IIC1S_RXAK)								//	If ACK from slave. 
					{
						if (prev_st == 2)								// If previous command is addr_wr.
							i2c_state = 6;								// Go to read_addr.
						else if (prev_st == 6)
							i2c_state = 4;								// Go to restart state.
						else if (prev_st == 12)
							i2c_state = 13;							// Change direction to read. 
						//i2c_state = 3;								// Wait before read. 
					}
					break;
				// Send sub_addr with multiple read flag.
				case 6:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					//IIC2D = *(i2c_buffer + 1);					// Send the rd_addr with multiple addresses flag.
					IIC2D = strt_addr;							// Send the rd_addr with multiple addresses flag.
					prev_st = 6;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Send a stop and go to slave mode.
				case 8:												// 
					IIC2C1_MST = 0;								// Send a stop (go to slave mode)
					if (prev_st == 3)
						i2c_state = 4;								// Send start bit with receive mode.
					else if (prev_st == 14)
					{
						ste_out.done = 1;							// Finish.
						done = 1;
					}
					break;
				// Wait for device to wake up.
				case 9:												// 
					delay(60);										// Delay.
					prev_st = 9;
					i2c_state = 8;
					break;
				// Send device addr. rd command.
				case 12:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 5);					// Send the device address and W/R bit sets to read.
					prev_st = 12;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Change mode to receive.
				case 13:												// 
					//delay(60);
					delay(30);
					//delay(100);
					IIC2C1_TX = 0;									// Change to read mode.
					prev_st = 13;
					i2c_state = 15; 								// Start to read data.
					break;
				// Wait before sending a stop bit.
				case 14:												
					delay(20);										// Wait.
					i2c_state = 8;									// send_stop_bit.
					prev_st = 14;
					break;
				// Read one byte of data from sensor.
				case 15:												// I2C_RD_BYTE:
					//while(!IIC2S_TCF);							// Wait until ready.
					if (rd_byte_cntr == (tot_bytes_rd - 1))	// One byte before end, not last byte.		
					//if (rd_byte_cntr == tot_bytes_rd - 1)	// One byte before end, not last byte.		
							//IIC2C1_TXAK = 0;							// Send NAK.
						IIC2C1_TXAK = 1;							// Send NAK.
					// When transmission is done, ready to do something. 
					while(!IIC2S_TCF);							// Wait until ready.
					*(ste_out.data + rd_byte_cntr) = IIC2D;// Read one byte of data from sensor.
					rd_byte_cntr += 1;							// Increment counter.
					// If has not read everything.  
					if (rd_byte_cntr < tot_bytes_rd)			
						i2c_state = 16;
					else
						//i2c_state = 8;								// send stop_bit.
						i2c_state = 14;							// Wait.
					prev_st = 15;
					break;
				// Send ACK after byte read.
				case 16:												// I2C_SND_ACK:
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2C1_TXAK = 0;								// Send ACK.
					i2c_state = 15;
					prev_st = 16;
					break;
			} 	// switch 
		}		// while 
	}			// strt 
	return (ste_out);
}
// This is for the Asair AHT20 sensor. 
struct Shtc3Outputs i2c_fsm_aht20_demo(unsigned char strt)
{
	unsigned short int i2c_state = 1;		// Go to state 1.
	unsigned short int prev_st = 0;			// Previous state.
	unsigned char rd_temp = 1;					// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;					// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;				// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;			// Send ack counter.
	unsigned char rd_byte_cntr =0;			// Byte read counter.
	unsigned char cmd_byte = 0;				// Read command byte.
	unsigned char i2c_buffer[8]= 	{
												0x70,	// Send addr. write. 	0	
												0xBE,	// Init command			1
												0x08,	// Init first byte		2
												0x00,	// Init second byte		3 		0
												0xAC,	// Meas. command			4		1
												0x33,	// Meas. first byte		5		2
												0x00, // Meas. sedond byte		6		3
												0x71	// Send addr. read		7		4
												};		
	struct Shtc3Outputs ste_out;				// state machine outputs
	unsigned char done = 0;

	ste_out.done = 0;

	
	if (strt) {
		while(!done)
		{
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
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					if (prev_st == 0)
						i2c_state = 2; 							// send dev. addr with wr bit.
					break;
				/***************************/
				// Send a device address and write bit before wake command.
				case 2:												// i2c_start
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Wait before meas. command.
				case 3:												// i2c_start
					delay(5000);									// Delay
					delay(5000);									// Delay
					delay(5000);									// Delay
					delay(4000);									// Delay
					prev_st = 3;
					i2c_state = 8;									// Send stop bit.
					break;
				/***************************/
				// Send a start condition with receive set.
				case 4:												// i2c_start
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					prev_st = 4;
					i2c_state = 12;								// send dev. addr with rd bit.
					break;
				/***************************/
				// Query for ACK response from slave.
				case 5: 												// I2C_ACK_QRY;
					if (prev_st == 0)								// If previous command is write before wakeup msb command.
						i2c_state = 10;							// Send meas. command.
					else if (prev_st == 10)
						i2c_state = 11;							// send meas. command first byte
					else if (prev_st == 11) 
						i2c_state = 21;							// send meas. command second byte
					else if (prev_st == 12)
						i2c_state = 13;							// change to read
					else if (prev_st == 21)
						i2c_state = 3;								// wait
					break;
				/***************************/
				// Send a stop and go to slave mode.
				case 8:												// 
					IIC2C1_MST = 0;								// Send a stop (go to slave mode)
					if (prev_st == 3)
						i2c_state = 4;								// Start bit.
					else if (prev_st == 9)
						i2c_state = 1;								// Send start bit.
					else if (prev_st == 21)
						i2c_state = 3;
					else if (prev_st == 14)
					//else if (prev_st == 16)
					{
						ste_out.done = 1;							// Finish.
						done = 1;
					}
					break;
				/**************************/
				// Send the meas. command.
				case 10:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 4);					// Send the meas. command.
					prev_st = 10;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send the meas. command first byte.
				case 11:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 5);					// Send the meas. command first byte.
					prev_st = 11;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Send device addr. rd command.
				case 12:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 7);					// Send the device address and W/R bit sets to read.
					prev_st = 12;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				/***************************/
				// Change mode to receive.
				case 13:												// 
					IIC2C1_TX = 0;									// Change to read mode.
					prev_st = 13;
					i2c_state = 15; 								// Start to read data.
					break;
				/***************************/
				// Wait before sending a stop bit.
				case 14:												// I2C_DUMMY_READ:
					delay(200);										// Wait.
					i2c_state = 8;									// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
					break;
				/***************************/
				// Read one byte of data from sensor.
				case 15:												// I2C_RD_BYTE:
					if(rd_byte_cntr == 6)
						IIC2C1_TXAK = 1;							// Send NACK.
					while(!IIC2S_TCF);							// Wait until ready.
					// When transmission is done, ready to do some thing. 
					*(ste_out.data + rd_byte_cntr) = IIC2D;// Read one byte of data from sensor.
					rd_byte_cntr += 1;							// Increment counter.
					i2c_state = 16;								// Send ACK.
					prev_st = 15;
					break;
				/***************************/
				// Send ACK after byte read.
				case 16:												// I2C_SND_ACK:
					if(rd_byte_cntr < 6)							// If less than 7 bytes.
					{
						while(!IIC2S_TCF);						// Wait until transmission is done.
						IIC2C1_TXAK = 0;							// Send ACK.
					}
					if (rd_byte_cntr < 7)						// If has not read 7 bytes yet.
						i2c_state = 15;
					else
						i2c_state = 14;							// Send stop bit;
					prev_st = 16;
					break;
				//***************************/
				// Send wakeup lsb command.
				case 21:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 6);					// Send meas. command second byte.
					prev_st = 21;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
			} 	// switch 
		}		// while 
	}			// strt 
	return (ste_out);
}
// This is for the Asair AHT20 sensor. 
// Does not work with mixed sensors.
// Need to try again.
struct Shtc3Outputs i2c_fsm_aht20(unsigned char strt)
{
	unsigned short int i2c_state = 1;		// Go to state 1.
	unsigned short int prev_st = 0;			// Previous state.
	unsigned char rd_temp = 1;					// read temp = 1, else read r.h.
	unsigned char snd_cmd = 1;					// 1 is for sending a command after ACK.
	unsigned char rpt_strt = 0;				// Send a repeated start command.
	unsigned char snd_ack_cntr = 0;			// Send ack counter.
	unsigned char rd_byte_cntr =0;			// Byte read counter.
	unsigned char cmd_byte = 0;				// Read command byte.
	unsigned char i2c_buffer[10]= 	{
												0x70,	// Send addr. write. 	0	
												0xBE,	// Wakeup command msb	1
												0x08,	// Wakeup command lsb	2
												0x00,	// Meas. command msb		3 		0
												0xAC,	// Meas. command lsb		4		1
												0x33,	// Send addr. read		5		2
												0x00, // Sleep command msb		6		3
												0x71,	// Sleep command lsb		7		4
												0x00,	// read r.h. data.		8		5
												0x00 	// read r.h. crc			9		6
												};		
	struct Shtc3Outputs ste_out;				// state machine outputs
	unsigned char done = 0;

	ste_out.done = 0;

	
	if (strt) {
		while(!done)
		{
			switch(i2c_state)
			{
				// I2C in idle state.
				case 0:												// i2c_idl
					prev_st = 0;									// Set previous state.
					break;
				// Send a start condition.
				case 1:												// i2c_start
					//IIC2C = 0xb0;									// Send the start bit.
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					if (prev_st == 0)
						i2c_state = 2; 							// send dev. addr with wr bit.
					break;
				// Send a device address and write bit before wake command.
				case 2:												// i2c_start
					while(!IIC2S_TCF);							// Wait until transmission is done.  Wait for any transfer to complete.
					IIC2D = *(i2c_buffer + 0);					// Send the addr. field with WR bit set (R/W = WR).
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Wait before meas. command.
				case 3:												// i2c_start
					delay(5000);									// Delay
					delay(5000);									// Delay
					delay(5000);									// Delay
					delay(5000);									// Delay
					prev_st = 3;
					i2c_state = 8;									// Send stop bit.
					break;
				// Send a start condition with receive set.
				case 4:												// i2c_start
					IIC2C1_TX = 1;									// Set for transmit.
					IIC2C1_MST = 1;								// Set for master transmit.
					prev_st = 4;
					i2c_state = 12;								// send dev. addr with rd bit.
					break;
				// Query for ACK response from slave.
				case 5: 												// I2C_ACK_QRY;
					if (prev_st == 0)								// If previous command is write before wakeup msb command.
						i2c_state = 10;								// Go to wakeup cmd msb.
					else if (prev_st == 10)
						i2c_state = 11;							// send meas. command lsb
					else if (prev_st == 11) 
						i2c_state = 21;							// Wait for measurement.
					else if (prev_st == 12)
						i2c_state = 13;							// wait for read to start 
					else if (prev_st == 16)
						i2c_state = 18;							// send sleep command msb
					else if (prev_st == 21)
						i2c_state = 3;								// Send stop bit state.
					break;
				// Send wakeup msb command.
				//case 6:												// 
				//	while(!IIC2S_TCF);							// Wait until transmission is done.
				//	IIC2D = *(i2c_buffer + 4);					// Send the wakeup msb command.
				//	prev_st = 6;
				//	i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				//	break;
				// Send wakeup lsb command.
				//case 7:												// 
				//	while(!IIC2S_TCF);							// Wait until transmission is done.
				//	IIC2D = *(i2c_buffer + 2);					// Send the wakeup lsb command.
				//	prev_st = 7;
				//	i2c_state = 5; 								// I2C_ACK_QRY;			// next state
				//	break;
				// Send a stop and go to slave mode.
				case 8:												// 
					IIC2C1_MST = 0;								// Send a stop (go to slave mode)
					if (prev_st == 3)
						i2c_state = 4;								// Send start bit with receive mode.
					else if (prev_st == 14)
					//else if (prev_st == 19)
					{
						ste_out.done = 1;							// Finish.
						done = 1;
					}
					break;
				// Wait for device to wake up.
				//case 9:												// 
				//	delay(60);										// Delay.
				//	prev_st = 9;
				//	i2c_state = 8;
				//	break;
				// Send the meas. msb command.
				case 10:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 4);					// Send the meas. msb command.
					prev_st = 10;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Send the meas. lsb command.
				case 11:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 5);					// Send the meas. lsb command.
					prev_st = 11;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Send device addr. rd command.
				case 12:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 7);					// Send the device address and W/R bit sets to read.
					prev_st = 12;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
				// Change mode to receive.
				case 13:												// 
					IIC2C1_TX = 0;									// Change to read mode.
					prev_st = 13;
					i2c_state = 15; 								// Start to read data.
					break;
				// Wait before sending a stop bit.
				case 14:												// I2C_DUMMY_READ:
					delay(50);										// Wait.
					prev_st = 14;
					i2c_state = 8;									// I2C_RD_BYTE;	// Dummy read, does not require an ACK send.
					break;
				// Read one byte of data from sensor.
				case 15:												// I2C_RD_BYTE:
					if(rd_byte_cntr == 6)
						IIC2C1_TXAK = 1;							// Send NACK.
					while(!IIC2S_TCF);							// Wait until ready.
					// When transmission is done, ready to do some thing. 
					*(ste_out.data + rd_byte_cntr) = IIC2D;// Read one byte of data from sensor.
					rd_byte_cntr += 1;							// Increment counter.
					i2c_state = 16;								// Send ACK.
					prev_st = 15;
					break;
				// Send ACK after byte read.
				case 16:												// I2C_SND_ACK:
					if(rd_byte_cntr < 7)							// If less than 7 bytes.
					{
						while(!IIC2S_TCF);						// Wait until transmission is done.
						IIC2C1_TXAK = 0;							// Send ACK.
						i2c_state = 15;
					}
					else
						i2c_state = 14;							// Wait.
					prev_st = 16;
					break;
				// Send the meas. command second byte.
				case 21:												// 
					while(!IIC2S_TCF);							// Wait until transmission is done.
					IIC2D = *(i2c_buffer + 6);					// 
					prev_st = 21;
					i2c_state = 5; 								// I2C_ACK_QRY;			// next state
					break;
			} 	// switch 
		}		// while 
	}			// strt 
	return (ste_out);
}

