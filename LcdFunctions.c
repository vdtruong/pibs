/* LCD functions */
/* 6-7-09 */

#include "lcdglobals.h"
#include "MiscFunctions.c"



/* functions prototype */
void dsplyLabel(void);
void dsplySav(void);
void dsplyRead(void);
void i2c1MstrStart(byte slavAddr, byte slavDir);
void contLcdUpdate(void);
/***********************/

/* begin of functions */
void dsplyLabel(void){  /* LCD labels */
  for(cnt2=0; cnt2 < sizeof(label); cnt2++){   
    if(!IIC1S_RXAK){  
      IIC1D = label[cnt2];  
      delay(500);
    }
    else
      IIC1C1_MST = 0; 
  }
}
void dsplySav(void){
  for(cnt2=0; cnt2 < sizeof(saveEEprmDsply); cnt2++){   
    if(!IIC1S_RXAK){  
      IIC1D = saveEEprmDsply[cnt2];  
      delay(200);
    }
    else
      IIC1C1_MST = 0; 
  }
}
void dsplyRead(void){
  for(cnt2=0; cnt2 < sizeof(readEEprmDsply); cnt2++){   
    if(!IIC1S_RXAK){  
      IIC1D = readEEprmDsply[cnt2];  
      delay(200);
    }
    else
      IIC1C1_MST = 0; 
  }
}
void contLcdUpdate(void){

  for(cnt2=0; cnt2 < sizeof(lcdOutPut); cnt2++){   
    if(!IIC1S_RXAK){  
      IIC1D = lcdOutPut[cnt2];  
      delay(200);
    }
    else
      IIC1C1_MST = 0; 
  }
}
void i2c1MstrStart(byte slavAddr, byte slavDir) { // for LCD    
  IIC1C1_TX   = 1;    // set TX bit for Addr. cycle
  IIC1C1_MST  = 1;    // set master bit to generate a Start
  IIC1D       = slavAddr | slavDir; // send addr. data; lsb is dir. of slave
}
/* end of functions */