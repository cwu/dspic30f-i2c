#include <p30f2020.h>
#include <timer.h>
#include <i2c.h>

#define START_ADDRESS 25
#define END_ADDRESS 26
#define N_ADDRESSES (END_ADDRESS - START_ADDRESS + 1)

_FOSC(CSW_FSCM_OFF & FRC_HI_RANGE);
_FOSCSEL(FRC);

unsigned char writelen[N_ADDRESSES] = {0};
unsigned char readlen[N_ADDRESSES] = {0};

int writeInfo[N_ADDRESSES][20];
int writeIds[N_ADDRESSES][20];
int readInfo[N_ADDRESSES][20];
int readIds[N_ADDRESSES][20];

int evenParity(int);
void sendAllInfo(unsigned char);
void readAllInfo(unsigned char);
int requestMsgType(unsigned char,unsigned char);
void registerSlave(unsigned char);
int MasterReadIntI2C();
void MasterWriteIntI2C(int);
void delay()
{
	// Idle for a little bit
    unsigned int x;
    for (x = 0; x < 65535; x++)
    {}
}	

int main(){
	TRISEbits.TRISE0 = 0; //output
	TRISEbits.TRISE1 = 0; //output

	OpenI2C(I2C_ON & I2C_IDLE_CON & I2C_CLK_REL & I2C_IPMI_DIS & I2C_7BIT_ADD 
			& I2C_SLW_DIS & I2C_SM_EN & I2C_GCALL_DIS & I2C_STR_EN & I2C_ACK 
			& I2C_ACK_DIS & I2C_RCV_DIS & I2C_STOP_DIS & I2C_RESTART_DIS & I2C_START_DIS,
			0x11);
	IdleI2C();
			
	int deviceExists[N_ADDRESSES] = {0, 0};
	while (1)
	{            
        int i;
        for (i = START_ADDRESS; i < END_ADDRESS; i++)
        {
            deviceExists[i-START_ADDRESS] = requestMsgType(i,1);
            
            if (deviceExists[i-START_ADDRESS])
            {
                registerSlave(i);
            }                                                  
        }   
        
        for (i = START_ADDRESS; i < END_ADDRESS; i++)
        {
            if (deviceExists[i-START_ADDRESS])
            {                
                if (readlen[i-START_ADDRESS] > 0)
                {
                    readAllInfo(i);            
                }
                
                if (writelen[i-START_ADDRESS] > 0)
                {
                    writeInfo[i-START_ADDRESS][0] = evenParity(i-START_ADDRESS);
                    PORTEbits.RE0 = writeInfo[i-START_ADDRESS][0] ;
                    
                    sendAllInfo(i);                      
                }        
            }
        }         
        
        delay();
	}
		
	return 0;
}

int evenParity(int a)
{
    int n = 0, i;
    for (i=0;i<readlen[a];i++)
    {
        int x = readInfo[a][i];
        while (x!=0)
        {
            x = (x - 1) & x;
            n++;
        }
    }
    return n & 1;
}

void sendAllInfo(unsigned char address)
{
    StartI2C();
    while(I2CCONbits.SEN);
    MasterWriteI2C((address<<1)|0);
    while (I2CSTATbits.TRSTAT);
    MasterWriteI2C(7); // type7  
    while (I2CSTATbits.TRSTAT);
    MasterWriteI2C(writelen[address-START_ADDRESS] * 2 + 1); 
    while (I2CSTATbits.TRSTAT);
    int i;
    for (i = 0; i < writelen[address-START_ADDRESS]; i++)
    {
        MasterWriteIntI2C(writeInfo[address-START_ADDRESS][i]);
        while (I2CSTATbits.TRSTAT);
    }
    MasterWriteI2C(writeInfo[0][0]); // crc
    while (I2CSTATbits.TRSTAT);
    IdleI2C();
    StopI2C();
    while(I2CCONbits.PEN);
}

