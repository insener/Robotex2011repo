
#include "i2c.h"

#define PRESCALE120M 12  // factor = 12, 120MHz/10MHz

void i2c_write(unsigned char i2c_device, unsigned char *i2c_data, unsigned int pair_count, int sccb_flag)
{    
    int i;    

    *pTWI_FIFO_CTL |= XMTFLUSH; // Clear the TX FIFO
    *pTWI_MASTER_STAT = BUFWRERR | BUFRDERR | DNAK | ANAK | LOSTARB; // Clear all status error
    SSYNC;
    *pTWI_FIFO_CTL = 0; // Clear the bit manually
    
    if (sccb_flag)
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M | SCCB; // PRESCALE = fsclk/10MHz
    else
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M; // PRESCALE = fsclk/10MHz
        
    *pTWI_CLKDIV = CLKLOW(67) | CLKHI(33);  // For 100KHz SCL speed: CLKDIV = (1/100KHz)/(1/10MHz) = 100
                        //SCL symetric: CLKHI = 50, CLKLOW = 50
    *pTWI_MASTER_ADDR = i2c_device; // Target address (7-bits plus the read/write bit the TWI controls
    for (i = 0; i < pair_count; i++) { // # of configurations to send to the sensor
        *pTWI_XMT_DATA8 = *i2c_data++; // Pointer to an array and load a value 
        *pTWI_MASTER_CTL = 0x80 | MEN; // Start transmission of 2 bytes
        while (*pTWI_FIFO_STAT == XMTSTAT) // wait to load the next sample into the TX FIFO
            SSYNC;

        *pTWI_XMT_DATA8 = *i2c_data++; 
            SSYNC;
        while ((*pTWI_INT_STAT & MCOMP) == 0) // Wait until transmission complete and MCOMP is set
            SSYNC;        
        *pTWI_INT_STAT = XMTSERV | MCOMP; // service TWI for next transmission
    }
}

/*
 * Write 3 bytes to i2c device
 */
void i2c_writeX(unsigned char i2c_device, unsigned char *i2c_data, unsigned int count, int sccb_flag)
{    
    int i;    

    *pTWI_MASTER_STAT = BUFWRERR | BUFRDERR | DNAK | ANAK | LOSTARB; // Clear all status error
    SSYNC;
    *pTWI_FIFO_CTL = XMTFLUSH | RCVFLUSH; // Clear the TX FIFO
    SSYNC;
    *pTWI_FIFO_CTL = 0; // Clear the bit manually
    SSYNC;
    *pTWI_INT_MASK = 0;
    SSYNC;
    
    if (sccb_flag)
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M | SCCB; // PRESCALE = fsclk/10MHz
    else
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M; // PRESCALE = fsclk/10MHz
        
    *pTWI_CLKDIV = CLKLOW(67) | CLKHI(33);  // For 100KHz SCL speed: CLKDIV = (1/100KHz)/(1/10MHz) = 100
                        //SCL symetric: CLKHI = 50, CLKLOW = 50
    SSYNC;
    *pTWI_MASTER_ADDR = i2c_device; // Target address (7-bits plus the read/write bit the TWI controls
    SSYNC;
    for (i = 0; i < count; i++) { // # of configurations to send to the sensor
        while ((*pTWI_FIFO_STAT & XMTSTAT) ==  XMT_FULL) // Wait until xmit buffer empty
            SSYNC;        
        *pTWI_XMT_DATA8 = *i2c_data++; // Load a value 
        if (i == 0)
            *pTWI_MASTER_CTL = (count << 6) | MEN; // Start transmission of n bytes
        SSYNC;
    }
    while (*pTWI_MASTER_STAT & MPROG)  // Wait for completion
        SSYNC;
}

