/* See MC9S08QE128RM for data sheet.
   2-23-20		First version of pibs.
	3-13-20		Got sci isr to work.
	3-20-20		Got sensor sht3x to work.  Able to change sensor from labview.
*/

#include <hidef.h>      // for EnableInterrupts macro 
#include "derivative.h" // include peripheral declarations 
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
  
  	unsigned char cntrl_reg = 0;																	// Set the counter for the tca9548a. 
	unsigned char tca_done = 0;
	struct Shtc3Outputs sens_outputs;
	unsigned char des_addr[8] = {0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee};	// destination address with write bit 
	unsigned char des_addr_cntr = 0;																// There are eight displays. 
	unsigned char capt_done = 0;
	unsigned char strt = 1;
	unsigned char pkt_size = 56;																	// 8 rows x 7 cols. of data, hdr for first col.
	unsigned char dat_buffr[56];																	// Holds all 8 sensors data.
	unsigned char num_of_dat_pkts = 8;															// 8 packets of temp. sensors.
	unsigned char num_of_dat_vals = 7;															// 7 data values per packet, dummy in front.
	unsigned char i = 0, j = 0;
	unsigned char hts221_strt_addr_arry[4] = {0xA8, 0xAF, 0xB6, 0xBD};				// Starting addresses for data and params. 
	//unsigned char hts221_strt_addr_arry[4] = {0xa8, 0xaf, 0xb6, 0xbd};				// Starting addresses for data and params. 
	unsigned char hts221_strt_addr_cntr = 0;													// Counter for array.
	unsigned char hts221_strt_addr = 0xA8;														// Address to capture the hts211 sensor data.
	unsigned char hts221_1st_addr_flg = 0;														// Set this flag when all params. are taken.
	
	sens_outputs.done = 0;							// Reset.

	EnableInterrupts;									// Enable sci interrupt.
  	initDevice();										// Initialize microcontroller.
  	delay(40);											// Wait
	
	initHt16k33(0);									// Initialize 7-seg displays.  They use iic1.
	delay(5000);
	delay(5000);
	
	for(;;) {
  
  		
		//sens_outputs = i2c_fsm_aht20_demo(strt);							// Capture ad adt7410 temp. sensor data.
		//delay(5000);
		//sens_outputs = i2c_fsm_adt7410(strt);							// Capture ad adt7410 temp. sensor data.
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//sens_outputs = i2c_fsm_sht3x(strt);							// Capture sht3x temp. sensor data.
		//delay(5000);
		//delay(5000);
		//delay(5000);
		//delay(5000);
		
		
		/*
		while(!tca9548a_fsm(0xee, 0, 1));								// Wait until done.
		delay(500);
		sens_outputs = i2c_fsm_aht20(strt);								// Capture asair aht20 temp. sensor data.
		delay(500);
		while(!tca9548a_fsm(0xee, 1, 1));								// Wait until done.
		delay(500);
		//sens_outputs = i2c_fsm_d6t_1a_01(strt);						// Capture d6t_1a_01 temp. sensor data. Does not work.
		//sens_outputs = i2c_fsm_sht3x(strt);							// Capture sht3x temp. sensor data.
		//sens_outputs = i2c_fsm_hts221(strt, hts221_strt_addr);
		//sens_outputs = i2c_fsm_shtc3(strt); 							// Capture asair aht20 temp. sensor data. Does not work.
		sens_outputs = i2c_fsm_aht20(strt);								// Capture asair aht20 temp. sensor data.
		delay(500);
		*/	
		

		// This is the main part. ******************************************
		// The whole routine is for one sensor at a time.
		// Switch tca to each sensor channel.  
		while(!tca9548a_fsm(0xee, cntrl_reg, 1));									// Wait until done.
		delay(5000);
		
		// Capture data.	
		
		while(!sens_outputs.done)
		{				
			// We actually skip the first byte because it is the header, 0x08 or 0x09.
			switch (*(sens_type_arry + cntrl_reg + 1))							// cntrl_reg tells which sensor, 0, 1, ... 7
			{
				case 0x00:																	// sens_type = 0
					sens_outputs = i2c_fsm_shtc3(strt);								// Capture shtc3 temp. sensor data.
					//sens_outputs = i2c_fsm_aht20(strt);							// Capture asair aht20 temp. sensor data.
					break;
				case 0x01:																	// sens_type = 1
					sens_outputs = i2c_fsm_sht3x(strt);								// Capture sht3x temp. sensor data.
					break;
				case 0x02:																	// sens_type = 2
					sens_outputs = i2c_fsm_d6t_1a_01(strt);						// Capture d6t_1a_01 temp. sensor data.
					break;
				case 0x03:																	// sens_type = 3
					//sens_outputs = i2c_fsm_hts221(strt);							// Capture st hts221 temp. sensor data.
					sens_outputs = i2c_fsm_hts221(strt, hts221_strt_addr);
					break;
				case 0x04:																	// sens_type = 4
					sens_outputs = i2c_fsm_aht20(strt);								// Capture asair aht20 temp. sensor data.
					//sens_outputs = i2c_fsm_shtc3(strt);							// Capture shtc3 temp. sensor data.
					break;
				case 0x05:																	// sens_type = 5
					sens_outputs = i2c_fsm_adt7410(strt);							// Capture ad adt7410 temp. sensor data.
					delay(5000);
					delay(5000);
					delay(5000);
					delay(5000);
					delay(5000);
					delay(5000);
					// Try to reduce wait next time.
					//delay(5000);
					//delay(5000);
					//sens_outputs = i2c_fsm_shtc3(strt);							// Capture shtc3 temp. sensor data.
					break;
				default:
					sens_outputs = i2c_fsm_shtc3(strt);								// Capture shtc3 temp. sensor data.
					break;
			}
		}
		

		// Concatenate all data into buffer.
		/*
		for(j = 0; j < num_of_dat_vals; j++)
		{
			if (cntrl_reg == 0)
				*(dat_buffr + cntrl_reg*7 + j) = 0; 								// Save data into buffer.
			else
				*(dat_buffr + cntrl_reg*7 + j) = *(sens_outputs.data + j); 	// Save data into buffer.
		}*/
		
		
		for(j = 0; j < num_of_dat_vals; j++)
		{
			*(dat_buffr + cntrl_reg*7 + j) = *(sens_outputs.data + j); 		// Save data into buffer.
		}
		delay(500);
			

		// Display data to a single (cnrl_reg) 7-seg display.
		// Wait until display is done.
		
		while(!ht16k33_fsm(*(des_addr + des_addr_cntr), sens_outputs.data, 0, *(sens_type_arry + cntrl_reg + 1)));
		des_addr_cntr += 1;																// Move to next display. 
		cntrl_reg += 1;																	// Move to the next tca (sensor) channel.
		

		// If all sensors are done.
		
		if(des_addr_cntr == 8)
		{	
			// Check for new data flag.
			if (new_dat_flg)
			{
				new_dat_flg = 0;															// Reset.
				if (*(sens_type_arry + 0) == 0x08)									// If header is 0x08.
					SendSCIOnePkt(dat_buffr, pkt_size); 							// send to pc data saved
        		else																			// If header is 0x09.
					SendSCIOnePkt(sens_type_arry, 9); 								// send sens_type_array to pc.
        	}
			delay(500);
			des_addr_cntr = 0;															// Reset to first display.
   		cntrl_reg = 0;																	// Reset to first channel.
		}
		delay(500);
		
		sens_outputs.done = 0;															// Reset.
		
	} 	// loop forever 
}		// please make sure that you never leave main 


