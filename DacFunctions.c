/* DAC functions */
/* 6-7-09 */

/* functions prototypes */
void dacOut(byte command, byte data);
void updateDAC(byte *array);
/* end of prototypes */

/* begin of functions */
// driver for maxim max550A 8-bit DAC chip 
void dacOut(byte command, byte data)
 {  
    while (!SPI1S_SPTEF);  // wait until empty then write
    SPI1D = command;       // Send command byte when it is empty
    while (!SPI1S_SPRF);   // wait until it's full then read
    (void)SPI1D;           // Clear flag, must read after every write
    while (!SPI1S_SPTEF);
    SPI1D = data;          // Send data byte
    while (!SPI1S_SPRF);   // Wait until transfer is complete 
    (void)SPI1D;           // Clear flag
    PTBD_PTBD5 = 1;        // generate strobe pulse at load pin to load 2 bytes
    PTBD_PTBD5 = 0;        // Disable chip select
 }
void updateDAC(byte *array){
  
  byte dac;
  word i;
  
  for(i=0; i<ARRAYSIZE; i++){  
    dac = *(array+i);  // value at this address
    dacOut(0x09, dac); // 0x09 - DAC Register Load Operation Enabled
  }                    // and select Address DAC A
}
/* end of functions */