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



void main(void) { 
  
  count = 0;
  labelCnt = 0;
  lcdUpdate = 0;
  cnt2 = 0;
  cnt3 = 0;
  row = 0;
  col = 0;
  flag = 0;
  
  initDevice();
  EnableInterrupts; // enable interrupts
  ADCSC1_AIEN = 1;  // Enable ADC interrupt
  APCTL1_ADPC0 = 1; // 1-analog, 0-digital
                                                       
  i2c1MstrStart(LCDSLAVE,0); /* initialize LCD display */              
  delay(20000);
  dsplyLabel(); // for LCD labels

  for(;;) {
  
    /* need these three functions to display 7-seg */
    /* continously */
    display_out(duty_cycle, (byte)0x0F); 
    display_out(normal, all_on); 
    display_out(activate_digit, activate_all_direct);     

    contUpdate(); /* for all updates */  
    sciComm();    /* comm. with labview */
  
  } /* loop forever */
  /* please make sure that you never leave main */
}


