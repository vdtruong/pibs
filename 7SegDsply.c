/* 7-seg display functions */
/* 6-7-09 */


/* functions prototypes */
//void display_out(byte command, byte data);
//void updateDisplay(byte *array);
/* end of prototypes */


/* start of functions */
/****************************************************************************/
/* This routine is used for writing to the Toshiba TB62709 Display driver.  */
/****************************************************************************/
void display_out(byte command, byte data)
 {  
    while (!SPI2S_SPTEF);  // wait until empty then write
    SPI2D = command;       // Send command byte when it is empty
    while (!SPI2S_SPRF);   // wait until it's full then read
    (void)SPI2D;           // Clear flag, must read after every write
    while (!SPI2S_SPTEF);
    SPI2D = data;          // Send data byte
    while (!SPI2S_SPRF);   // Wait until transfer is complete 
    (void)SPI2D;           // Clear flag
    PTDD_PTDD3 = 1;        // generate strobe pulse at load pin to load 2 bytes
    PTDD_PTDD3 = 0;        // Disable chip select
 }
void updateDisplay(byte *array) {
  
  word i;
  byte dig_display_hi, dig_display_med, dig_display_low;
  byte highVal, lowVal, temp_hi, temp_med, temp_low; 
  char display_digits[10] = {
        0x3F, // 0
        0x06, // 1
        0x5B,
        0x4F,
        0x66,
        0x6D,
        0x7D,
        0x07,
        0x7F,
        0x6F  // 9
  }; 
  float highValDec, lowValDec, pkToPkVal; // highVal * 0.0121 V
  float a, b, c, d, e, f;
  
  // determine Max value for 7-seg and LCD     
  highVal = *array; // highest value is assumed at the beginning 
                    // value at the first element of the array
                    // first element of the array is the pointer
                    // to the array
  for(i=1; i<ARRAYSIZE; i++){  // important to start at i=1
    if (*(array+i) > highVal){
      highVal = *(array+i);
    }
  }
            
  highValDec = (float) highVal * 0.0121;
     
    if (highValDec >= 1.0) { /* assuming high val. doesn't get higher */
      temp_hi = 1;           /* than 1.9 */
      lcd1 = temp_hi;
      lcdOutPut[3] = lcdDigits[lcd1];
      a = highValDec - 1.0;
      b = a*10.0;
      temp_med = (byte) b;  // taking only the int. part of b
      lcd2 = temp_med;
      lcdOutPut[11] = lcdDigits[lcd2];
      c = b - temp_med;
      d = c*10.0;
      temp_low = (byte) d;
      lcd3 = temp_low;
      lcdOutPut[15] = lcdDigits[lcd3];
      e = d - lcd3;
      f = e*10;
      lcd4 = (byte) f;
      lcdOutPut[19] = lcdDigits[lcd4]; 
    }
    else {
      temp_hi = 0;
      lcd1 = temp_hi;
      lcdOutPut[3] = lcdDigits[lcd1];
      a = highValDec*10.0;
      b = a - (byte) a;
      c = b*10.0; 
      d = c - (byte) c; 
      e = d*10.0;   
      temp_med = (byte) a;
      lcd2 = temp_med;
      lcdOutPut[11] = lcdDigits[lcd2];
      temp_low = (byte) c;
      lcd3 = temp_low;
      lcdOutPut[15] = lcdDigits[lcd3];     
      lcd4 = (byte) e;
      lcdOutPut[19] = lcdDigits[lcd4];
    }
            
    dig_display_hi  = display_digits[temp_hi]|DP_mask;
    dig_display_med = display_digits[temp_med];
    dig_display_low = display_digits[temp_low];
            
    display_out(load_digit_2, dig_display_hi); // write to toshiba 7-seg driver
    display_out(load_digit_1, dig_display_med);
    display_out(load_digit_0, dig_display_low);
    
  /* for LCD Min. value */   
  lowVal = *array; /* lowest value is assumed at the beginning */ 

  for(i=1; i<ARRAYSIZE; i++){  
    if (*(array+i) < lowVal){
      lowVal = *(array+i);
    }
  }
  
  lowValDec = (float) lowVal * 0.0121;

    if (lowValDec >= 1.0) {
      lcd5 = 1;
      lcdOutPut[23] = lcdDigits[lcd5];
      a = lowValDec - 1.0;
      b = a*10.0;
      lcd6 = (byte) b;
      lcdOutPut[31] = lcdDigits[lcd6];
      c = b - lcd6;
      d = c*10.0;
      lcd7 = (byte) d;
      lcdOutPut[35] = lcdDigits[lcd7];
      e = d - lcd7;
      f = e*10;
      lcd8 = (byte) f;
      lcdOutPut[39] = lcdDigits[lcd8]; 
    }
    else {
      lcd5 = 0;
      lcdOutPut[23] = lcdDigits[lcd5];
      a = lowValDec*10.0;
      b = a - (byte) a;
      c = b*10.0; 
      d = c - (byte) c; 
      e = d*10.0;   
      lcd6 = (byte) a;
      lcdOutPut[31] = lcdDigits[lcd6];
      lcd7 = (byte) c;
      lcdOutPut[35] = lcdDigits[lcd7];
      lcd8 = (byte) e;
      lcdOutPut[39] = lcdDigits[lcd8]; 
    }
  
  /* for LCD Pk-Pk */ 
  pkToPkVal = highValDec - lowValDec;
  
  if (pkToPkVal >= 1.0) {
      lcd9 = 1;
      lcdOutPut[43] = lcdDigits[lcd9];
      a = pkToPkVal - 1.0;
      b = a*10.0;
      lcd10 = (byte) b;
      lcdOutPut[51] = lcdDigits[lcd10];
      c = b - lcd10;
      d = c*10.0;
      lcd11 = (byte) d;
      lcdOutPut[55] = lcdDigits[lcd11];
      e = d - lcd11;
      f = e*10;
      lcd12 = (byte) f;
      lcdOutPut[59] = lcdDigits[lcd12]; 
    }             
    else {
      lcd9 = 0;
      lcdOutPut[43] = lcdDigits[lcd9];
      a = pkToPkVal*10.0;
      b = a - (byte) a;
      c = b*10.0; 
      d = c - (byte) c; 
      e = d*10.0;   
      lcd10 = (byte) a;
      lcdOutPut[51] = lcdDigits[lcd10];
      lcd11 = (byte) c;
      lcdOutPut[55] = lcdDigits[lcd11];
      lcd12 = (byte) e;
      lcdOutPut[59] = lcdDigits[lcd12]; 
    }  
} 
/* end of functions */