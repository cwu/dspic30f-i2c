#include <p30f2020.h>
#include "../i2c_api/i2c_api.h"

_FOSC(CSW_FSCM_OFF & FRC_HI_RANGE);
_FOSCSEL(FRC);

int shift(int x)
{
	return x << 10;
}

int main()
{
	ADPCFG = 0xffff;
	PTCONbits.PTEN=0;
		
	TRISEbits.TRISE0 = 0; // output
	TRISEbits.TRISE1 = 0;
	TRISEbits.TRISE2 = 0;
	TRISEbits.TRISE3 = 0;
	TRISEbits.TRISE4 = 0;
	
	TRISBbits.TRISB0 = 1; // input
	TRISBbits.TRISB1 = 1; // input
	TRISBbits.TRISB2 = 1; // input
	TRISBbits.TRISB3 = 1; // input
			
	I2CSlaveInitialize(25);	
	
	slaveWriteLen = 4;	
	slaveReadLen = 1;	
	slaveReadIds[0] = 0xF00F;
	slaveWriteIds[0] = 0x0000;	
	slaveWriteIds[1] = 0x0002;
	slaveWriteIds[2] = 0x0004;
	slaveWriteIds[3] = 0x00FF;
	int i;
	
	while (1)
	{	
		
		//PORTEbits.RE0 = I2CCONbits.GCEN;
		
        for (i = 0; i < slaveWriteLen; i++)
        {
            slaveWriteBuffer[i] = (PORTB >> i) & 1;
        }
        
        PORTEbits.RE0 = slaveReadBuffer[0] == 0;     
        
        
        /*slaveWriteBuffer[0] = 1;
        
        slaveWriteBuffer[1] = 2;
        slaveWriteBuffer[2] = 3;
        slaveWriteBuffer[3] = 4;*/
        
        
	}
	
	return 0;
}
