#define HIPWMCNT  625 /*  This sets the period of the pwm to 200 Hz. 
                          Don't forget to change TPM3SC accordingly
                          to get the correct tpm clock. */
#define PWMCNTRES 3   /*  This sets the resolution of the pusle
                          width according to 256 steps of the
                          8 bit adc (2^8).  For example, 625/256 = 2.44. */
#define ADCSTEPS  256 /*  We have 256 adc steps because we are
                          using the 8-bit adc mode. */
#define PWARRAY   20  /*  Pulse width array size. */
#define CANLENGTH 8   /*  CAN message has a length of 8 bytes. */
#define RD			1
#define WR			0
#define CLK_STR_NM_TEMP	0x7CA2
#define CLK_STR_NM_RH	0x5C24
