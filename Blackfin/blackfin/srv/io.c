/*
 * io.c
 *
 *  Created on: 18.10.2011
 *      Author: peeter
 */

#include "io.h"
#include "timer.h"
#include "srv.h"
#include "stdlib.h"

//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global defines
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
void INTERRUPT_ballSensor(void) __attribute__((interrupt_handler));

//////////////////////////////
// Private global variables
//////////////////////////////
volatile unsigned int _ballSensorCount = 0;


/*
 * Initialized MCU's IO periphery (GPIO inputs, LED outputs etc)
 */
void io_initPeriphery(void)
{
	// enable TMR2 for relay, TMR3 for IR beacon left reception and TMR7 for IR beacon right reception
	//*pPORTF_FER |= TIMER2_PIN | TIMER3_PIN | TIMER7_PIN;

	*pPORTGIO_DIR   = LED1 | LED2;      					  // LEDs (PG8 and PG9)
	*pPORTGIO		= 0x0000;								  // clear LED states
	//*pPORTH_FER     = 0x0000;           					  // set portH for GPIO
	// GPIO outputs are serial flow control, UART2 Tx, and relay control (LASER2)
	//*pPORTHIO_DIR   = UART0_FLOW_CRTL | UART2_TX | LASER2;
	//*pPORTHIO       = 0x0000;           					  // set play output high, all others low
	//*pPORTHIO_INEN  = MATCHPORT_RTS0 | BALL_SENSOR | BATTERY ; // enable inputs: Matchport RTS0 (H0), ball sensor (H1), battery (H2)
	// enable inputs
	//*pPORTHIO_INEN  = IR_DIST_LEFT | IR_DIST_RIGHT | GOAL_SELECTION | PLAY_SWITCH;
	//*pPORTHIO_DIR  |= 0x0380;   // set up lasers - note that GPIO-H8 is used for SD SPI select on RCM board
	//*pPORTHIO |= 0x0100;      // set GPIO-H8 high in case it's used for SD SPI select

	/**pPORTHIO_EDGE	 	 = BALL_SENSOR;     		// rising edge detection
	*pPORTHIO_MASKA_SET  = BALL_SENSOR;   	 		// enable pin PH1 channel A interrupt
	SSYNC;
	*pSIC_IMASK     	|= IRQ_PFA_PORTH; 	      	// enable interrupt on SIC_IMASK level
	SSYNC;
	*pEVT11 			 = INTERRUPT_ballSensor; 	// Set interrupt_ballSensor as IVG11 handler
	SSYNC;
	//asm volatile ("cli %0; bitset (%0, 11); sti %0; csync;": "+d"(interrupt));  // set *pIMASK for EVT_IVG11
	*pIMASK				|= EVT_IVG11;*/
}

/*
 * Initialize pin as GPIO
 */
void io_initGpioPin(int pinType, int port, int pin)
{
    switch (pinType)
    {
        case ioInput:
            // set for GPIO, set direction to input and enable input
            switch (port)
            {
                case portF:
                    *pPORTF_FER &= ~pin;
                    *pPORTFIO_DIR &= ~pin;
                    *pPORTFIO_INEN |= pin;
                    break;
                case portG:
                    *pPORTG_FER &= ~pin;
                    *pPORTGIO_DIR &= ~pin;
                    *pPORTGIO_INEN |= pin;
                    break;
                case portH:
                    *pPORTH_FER &= ~pin;
                    *pPORTHIO_DIR &= ~pin;
                    *pPORTHIO_INEN |= pin;
                    break;
            }
            break;
        case ioOutput:
            // set for GPIO, set direction to input and enable input
            switch (port)
            {
                case portF:
                    *pPORTF_FER &= ~pin;
                    *pPORTFIO_DIR |= pin;
                    break;
                case portG:
                    *pPORTG_FER &= ~pin;
                    *pPORTGIO_DIR |= pin;
                    break;
                case portH:
                    *pPORTH_FER &= ~pin;
                    *pPORTHIO_DIR |= pin;
                    break;
            }
            break;
        default:
            break;
    }
}

