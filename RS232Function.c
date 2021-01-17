/* RS232 Functions */
/* 6-7-09 */

/* functions prototypes */
void SendSCI(byte *array);
void SendSCIOnePkt(unsigned char *array);
/* end of prototypes */


/* begin of functions */
void SendSCI(byte *array)
{
  	word i;
  
  	for(i=0; i<ARRAYSIZE; i++)
	{
    	while (!SCI1S1_TDRE); 	// When it is not empty,
    	SCI1D = *(array+i);   	// then send out to SCI
  	}
}
void SendSCIOnePkt(unsigned char *array)
{
 	unsigned char pkt_size = 7;
  	unsigned char i = 0;
  
  	for(i=0; i < pkt_size; i++)
	{
    	while (!SCI1S1_TDRE);	// When it is not empty,
    	SCI1D = *(array + i);  	// then send out to SCI
  	}
}
/* end of functions */
