/*********************************************************************************************
 *$Workfile:   display.h  $  $Revision:   1.1  $
 * 
 * $Archive:   P:/NPD/NPD - Systems/Engineering/PVCS/S4000C/archives/Source/display.h-arc  $
 *
 * $Author:   martinv  $
*/
/****************************************************************************/
/*                               Display Constants                          */
/*                                                                          */
/*  Project:          S4000C Combustible Smart Sensor                       */
/*                                                                          */
/*  Author:           Tony Pipitone                                         */
/*                                                                          */
/*  Creation Date:    01/06/98                                              */
/*                                                                          */
/*  Purpose:          This file contains some constants that will be used   */
/*                    for controlling the display through the SPI interface.*/
/****************************************************************************/
/*                            Modification Record                           */
/*                                                                          */
/* Modification Date       Purpose of modification                          */
/* -----------------       -----------------------                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/* Display command word constants  */

#define blank (unsigned char)0x00
#define normal (unsigned char)0x10
#define duty_cycle (unsigned char)0x20
#define all_on (unsigned char)0x30
#define standby (unsigned char)0x40

#define activate_digit (unsigned char)0x21

#define load_digit_0 (unsigned char)0x22        /* this is the most sig digit (left) */
#define load_digit_1 (unsigned char)0x23
#define load_digit_2 (unsigned char)0x24
#define load_digit_3 (unsigned char)0x25        /* this is the least sig digit (right) */

/* Display data word constants  */

#define DIG_S (unsigned char)0x6D /* this value used for '5' and 'S'  */
#define DIG_A (unsigned char)0x77
#define DIG_b (unsigned char)0x7C
#define DIG_c (unsigned char)0x58
#define DIG_C (unsigned char)0x39
#define DIG_d (unsigned char)0x5E
#define DIG_E (unsigned char)0x79
#define DIG_F (unsigned char)0x71
#define DIG_n (unsigned char)0x54
#define DIG_L (unsigned char)0x38
#define DIG_P (unsigned char)0x73
#define DIG_t (unsigned char)0x78
#define DIG_U (unsigned char)0x3E
#define DIG_r (unsigned char)0x50
#define DIG_o (unsigned char)0x5C
#define DIG_H (unsigned char)0x76
#define DIG_i (unsigned char)0x04
#define DIG_dash (unsigned char)0x40
#define DP_mask (unsigned char)0x80

#define standby_clear (unsigned char)0x01
#define standby_no_clear (unsigned char)0x00
#define activate_all_direct (unsigned char)0x03
#define activate_all_decode (unsigned char)0x13

#define alarm_led (unsigned char)0x01
#define warn_led (unsigned char)0x02

/**************************************************************************************
 * Revision History:
 *
 * $Log:   P:/NPD/NPD - Systems/Engineering/PVCS/S4000C/archives/Source/display.h-arc  $
 * 
 *    Rev 1.1   Feb 26 2003 10:27:00   martinv
 * -Removed Analog Output Error
 * -Improved Overrange Latch Functionality
 * 
 *    Rev 1.0   Dec 31 2002 16:07:40   marinar
 * Initial revision.
 * 
 *  
 * 
 *  
 *************************************************************************************/