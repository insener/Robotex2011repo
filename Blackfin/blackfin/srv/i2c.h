#ifndef I2C_H_
#define I2C_H_

#include "config.h"

//////////////////////////////
// Public global defines
//////////////////////////////
#define SCCB_ON  1   // enables SCCB mode for I2C (used by OV9655 camera)
#define SCCB_OFF 0

//////////////////////////////
// Public global type definitions
//////////////////////////////


//////////////////////////////
// Public global variables
//////////////////////////////


//////////////////////////////
// Public global functions
//////////////////////////////
extern void i2c_write(unsigned char i2c_device, unsigned char *i2c_data, unsigned int pair_count, int sccb_flag);
extern void i2c_writeX(unsigned char i2c_device, unsigned char *i2c_data, unsigned int count, int sccb_flag);
extern void i2c_read(unsigned char i2c_device, unsigned char *i2c_data, unsigned int pair_count, int sccb_flag);
extern void i2c_slowRead(unsigned char i2c_device, unsigned char *i2c_data, unsigned int pair_count, int sccb_flag);

#endif
