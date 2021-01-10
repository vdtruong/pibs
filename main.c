/* Version 38 See MC9S08QE128RM for data sheet.
   01-03-2009 
   02-02-2009 	got LCD labels to work
   02-07-2009 	got lcd max to work, but has to leave bus on 
              	all the time. 
   02-10-2009 	try to implement iic eeprom, able to transmit 16 bytes
   02-19-2009 	able to write and read 256 bytes with eeprom
   02-28-2009 	can update lcd, save to eeprom and read from eeprom,
              	after saving or reading from eeprom, resume lcd update.
   05-14-2009 	got the LCD and 7-seg. to work with the new box.
              	do some clean up of codes, more subroutines.
   05-15-2009 	try to implement 8-bit DAC
              	doesn't seem to work correctly, the amplitude
              	may be correct but the frequecy is not correct
   06-06-09   	try to implement function prototyping
   6-8-09     	Reimplement the eeprom read routine
   6-12-09    	Try to implement function returning pointer to 
              	array of adc samples. This works, but each function
              	using this function will have a different waveform. 
              	Before, all functions use only one waveform from memory.
              	Maybe we can have a variable that contains the return pointer,
              	and use that variable everywhere else that need that
              	waveform. 
   6-12-09    	Use a pointer to a returned pointer for the new waveform.
              	This works, so all functions use the same waveform.
   6-13-09    	Try to compact main into smaller functions. 
              	This works. 
	12-23-20		First version of pibs.

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
	unsigned char slv_addr[8] = {0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee};	/* slave address with write bit */
	unsigned char slv_addr_cntr = 0;																/* There are eight displays. */
	unsigned char capt_done = 0;
	unsigned char strt = 1;

	sens_outputs.done = 0;
  	
  	initDevice();										// Initialize microcontroller.
  	delay(1000);										// Wait
	
	//initHt16k33();										// Initialize 7-seg displays.
	
	/*
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
	
	ht16k33_test(0xe0, 1);
	
	//tca9548a_fsm(0xee, 1);

	for(;;) {
  		
		/* Switch tca to each temp. sensor. */
		// tca_done = tca9548a_fsm(cntrl_reg);
		
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
   	
    	/* need these three functions to display 7-seg */
    	/* continously */
    	/*
		display_out(duty_cycle, (byte)0x0F); 
    	display_out(normal, all_on); 
    	display_out(activate_digit, activate_all_direct);     
		*/

		// This part works for shtc3 measurement.
		/*while(!sens_outputs.done){				
			sens_outputs = i2c_fsm_shtc3(strt);	// Capture temp. sensor data.
		}
		delay(1000);
		sens_outputs.done = 0;
		strt = 1;*/
		

    	//sciComm();    /* comm. with labview */
  
  } 	/* loop forever */
  		/* please make sure that you never leave main */
}

