#ifndef I2C_API_H
#define I2C_API_H

//You main.c should have lines like these to actually allocate storage
//#define INIT_I2C
//#define MAX_WRITE_DATA 10
//#define MAX_READ_DATA 0
//#include "../lib/i2c_api/i2c_api.h"
//#undef INIT_I2C

//WARNING: the 30f5015 needs FRC_PLL4 on to meet the timing requirements
//Our PC does not handle clock stretching sanely (in some cases)

/**
 * To use this module, assign ids to the values you read or write,
 * and indicate how many of each there are. The i2c master will
 * populate the slaveReadBuffer for you and set slaveReadUpdated
 * on each update from the master. You need to fill in the 
 * slaveWriteBuffer with your sensor values.
 * slaveMessageError is set when an errant message is received.
 * While the protocol supports 16 bit ids, the master replaces
 * the upper 8 bits with the device id. Ensure the ids you assign
 * are unique in the lower 8 bits, or the client will see them as 
 * the same value.
 */

#ifdef INIT_I2C

#ifndef MAX_WRITE_DATA
#define MAX_WRITE_DATA 2
#endif

#ifndef MAX_READ_DATA
#define MAX_READ_DATA 2
#endif

volatile unsigned int slaveWriteBuffer[MAX_WRITE_DATA];
volatile unsigned int slaveWriteIds[MAX_WRITE_DATA];
volatile unsigned int slaveReadBuffer[MAX_READ_DATA];
volatile unsigned int slaveReadIds[MAX_READ_DATA];
//tempBuffer should be private but I want the main.c file
//to allocate space for it
unsigned int tempBuffer[MAX_READ_DATA];
#else
extern volatile unsigned int slaveWriteBuffer[];
extern volatile unsigned int slaveWriteIds[];
extern volatile unsigned int slaveReadBuffer[];
extern volatile unsigned int slaveReadIds[];
#endif

extern volatile unsigned char slaveReadUpdated;
extern volatile unsigned char slaveMessageError; //read only
extern volatile unsigned char slaveMsgErrorCount; // read only

/**
 * Initializes and starts this I2C module.
 *
 * Parameters: The 7 bit slave address of this device.
 */
void I2CSlaveInitialize(unsigned char address, 
		unsigned char slaveReadLength, unsigned char slaveWriteLength);

#endif
