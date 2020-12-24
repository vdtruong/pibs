/* Nov 29, 2019
	Create states for senserion iic temp/rh sensor.

	Copy from hcs08 Unleashed text, Fabio Pereira. 
*/

/* Other constants. */
	#define WR 0	// Write bit.
	#define RD 1	// Read bit.
	#define CLK_STR_NM_TEMP	0x7CA2	// clock stretching, normal, temp first
	#define CLK_STR_NM_RH	0x5C24	// clock stretching, normal, rh first

enum ei2c_states				// Sensor capturing/other states.
{	// Original states from Fabio.
	I2C_IDLE,
	I2C_START,
	I2C_REPEATED_START,
	I2C_WRITE_HIGER_ADDR,
	I2C_WRITE_LOVER_ADDR,
	I2C_WRITE_BYTE,
	I2C_DUMMY_READ,
	I2D_RD_BYTE,
	I2C_STOP,
	I2C_SENS_WR,				// write to sensor
	I2C_SENS_RD,				// read from sensor
	// Following are states for senserion sensor states.
	I2C_SND_SLV_ADDR_WR,		// Send slave address to write.
	I2C_SND_SLV_ADDR_RD,		// Send slave address to read.
	I2C_ACK_QRY,				// ACK query.
	I2C_SND_RD_TEMP_CMD,		// Read Temp. command.
	I2C_SND_RD_RH_CMD,		// Read Relative humility command.
	I2C_WAIT,					// Wait state.
	I2C_SND_ACK,				// Send ACK.
	I2C_SND_NAK,				// Send NAK.
	I2C_NACK_QRY,				// NACK query.
	I2C_SND_STOP_BIT,			// Send stop bit.
	I2C_RD_CRC,					// Read CRC.
	I2C_RD_2ND_BYTE,			// Read second byte.
	I2C_STRT_RD,				// Send a start with read bit.
	I2C_STRT_WR,				// Send a start with a write bit.
	I2C_RD_WAIT,				// Wait before reading back data.
	I2C_SND_RD_CMD				// Send the read command.
	// Following are states for Sensirion SHTC3 sensor.

};

enum app_states		// Application states.
{	APP_CAPT_DATA,		// Capture sensor data state.
	APP_DISP_DATA,		// Display data on LCD display state.
	APP_DISP_WEB,		// Display data on Web state.
	APP_WAIT,			// Wait state.
	APP_STOP,			// User press stop?
	APP_SHW_DONE,		// Show done on LCD screen.
	APP_SHW_DONE_WEB	// Show done on web.
};
