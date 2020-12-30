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
//#include "MiscFunctions.c"
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
  
  	/*count = 0;
  	labelCnt = 0;
  	lcdUpdate = 0;
  	cnt2 = 0;
  	cnt3 = 0;
  	row = 0;
  	col = 0;
  	flag = 0;
  	*/

	static unsigned char cntrl_reg = 0;																		/* Set the counter for the tca9548a. */
	static unsigned char tca_done = 0;
	static struct Shtc3Outputs sens_outputs;
	static unsigned char slv_addr[8] = {0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee};	/* slave address with write bit */
	static unsigned char slv_addr_cntr = 0;																/* There are eight displays. */
	
	sens_outputs.done = 0;
  	
  	initDevice();
  	delay(1);						// 1 ms
	//initHt16k33();

	/*
	EnableInterrupts; // enable interrupts
  	ADCSC1_AIEN = 1;  // Enable ADC interrupt
  	APCTL1_ADPC0 = 1; // 1-analog, 0-digital
  	*/

  	//i2c1MstrStart(LCDSLAVE,0); /* initialize LCD display */              
  	//delay(20000);
	//dsplyLabel(); 					// for LCD labels

  	for(;;) {
  		
		/*IIC2C1_TX = 1;		// SEt fror transmit.
		IIC2C1_MST = 1;	// Set for master start bit.
		IIC2D = 0x70;
		//delay(1);
		IIC2C1_MST = 0;	// Set for master stop bit.
		*/
		/* Switch tca to each temp. sensor. */
		// tca_done = tca9548a_fsm(cntrl_reg);
		//while(!tca9548a_fsm(cntrl_reg));		/* Wait until the switch is done. */
		/* Capture temperature data. */
		while(!sens_outputs.done){				
			sens_outputs = i2c_fsm_shtc3(1);	/* Capture temp. sensor data. */
		}
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

		/*while(!sens_outputs.done){				
			sens_outputs = i2c_fsm_shtc3(1);	/* Capture temp. sensor data. */
		/*	}
		delay(1);
		sens_outputs.done = 0;
		*/

    	//contUpdate(); /* for all updates */  
    	//sciComm();    /* comm. with labview */
  
  } 	/* loop forever */
  		/* please make sure that you never leave main */
}


