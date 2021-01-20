/* 6-13-09 Commands sent from labview */

/* function prototypes **************************************/
void caseSingle(void);
void caseCont(byte temp);
void caseSavEeprom(void);
void caseReadEeprom(void);
void sciComm(unsigned char *pkt, unsigned char pkt_size);
/************************************************************/


/* functions begin ******************************************/

void caseSingle(void){

  byte *ptr;
  
  ptr = adcSampl();     /* get pointer to new waveform */
  buildEepromMatrx(ptr);/* build eeprom matrix for writing */
  SendSCI(ptr);         /* send to PC */ 
  updateDisplay(ptr);   /* update 7-seg. */ 
}
void caseCont(byte temp){
  
  byte *ptr;
  
  do {
    ptr = adcSampl();
    buildEepromMatrx(ptr);
    delay(30000);
    SendSCI(ptr);       /* send adc to PC continously */
    updateDisplay(ptr); /* update 7-seg. */
    contLcdUpdate();    /* lcd update */
    updateDAC(ptr);
    delay(30000);
    if (SCI1S1_RDRF){ /* check serial input */
      SCI1S1_RDRF = 0;/* if it's 0x03, stop these activities */
      temp = SCI1D;   
    }
  } while (temp != 0x03); 
}
void caseSavEeprom(void){
  
  dsplySav();
  col = 0;
  flag = 0; /* eeprom write */
  i2cMstrStart(eepromslav,0); // save to eeprom
  delay(10000); // give eeprom sometime to react
}
void caseReadEeprom(void){
            
   dsplyRead();
   rcvseq = 0;
   count = 0;
   flag = 1; // read from eeprom
   i2cMstrStart(eepromslav,0x01); // read from eeprom
   delay(20000); // give eeprom sometime to react  
}
void sciComm(unsigned char *pkt, unsigned char pkt_size)
{
	unsigned char temp;

	// SCI communication between Labview and micro
  	if (SCI1S1_RDRF)
	{
   	SCI1S1_RDRF = 0; 							// reset flag
    	temp = SCI1D;
      
    	switch (temp)
		{
      	case 0x01:
        		caseSingle();
        		break;
      	case 0x02:
        		caseCont(temp);
        		break;
      	case 0x03: 
        		break;
      	case 0x04:
        		caseSavEeprom();
        		break;
      	case 0x05:
        		caseReadEeprom();
        		break;
      	case 0x06:
        		SendSCI(eepromReadData); 		// send to pc data saved
        		break;
      	case 0x07:
        		i2c1MstrStart(LCDSLAVE,0); 	// send start signal to lcd
        		break;
      	case 0x08:
        		SendSCIOnePkt(pkt, pkt_size); // send to pc data saved
        		break;
      	default:
        		break;
    	}
  	}
}
/* functions end ******************************************************/
