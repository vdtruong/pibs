/* 6-12-09 attempt to write the adc samples function
           that returns a pointer to the array of the samples.
           This works, but everytime we call this function
           we will get a different pointer, i.e., a different
           waveform.  If we want to use one waveform for other
           functions, this won't work. Unless, we put all functions
           that use one waveform into one big function that only 
           use one pointer returning. 
   7-11-09 This was solved by storing this pointer to a variable,
           all functions use this variable until the next pointer
           is returned. */
           

/* function prototype */
byte *adcSampl(void); /* function returning a pointer */
/**********************/


/* function begins */

byte *adcSampl(void){ 

  word i;
  static byte adcRes[ARRAYSIZE];
  
  for(i=0; i<ARRAYSIZE; i++){
    adcRes[i] = ADCRL;
  }
  return adcRes; /* return pointer to array */  
}

/* function ends */