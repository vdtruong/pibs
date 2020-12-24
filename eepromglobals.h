/* eeprom globals, we need globals because we're using ISR for eeprom */
/* 6-7-09 */

#include "arrayglobals.h"

#define eepromslav 0xa0 // eeprom slave address
#define ROW 16          // row of eeprom
#define COL 17          // col of eeprom
                                                            
word count;
byte eepromData[ROW][COL];// needs to be built up 16 times (rows)
			                    // and 17 bytes (cols) each row
                    	    // each time we need a word address and 16 bytes.
	            	          // we need to do it 16 times because we have 256 bytes.
                    	    // for the col: 0-word address, 1->16 is the data bytes.
byte row, col;            // for eeprom data matrix
byte eepromReadData[ARRAYSIZE];// for reading from eeprom
byte flag, rcvseq;        /* flag=0 is writing; flag=1 is reading */
/**********************************************************************/