void i2c_read(unsigned char i2c_device, unsigned char *i2c_data, unsigned int data_count, int sccb_flag)
{
    int i;    

    *pTWI_MASTER_STAT = BUFWRERR | BUFRDERR | DNAK | ANAK | LOSTARB; // Clear all status error
    SSYNC;
    *pTWI_FIFO_CTL = XMTFLUSH | RCVFLUSH;          // Flush FIFO
    SSYNC;
    *pTWI_FIFO_CTL = 0; 
    SSYNC;
    *pTWI_INT_STAT = 0xFF; 
    SSYNC;

    if (sccb_flag)
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M | SCCB; // PRESCALE = fsclk/10MHz
    else
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M;

    *pTWI_CLKDIV = CLKLOW(67) | CLKHI(33);    // For 100KHz SCL speed: CLKDIV = (1/100KHz)/(1/10MHz) = 
                              //  100 -> SCL symmetric: CLKHI = 50, CLKLOW = 50 
                              //  current settings of 120/80 yield 50kHz SCL for slow uBlox5 I2C interface
    *pTWI_MASTER_ADDR = i2c_device; // Target address (7-bits plus the read/write bit the TWI controls

    while ((*pTWI_FIFO_STAT & XMTSTAT) == XMT_FULL)
        SSYNC;
    *pTWI_XMT_DATA8 = *i2c_data; // send the start register 
    *pTWI_MASTER_CTL = 0x40 | MEN;     
    while ((*pTWI_INT_STAT & MCOMP) == 0) // Wait until transmission complete and MCOMP is set
        SSYNC;        
    *pTWI_INT_STAT = XMTSERV | MCOMP; // service TWI for next transmission

    SSYNC;
    *pTWI_MASTER_CTL = (data_count << 6) | MEN | MDIR; 
    for (i=0; i<data_count; i++) {
        while ((*pTWI_FIFO_STAT & RCVSTAT) == 0)
            SSYNC;
        *i2c_data++ = *pTWI_RCV_DATA8; // Load the next sample into the TX FIFO. 
        SSYNC;
    }
}

void i2c_slowRead(unsigned char i2c_device, unsigned char *i2c_data, unsigned int data_count, int sccb_flag)
{
    int i;    

    *pTWI_MASTER_STAT = BUFWRERR | BUFRDERR | DNAK | ANAK | LOSTARB; // Clear all status error
    SSYNC;
    *pTWI_FIFO_CTL = XMTFLUSH | RCVFLUSH;          // Flush FIFO
    SSYNC;
    *pTWI_FIFO_CTL = 0; 
    SSYNC;
    *pTWI_INT_STAT = 0xFF; 
    SSYNC;

    if (sccb_flag)
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M | SCCB; // PRESCALE = fsclk/10MHz
    else
        *pTWI_CONTROL = TWI_ENA | PRESCALE120M;

    *pTWI_CLKDIV = CLKLOW(120) | CLKHI(80);    // For 100KHz SCL speed: CLKDIV = (1/100KHz)/(1/10MHz) = 
                              //  100 -> SCL symmetric: CLKHI = 50, CLKLOW = 50 
                              //  current settings of 120/80 yield 50kHz SCL for slow uBlox5 I2C interface
    *pTWI_MASTER_ADDR = i2c_device; // Target address (7-bits plus the read/write bit the TWI controls

    while ((*pTWI_FIFO_STAT & XMTSTAT) == XMT_FULL)
        SSYNC;
    *pTWI_XMT_DATA8 = *i2c_data; // send the start register 
    *pTWI_MASTER_CTL = 0x40 | MEN;     
    while ((*pTWI_INT_STAT & MCOMP) == 0) // Wait until transmission complete and MCOMP is set
        SSYNC;        
    *pTWI_INT_STAT = XMTSERV | MCOMP; // service TWI for next transmission

    SSYNC;
    *pTWI_MASTER_CTL = (data_count << 6) | MEN | MDIR; 
    for (i=0; i<data_count; i++) {
        while ((*pTWI_FIFO_STAT & RCVSTAT) == 0)
            SSYNC;
        *i2c_data++ = *pTWI_RCV_DATA8; // Load the next sample into the TX FIFO. 
        SSYNC;
    }
}

