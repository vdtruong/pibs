/* 6-13-09 
   This file contains functions to continously update 
   the displays and eeprom matrix for writing. */

/* function prototype */
void contUpdate(void);


/* function begins */

void contUpdate(void){
            
  byte *ptr;

  /* this section captures the waveform
     and display everything and perform
     dac task */
  ptr = adcSampl(); /* points to the returned pointer, 
                       in this way all functions use the same
                       waveform */
  /* get ready for eeprom write */
  buildEepromMatrx(ptr);
  /* write to 7-seg display */
  updateDisplay(ptr);
  /* continous LCD update */
  contLcdUpdate();
  /* digital to analog conversion */
  updateDAC(ptr);
}
/* function ends */