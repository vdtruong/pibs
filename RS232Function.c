/* RS232 Functions */
/* 6-7-09 */

/* functions prototypes */
void SendSCI(byte *array);
/* end of prototypes */


/* begin of functions */
void SendSCI(byte *array){

  word i;
  
  for(i=0; i<ARRAYSIZE; i++){
    while (!SCI1S1_TDRE); // When it is not empty,
    SCI1D = *(array+i);   // then send out to SCI
  }
}
/* end of functions */