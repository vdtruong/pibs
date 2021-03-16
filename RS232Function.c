/* RS232 Functions */
/* 6-7-09 */

/* functions prototypes */
void SendSCI(byte *array);
void SendSCIOnePkt(unsigned char *array, unsigned char pkt_size);
/* end of prototypes */


/* begin of functions ******************************************************************************/
void SendSCI(byte *array)
{
  	word i;
  
  	for(i=0; i<ARRAYSIZE; i++)
	{
    	while (!SCI1S1_TDRE); 	// When it is not empty,
    	SCI1D = *(array+i);   	// then send out to SCI.
  	}
}
void SendSCIOnePkt(unsigned char *array, unsigned char pkt_size)

{
 	unsigned char i = 0;
  
  	for(i=0; i < pkt_size; i++)
	{
    	while (!SCI1S1_TDRE);	// When it is not empty,
    	SCI1D = *(array + i);  	// then send out to SCI.
  	}
}
void interrupt VectorNumber_Vsci1rx SCI_RX_ISR_PIBS(void)
{
	// Interrupt routine for sci module 1.
	// This routine will caputure each byte coming from the pc.
	// It will keep doing this until 9 bytes have came over.
	// Assumming we are working with 9 bytes.
	// The sensor type array will be set by labview as:
	// hdr, sens1, sens2, sens3, sens4, sens5, sens6, sens7, sens8
	// The hdr could be 0x08 or 0x09.  0x08 will trigger new sensor data
	// to be sent to the pc.  0x09 will only send the sensor type array 
	// back to the pc.
	
	SCI1S1_RDRF = 0;										// Clear the rx data reg. flag.
	*(sens_type_arry + sens_type_cntr) = SCI1D;	// Collect data.
	sens_type_cntr += 1;
	if(sens_type_cntr == 9)
	{
		new_dat_flg = 1;									// Indicate new dat is available.
		sens_type_cntr = 0;								// Reset counter.
	}
}
/* end of functions *********************************************************************************/
