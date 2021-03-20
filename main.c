/* See MC9S08QE128RM for data sheet.
   2-23-20		First version of pibs.
	3-13-20		Got sci isr to work.
	3-20-20		Got sensor sht3x to work.  Able to change sensor from labview.
*/

#include <hidef.h>      /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "display.h"
#include "eepromglobals.h"
#include "arrayglobals.h"

#include "InitFunctions.c"
#include "LcdFunctions.c"
#include "RS232Function.c"
#include "7SegDsply.c"
#include "DacFunctions.c"
#include "EepromFunctions.c"
#include "AdcFunctions.c"
#include "ContUpdate.c"
#include "CommandsFunctions.c"
#include "i2c_sens_states.c"
#include "holtek_ht16k33.c"
#include "tca9548a.c"

void main(void) { 
  
  	unsigned char cntrl_reg = 0;																	/* Set the counter for the tca9548a. */
	unsigned char tca_done = 0;
	struct Shtc3Outputs sens_outputs;
	unsigned char des_addr[8] = {0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee};	/* destination address with write bit */
	unsigned char des_addr_cntr = 0;																/* There are eight displays. */
	unsigned char capt_done = 0;
	unsigned char strt = 1;
	unsigned char pkt_size = 56;																	// 8 rows x 7 cols. of data, hdr for first col.
	unsigned char dat_buffr[56];																	// Holds all 8 sensors data.
	unsigned char num_of_dat_pkts = 8;															// 8 packets of temp. sensors.
	unsigned char num_of_dat_vals = 7;															// 7 data values per packet, header in front.
	unsigned char i = 0, j = 0;

	sens_outputs.done = 0;

	EnableInterrupts;									// Enable sci interrupt.
  	initDevice();										// Initialize microcontroller.
  	delay(40);											// Wait
	
	initHt16k33(0);									// Initialize 7-seg displays.  They use iic1.
	delay(5000);

	/*	Test code.
	ht16k33_single_cmd_wr(0xe0, 0x21);			// osc
	delay(20);
	ht16k33_single_cmd_wr(0xe0, 0xa0);			// row_output
	delay(20);
	ht16k33_single_cmd_wr(0xe0, 0xe7);			// dim
	delay(20);
	ht16k33_single_cmd_wr(0xe0, 0x80);			// blink
	delay(20);
	ht16k33_single_dat_wr(0xe0, 0x00, 0x66);	// Set slav_x, digit 0, value .
	delay(20);
	ht16k33_single_dat_wr(0xe0, 0x02, 0x86);	// Set slav_x, digit 1, value .
	delay(20);
	ht16k33_single_dat_wr(0xe0, 0x06, 0x06);	// Set slav_x, digit 2, value .
	delay(20);
	ht16k33_single_dat_wr(0xe0, 0x08, 0x71);	// Set slav_x, digit 3, value .
	delay(20);
	ht16k33_single_cmd_wr(0xe0, 0x81);			// Turn on the display.
	*/

	// This works.
	/*ht16k33_test(0xe0, 0);	// Test display 1.
	delay(50);
	ht16k33_test(0xe2, 0);	// Test display 2.
	delay(50);
	ht16k33_test(0xe4, 0);	// Test display 3.
	delay(50);
	ht16k33_test(0xe6, 0);	// Test display 4.
	delay(50);
	ht16k33_test(0xe8, 0);	// Test display 5.
	delay(50);
	ht16k33_test(0xea, 0);	// Test display 6.
	delay(50);
	ht16k33_test(0xec, 0);	// Test display 7.
	delay(50);
	ht16k33_test(0xee, 0);	// Test display 8.
	delay(50);
	*/

	//tca9548a_fsm(0xee, 1);

	// This works.
	/* Switch tca to each temp. sensor. */
	/*tca_done = tca9548a_fsm(0xee, 1, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	tca_done = tca9548a_fsm(0xee, 2, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	tca_done = tca9548a_fsm(0xee, 3, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	tca_done = tca9548a_fsm(0xee, 4, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	tca_done = tca9548a_fsm(0xee, 5, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	tca_done = tca9548a_fsm(0xee, 6, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	tca_done = tca9548a_fsm(0xee, 7, 1);
	delay(20);
	sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
	delay(50);
	*/

	for(;;) {
  		// This is the main part. ******************************************
		// The whole routine is for one sensor at a time.
		// Switch tca to each sensor channel.  
		while(!tca9548a_fsm(0xee, cntrl_reg, 1));								// Wait until done.
		delay(500);
		
		// Capture data.
		while(!sens_outputs.done)
		{				
			// We actually skip the first byte because it is the header, 0x08 or 0x09.
			switch (*(sens_type_arry + cntrl_reg + 1))						// cntrl_reg tells which sensor, 0, 1, ... 7
			{
				case 0x00:																// sens_type = 0
					sens_outputs = i2c_fsm_shtc3(strt);							// Capture shtc3 temp. sensor data.
					break;
				case 0x01:																// sens_type = 1
					sens_outputs = i2c_fsm_sht3x(strt);							// Capture sht3x temp. sensor data.
					break;
				case 0x02:																// sens_type = 2
					sens_outputs = i2c_fsm_d6t_1a_01(strt);					// Capture d6t_1a_01 temp. sensor data.
					break;
				default:
					sens_outputs = i2c_fsm_shtc3(strt);							// Capture shtc3 temp. sensor data.
					break;
			}
		}
		
		// Concatenate all data into buffer.
		for(j = 0; j < num_of_dat_vals; j++)
		{
			*(dat_buffr + cntrl_reg*7 + j) = *(sens_outputs.data + j); 	// Save data into buffer.
		}
		delay(500);
		
		// Display data to a single (cnrl_reg) 7-seg display.
		// Wait until display is done.
		while(!ht16k33_fsm(*(des_addr + des_addr_cntr), sens_outputs.data, 0, *(sens_type_arry + cntrl_reg + 1)));
		des_addr_cntr += 1;															// Move to next display. 
		cntrl_reg += 1;																// Move to the next tca (sensor) channel.

		// If all sensors are done.
		if(des_addr_cntr == 8)
		{	
			// Check for new data flag.
			if (new_dat_flg)
			{
				new_dat_flg = 0;														// Reset.
				if (*(sens_type_arry + 0) == 0x08)								// If header is 0x08.
					SendSCIOnePkt(dat_buffr, pkt_size); 						// send to pc data saved
        		else																		// If header is 0x09.
					SendSCIOnePkt(sens_type_arry, 9); 							// send sens_type_array to pc.
        	}
			delay(500);
			des_addr_cntr = 0;														// Reset to first display.
   		cntrl_reg = 0;																// Reset to first channel.
		}
		delay(500);
		
		sens_outputs.done = 0;														// Reset.
		
		/*********************************************************************/

		// Below are test codes. ///////////////////////////////////////////////////
		/* Switch tca to each temp. sensor. */
		//tca_done = tca9548a_fsm(0xee, 1, 1);
		//delay(20);

		//while(!tca9548a_fsm(cntrl_reg));		/* Wait until the switch is done. */
		/* Capture temperature data. */
		
		//while(!sens_outputs.done){				
		//while(!capt_done){				
		//	sens_outputs = i2c_fsm_shtc3(1);	/* Capture temp. sensor data. */
		//	capt_done = sens_outputs.done;
		//}
		
		//sens_outputs = i2c_fsm_shtc3(1);	/* Capture temp. sensor data. */
		//i2c_fsm_shtc3();
		//delay(1050);
		//sens_outputs.done = 0;					// Reset.
		//capt_done = 0;
		
		/* Display data to 7-seg display. */
		/* Wait until display is done. */
		//while(!ht16k33_fsm(*(slv_addr + slv_addr_cntr), sens_outputs.data))
		//slv_addr_cntr += 1;						/* Move to next display. */
		//if(slv_addr_cntr == 8)
		//	slv_addr_cntr = 0;					/* Reset to first display. */
   	
    	////////////////////////////////////////////////////////////////////
		// This part works for one shtc3 measurement.  It includes sci to labview.
		/*while(!sens_outputs.done){				
			sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
		}
		delay(5000);
		
		// If done, send data to pc.  If asked.
		if(sens_outputs.done)
		{
			sciComm(sens_outputs.data, 7);
		}
		sens_outputs.done = 0;						// Reset
		*/
		//strt = 1;
		/////////////////////////////////////////////////////////////////////

		//ht16k33_test(0xe0, 0);	// Test display 1.
		//delay(500);
		//ht16k33_test(0xe2, 0);	// Test display 2.
		//delay(50);
		//ht16k33_test(0xe4, 0);	// Test display 3.
		//delay(50);
		//ht16k33_test(0xe6, 0);	// Test display 4.
		//delay(50);
		//ht16k33_test(0xe8, 0);	// Test display 5.
		//delay(50);
		//ht16k33_test(0xea, 0);	// Test display 6.
		//delay(50);
		//ht16k33_test(0xec, 0);	// Test display 7.
		//delay(50);
		//ht16k33_test(0xee, 0);	// Test display 8.
		//delay(50);

    	//sciComm();    /* comm. with labview */
  
  } 	/* loop forever */
  		/* please make sure that you never leave main */
}

