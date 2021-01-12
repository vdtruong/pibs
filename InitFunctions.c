/* 	Initialization functions */
/* 	6-7-09  
		12-24-20	Changed for pibs.
*/


/* function prototypes */
void MCU_Init(void);
//void ADC_Init (void);
void SCI_Init (void);
void spi2Init(void);
void spi1Init(void);
void iic1Init(void);
void iic2Init(void);
void GPIO_Init(void);
void initDevice(void);
//void initFuncts(void);
//void initHt16k33(void);
/***********************/


/* begin of functions */
void MCU_Init(void) {
  	SOPT1 = 0x23;     // Watchdog disabled. Stop Mode Enabled. Background Pin
                    	// enabled. RESET pin enabled
  	SOPT2_IIC1PS = 1; // move icc1 to port B; see 12.1.1 page 217 and page 102.
  	//SCGC1 = 0x1D;     // Bus Clock to ADC, SCI1, iic1 and iic2 modules are enabled
  	// Don't forget to enable sci later.
	SCGC1 = 0x0C;     // Bus Clock to iic1 and iic2 modules are enabled
  	//SCGC2 = 0x03;     // enable spi1 and spi2 modules
}
/*void ADC_Init (void) {
  	ADCSC1 = 0x20; 	// Interrupt disable. Continuous conversion and channel 0
   	              	// active
  	ADCSC2 = 0x00; 	// Software trigger selected
  	ADCCFG = 0x30; 	// Input clock/2. Long Sample time configuration. 8-bit
                 		// conversion
  	APCTL1 = 0x00; 	// ADC0 pin disable, 1 is for digital i/o.
}*/
void SCI_Init (void){
  	SCI1C1  = 0x00;  	// 8-bit mode. Normal operation
  	SCI1C2  = 0x0C;  	// Receiver interrupt disabled. Transmitter and receiver enabled
  	SCI1C3  = 0x00;  	// Disable all errors interrupts
  	SCI1BDL = 0x1A;  	// This register and the SCI1BDH are used to configure the SCI baud rate
  	SCI1BDH = 0x00;  	//                    BUSCLK                4MHz
                   	// Baud rate = -------------------- = ------------ = 9600bps
                   	//              [SBR12:SBR0] x 16         26 x 16
}
void spi2Init(void){	// for 7-seg. display
  	SPI2BR = 0x75;    // determine the baud rate
  	SPI2C1 = 0x50;    // enable system and master mode
  	SPI2C2 = 0x00;
}
void spi1Init(void){	// for DAC
  	SPI1BR = 0x00;    // determine the baud rate
  	SPI1C1 = 0x50;    // enable system and master mode
  	SPI1C2 = 0x00;
}
/* For demo board, iic1 is on pta. */
void iic1Init(void){
  	//IIC1F = 0x09;     // multiply factor of 1, SCL divider of 32, br=125k
  	IIC1F = 0x0A;     // multiply factor of 1, SCL divider of 36, br=111.11k
  	//IIC1F = 0x0B;     // multiply factor of 1, SCL divider of 40, br=100k
  	//IIC1F = 0x0C;     // multiply factor of 1, SCL divider of 440, br=90.9k
  	IIC1C1_IICEN = 1; // iic is enabled
  	IIC1C1_IICIE = 0; // iic int. not enabled
}
void iic2Init(void){
  	//IIC2F = 0x09;     // multiply factor of 1, SCL divider of 32, br=125k
  	IIC2F = 0x0A;     // multiply factor of 1, SCL divider of 36, br=111.11k
  	//IIC2F = 0x0B;     // multiply factor of 1, SCL divider of 40, br=100k
  	//IIC2F = 0x0C;     // multiply factor of 1, SCL divider of 44, br=90.9k
  	IIC2C1_IICEN = 1; // iic is enabled
  	IIC2C1_IICIE = 0; // iic int. not enabled
}
void GPIO_Init(void) {
  	PTCDD = (byte) (PTCD | 0x3F); // Configure PTC0-PTC5 as outputs
  	PTEDD = (byte) (PTED | 0xC0); // Configure PTE6 and PTE7 pins as outputs
  	PTCD = 0x3F;  						// Put 1's in port C in order to turn off the LEDs
  	PTED = 0xC0;  						// Put 1's in port E in order to turn off the LEDs
  	PTDDD = 0x08; 						// for spi2, ptd data direction; 0-input, 1-output; for 7-seg display
  	PTBDD = 0x20; 						// for spi1, ptb data direction; 0-input, 1-output
  	PTHPE = 0xC0; 						// enable pull ups on PTH7 and PTH6 pins for IIC2, port H, expander and sensors
  	/* For demo board, do not use ptb for iic. */
	PTBPE = 0xC0; 						// enable pull ups on PTB7 and PTB6 pins for iic1, port B, 7-seg displays
}
//void initHt16k33(void){
	/* Initialize all displays. 
	 * Send four init. commands to each 
	 * display.  The displays use iic1. */
//	unsigned char slv_addr[8] = {0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee};	/* slave address with write bit */
//	unsigned char cmd_codes[4] = {0x21, 0xa0, 0xe7, 0x80}; 								/* osc, row_output, dim, blink */
//	unsigned char slv_addr_cntr = 0;																/* There are eight displays. */
//	unsigned char cmd_code_cntr = 0;																/* There are four command codes. */
//	unsigned char done_slv_addr = 0;																/* When done sending initialization for each display. */
//	unsigned char done_cmd_code = 0;																/* When done sending init. command code. */

/*	while(!done_slv_addr){
		while(!done_cmd_code){
			IIC1C1_TX   =	 1; 									// set TX bit for Addr. cycle
  			IIC1C1_MST  = 1;    									// set master bit to generate a Start
  			IIC1D       = *(slv_addr + slv_addr_cntr); 	// send addr. data; lsb is dir. of slave
			IIC1S_IICIF = 1; 										// clear iic int. flag for succ. byte transfer
                   												// after sending the master start command
  			if(!IIC1S_RXAK){                					// if ack by slave    
       		IIC1D = *(cmd_codes + cmd_code_cntr); 		// send command code
		  		IIC1C1_MST = 0; 									// send stop bit
				//delay(10);											// delay after each command
			}
			else
				IIC1C1_MST = 0; 									// send stop bit
			cmd_code_cntr += 1;
			if (cmd_code_cntr > 3)
				done_cmd_code = 1;
		}
		slv_addr_cntr += 1;										// Move to next display.
		if (slv_addr_cntr > 8)
			done_slv_addr = 1;
	}
}*/
void initDevice(void){
  	MCU_Init();       											// initializes the MCU
  	GPIO_Init();      											// initializes GPIO
  	//ADC_Init();       											// Function that initializes the ADC module
  	//SCI_Init();
  	//spi1Init();
  	//spi2Init();
  	iic1Init();
  	iic2Init();
}
/* end of functions */