void readAllInfo(unsigned char address)
{
    requestMsgType(address, 7);
    //return;
    StartI2C();
    while(I2CCONbits.SEN);
    MasterWriteI2C((address<<1)|1);
    while (I2CSTATbits.TRSTAT);
    MasterReadI2C();// ' 2 * Nrdv + 2 '
    AckI2C();
    while (I2CCONbits.ACKEN==1);
    MasterReadI2C(); // '11111111'
    AckI2C();
    while (I2CCONbits.ACKEN==1);
    
         
    int i;
    for (i = 0; i < readlen[address-START_ADDRESS]; i++)
    {
        readInfo[address-START_ADDRESS][i] = MasterReadIntI2C();  
        AckI2C();
        while (I2CCONbits.ACKEN==1);
    }
 	MasterReadI2C(); // 0 | merr | CRC
    NotAckI2C();
    while(I2CCONbits.ACKEN==1);

    IdleI2C();
    StopI2C();    
    while(I2CCONbits.PEN);
}


int requestMsgType(unsigned char address, unsigned char type)
{
	StartI2C();
    while(I2CCONbits.SEN);
    MasterWriteI2C(address<<1);
    while (I2CSTATbits.TRSTAT);
    
    if (I2CSTATbits.ACKSTAT!=0)
    {
        IdleI2C();
        StopI2C();
        while (I2CCONbits.PEN);
        return 0;
    }
    
    MasterWriteI2C(type);   
    while (I2CSTATbits.TRSTAT);
    int x = I2CSTATbits.ACKSTAT;
    IdleI2C();
    StopI2C();
    while (I2CCONbits.PEN);
    
    return x==0;
}	

void registerSlave(unsigned char address)
{      
    // read type1   
    StartI2C();
    while(I2CCONbits.SEN);
    MasterWriteI2C((address<<1)|1);   
    while(I2CSTATbits.TRSTAT);
    writelen[address-START_ADDRESS] = (MasterReadI2C() - 2) >> 1; // 2 * Nsdv + 2  
    AckI2C();		
	while(I2CCONbits.ACKEN == 1);
	MasterReadI2C(); // Need to check header and verify writelen   
    AckI2C();		
	while(I2CCONbits.ACKEN == 1);
    
    int i;  
    for (i = 0; i < writelen[address-START_ADDRESS]; i++)
    {
        writeIds[address-START_ADDRESS][i] = MasterReadIntI2C();               
	    AckI2C();		
		while(I2CCONbits.ACKEN == 1);                         
    }   
    MasterReadI2C(); // need to verify error and CRC
    NotAckI2C();
   	while(I2CCONbits.ACKEN == 1); 
    IdleI2C();
    StopI2C();
    while (I2CCONbits.PEN);  
  
  	//if (1) return;
    //request type2
    PORTEbits.RE0 = requestMsgType(address, 2);     
                 
    StartI2C();
    while(I2CCONbits.SEN);
    MasterWriteI2C((address<<1)|1);
    while(I2CSTATbits.TRSTAT);
    readlen[address-START_ADDRESS] = (MasterReadI2C() - 2) >> 1; // 2 * Nrdv + 2  
    AckI2C();		
	while(I2CCONbits.ACKEN == 1);
    MasterReadI2C(); // Need to check header and verify writelen
    AckI2C();		
	while(I2CCONbits.ACKEN == 1);	
    for (i = 0; i < readlen[address-START_ADDRESS]; i++)
    {
        readIds[address-START_ADDRESS][i] = MasterReadIntI2C();   
        AckI2C();		
		while(I2CCONbits.ACKEN == 1);                                 
    }
    MasterReadI2C(); // need to verify error and CRC
    NotAckI2C();		
	while(I2CCONbits.ACKEN == 1);
    IdleI2C();
    StopI2C();
    while (I2CCONbits.PEN);
}

void MasterWriteIntI2C(int x)
{
	MasterWriteI2C(x >> 8);
	while (I2CSTATbits.TRSTAT);
	MasterWriteI2C(x);
}

int MasterReadIntI2C()
{
	int x;
	x = MasterReadI2C();
	AckI2C();
	while (I2CCONbits.ACKEN == 1);
	x = (x << 8) | MasterReadI2C();
	return x;
}		


