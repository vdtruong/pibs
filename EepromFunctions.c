/*  eeprom  functions 
    6-7-09  move to a separate file
    6-8-09  reimplement eeprom read 
    6-10-09 eeprom is written twice, need to fix that. 
            new read eeprom algorithm works, the SDA line
            is pulled up but cannot save or read again 
    6-10-09 fixed the read eeprom routine, can save and read
            eeprom multiple times, followed the flow chart on
            page 236 of reference manual for reading routine.
            still need to solve the write problem of writing twice.
    6-11-09 Solve the writing twice to eeprom problem.  Labview
            was actually writing the command twice. */
    

/* functions prototypes */
void buildEepromMatrx(byte *array);
void i2cMstrStart(byte slavAddr, byte slavDir);
void interrupt VectorNumber_Viicx i2cMstrTX(void);
/* end of prototypes */


/* begin of functions */
void buildEepromMatrx(byte *array){
  
  word i, j, m, k;
  
  k = -17;
  m = 0;
  
  for(i=0; i<ROW; i++){
    eepromData[i][0] = m; // word address
    m+=16;
    k+=16;	
    for(j=1; j<COL; j++){ // start at col 1
      eepromData[i][j] = *(array+(k+j));
    }
  }    
}
void i2cMstrStart(byte slavAddr, byte slavDir) { // for eeprom 
  IIC2C1_TX   = 1;    // set TX bit for Addr. cycle
  IIC2C1_MST  = 1;    // set master bit to generate a Start
  IIC2D       = slavAddr | slavDir; // send addr. data; lsb is dir. of slave
}
void interrupt VectorNumber_Viicx i2cMstrTX(void){ // for eeprom
  
  IIC2S_IICIF = 1; // clear iic int. flag for succ. byte transfer
                   // after sending the master start command
  switch(flag){
      case 0: // write eeprom (flag=0)
        if(!IIC2S_RXAK){                // if ack by slave    
          IIC2D = eepromData[row][col]; // trigger interrupt
    	    delay(10000);  
          col++; // move to next col
          if (col == COL) {
            col = 0; // reset when done with one row
            row++;   // increment to next row
    	      IIC2C1_MST = 0; // need to stop for every 17 bytes
    	      delay(60000);   // let eeprom saves the info.
    	      delay(60000);
    	      if(row < ROW) {
    	        i2cMstrStart(eepromslav,0x00); // start next row
    	        delay(20000); 
    	      }
    	      else {
      		   row = 0;        // reset when done with matrix
      	      IIC2C1_MST = 0; // park the bus when done      	      
      	      dsplyLabel();   // write lcd labels     
    	      }
          }
        }
        else
          IIC2C1_MST = 0; // no ack from slave, send a stop signal
        break;
      case 1: /* read eeprom (flag = 1) */
        if(count == 0){ /* first data byte from eeprom */          
          if(!IIC2S_RXAK){                /* if ack by slave */
            IIC2C1_TX = 0;                /* set for receive */
            eepromReadData[count] = IIC2D;/* first byte from eeprom; int. flag set */
            IIC2C1_TXAK = 0;              /* ack after reception */
            delay(10000);                 /* essential to wait */
            count++;                      /* next array element */
            if (count == (ARRAYSIZE)) {   /* take into account of 0xa1 */
              count = 0;
              IIC2C1_TXAK = 1; /* send no ack, stop comm. with slave */
              IIC2C1_MST = 0;  /* send stop signal */
              dsplyLabel();    /* write lcd labels */
            }                                            
          } 
          else {
            IIC2C1_TXAK = 1; /* send no ack, stop comm. with slave */
            IIC2C1_MST = 0;  /* send stop signal */ 
          }
        } 
        else { /* subsequent data byte */
          if (count == (ARRAYSIZE)) {   /* last byte? take into account of 0xa1 */
            IIC2C1_MST = 0;             /* send stop signal */
            eepromReadData[count] = IIC2D;
            count = 0; 
          } 
          else {/* not last byte */
            if (count == ARRAYSIZE-1){/* 2nd last byte? */
              IIC2C1_TXAK = 1;        /* send no ack, stop comm. with slave */
              delay(10000);
              eepromReadData[count] = IIC2D;
              /*IIC2C1_TXAK = 0; */
              count++;
            } 
            else { /* not 2nd to last byte, just next byte */
              eepromReadData[count] = IIC2D;
              IIC2C1_TXAK = 0;
              delay(10000);
              count++;
            }
          }   
        }
        dsplyLabel();    /* write lcd labels */
        
        /*switch(rcvseq){
          case 0:
            IIC2D = 0x00; // addr. byte
            delay(10000);
            rcvseq++;
            break;
          case 1:
            IIC2C1_RSTA = 1;  // repeat start
            delay(10000);
            IIC2D = 0xA1;     // receive mode 
            delay(10000);
            IIC2C1_TX = 0;    // set for receive
            rcvseq++;
            break;
          case 2:
            eepromReadData[count] = IIC2D;
            IIC2C1_TXAK = 0; // send out ack after reception
            //IIC2C1_TX = 0;
            delay(10000);
            count++;
            if (count == (ARRAYSIZE+1)) { // taken into acount of 0xa1
              count = 0;
              IIC2C1_TXAK = 1; // send no ack, stop comm. with slave
              IIC2C1_MST = 0;
              rcvseq = 0;
              dsplyLabel();    // write lcd labels
            }
            break;
          default:
            break;*/
      default:
        break;
    }
}  
/* end of subroutines */