/*
 * Returns the value of how many times the sensor's beam has been cut
 */
unsigned int io_getBallSensorCount(void)
{
	unsigned int count;
	DISABLE_BALL_SENSOR_INTERRUPT; // disable interrupt
	SSYNC;
	count = _ballSensorCount;
	ENABLE_BALL_SENSOR_INTERRUPT;  // enable interrupt
	SSYNC;
	return count;
}

/*
 * Enables ball sensor interrupt
 */
void io_enableBallSensorInterrupt(void)
{
	*pPORTHIO_MASKA_SET  = BALL_SENSOR;
	SSYNC;
	_ballSensorCount = 0;
}

/*
 * Disables ball sensor interrupt
 */
void io_disableBallSensorInterrupt(void)
{
	*pPORTHIO_MASKA_CLEAR  = BALL_SENSOR;
	SSYNC;
}

/*
 * Interrupt routine!!! Detects ball sensor rising edge.
 */
__attribute__((interrupt_handler))
void INTERRUPT_ballSensor(void)
{
	*pPORTHIO_CLEAR = BALL_SENSOR;
	SSYNC;
	io_disableBallSensorInterrupt();
	_ballSensorCount++;
	srv_relayOn();
	//timer_enableTimer(TIMER5);
	//io_LED1Set();
}

/*
 * Lights LED1
 */
void io_LED1Set(void)
{
	*pPORTGIO_SET = LED1;
}

/*
 * Clears LED1
 */
void io_LED1Clear(void)
{
	*pPORTGIO_CLEAR = LED1;
}

/*
 * Toggles the state of the LED1
 */
void io_LED1Toggle(void)
{
	*pPORTGIO_TOGGLE = LED1;
}

/*
 * Lights LED2
 */
void io_LED2Set(void)
{
	*pPORTGIO_SET = LED2;
}

/*
 * Clears LED2
 */
void io_LED2Clear(void)
{
	*pPORTGIO_CLEAR = LED2;
}

/*
 * Toggles the state of the LED2
 */
void io_LED2Toggle(void)
{
	*pPORTGIO_TOGGLE = LED2;
}

/*
 * Toggle Port pin
 */
void io_togglePortPin(int port, int pin)
{
	switch(port)
	{
        case portF:
            *pPORTFIO_TOGGLE = pin;
            break;
        case portG:
            *pPORTGIO_TOGGLE = pin;
            break;
        case portH:
            *pPORTHIO_TOGGLE = pin;
            break;
        default:
            break;
	}
}

/*
 * Sets Port pin
 */
void io_setPortPin(int port, int pin)
{
    switch(port)
    {
        case portF:
            *pPORTFIO_SET = pin;
            break;
        case portG:
            *pPORTGIO_SET = pin;
            break;
        case portH:
            *pPORTHIO_SET = pin;
            break;
        default:
            break;
    }
}

/*
 * clears Port pin
 */
void io_clearPortPin(int port, int pin)
{
    switch(port)
    {
        case portF:
            *pPORTFIO_CLEAR = pin;
            break;
        case portG:
            *pPORTGIO_CLEAR = pin;
            break;
        case portH:
            *pPORTHIO_CLEAR = pin;
            break;
        default:
            break;
    }
}

/*
 * Get Port pin value. Receive either 0 or 1
 */
int io_getPortPinValue(int port, int pin)
{
    int value;
    switch(port)
    {
        case portF:
            if ((*pPORTFIO & pin) == pin)
            {
                value = 1;
            }
            else
            {
                value = 0;
            }
            break;
        case portG:
            if ((*pPORTGIO & pin) == pin)
            {
                value = 1;
            }
            else
            {
                value = 0;
            }
            break;
        case portH:
            if((*pPORTHIO & pin) == pin)
            {
                value = 1;
            }
            else
            {
                value = 0;
            }
            break;
        default:
            value = -1;
            break;
    }
    return value;
}
