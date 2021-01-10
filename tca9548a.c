/* Nov. 5, 2020
	States for tca9548a.

	Adapted from Fabio Pereira.
	HCS08 Unleashed. 2008
*/

/* Function prototype(s). ************************************************************************/
// Change which i2c channel to use.
unsigned char tca9548a_fsm(unsigned char des_addr, unsigned char cntrl_reg, unsigned char iic_chnl);	
/**************************************************************************************************/

/***** Function begins ****************************************************************************/
/* This is for the tca9548a iic expander. */
unsigned char tca9548a_fsm(unsigned char des_addr, unsigned char cntrl_reg, unsigned char iic_chnl)
{
	unsigned char cntrl_reg_label = 0x01;
	unsigned char done_tx = 0, strt = 1, i2c_state = 1, prev_st = 0;
	
	/* Choose tca channel. */	
	if (cntrl_reg == 0)
		cntrl_reg_label = 0x01;
	else if (cntrl_reg == 1)
		cntrl_reg_label = 0x02;
	else if (cntrl_reg == 2)
		cntrl_reg_label = 0x04;
	else if (cntrl_reg == 3)
		cntrl_reg_label = 0x08;
	else if (cntrl_reg == 4)
		cntrl_reg_label = 0x10;
	else if (cntrl_reg == 5)
		cntrl_reg_label = 0x20;
	else if (cntrl_reg == 6)
		cntrl_reg_label = 0x40;
	else if (cntrl_reg == 7)
		cntrl_reg_label = 0x80;
	// Switch tca channel and return done.
	return(ht16k33_single_cmd_wr(des_addr, cntrl_reg_label, iic_chnl));
	delay(20);
}
