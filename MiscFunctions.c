/* Misc. functions */
/* 6-7-09 */

/* functions prototypes */
void delay(word value);
void interrupt VectorNumber_Vadc ADC_ISR(void);
/************************/


/* functions begins */
void delay(word value) {
  for (;value; value--);
}
void interrupt VectorNumber_Vadc ADC_ISR(void) {

  byte temp;      // Create a temp variable used for further operations
  temp = ~ADCRL;  // Negate the ADC converted value because it is going to be display
                  // on the LEDs (The LEDs turn on with 0's)
  PTED = (byte) (temp & 0xC0); // Move the acquired ADC value to port E
  PTCD = (byte) (temp & 0x3F); // Move the acquired ADC value to port C
}
/* functions end */