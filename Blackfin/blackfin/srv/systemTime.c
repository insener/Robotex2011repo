
#include "systemTime.h"
#include "timer.h"


//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define	INTERRUPT_FREQUENCY	1000
#define	PRESCALE_VALUE		1
#define DECREMENT_VALUE		(CORE_CLOCK / PRESCALE_VALUE) / (INTERRUPT_FREQUENCY)
#define DISABLE_CORE_TIMER_INTERRUPT *pIMASK &= ~EVT_IVTMR; SSYNC;
#define ENABLE_CORE_TIMER_INTERRUPT  *pIMASK |= EVT_IVTMR;  SSYNC;

//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
void 		 initializeRTC(void);
void 		 initializeCoreTimer(void);
unsigned int getCoreTimerValue(void);
void 		 INTERRUPT_coreTimer(void) __attribute__((interrupt_handler));


//////////////////////////////
// Private global variables
//////////////////////////////
unsigned int _coreTimerCount;
unsigned int _systemTimeBase;


/*
 * Initializes Real Time Clock (RTC) module
 */
void initializeRTC(void)
{
	*pRTC_ICTL = 0;  // disable interrupts
	SSYNC;
	*pRTC_PREN = 0;  // disable prescaler - clock counts at 32768 Hz
	SSYNC;
	*pRTC_STAT = 0;  // clear counter
	SSYNC;
}

/*
 * Initializes core timer (system timer)
 */
void initializeCoreTimer(void)
{
	_coreTimerCount = 0;
	_systemTimeBase = 0;
	// Put core timer in active state enable autoreload
	*pTCNTL |= TAUTORLD | TMPWR;
	// Init the timer scale register with divisor - 1
	*pTSCALE = PRESCALE_VALUE - 1;
	// Load count register with number to decrement from
	*pTCOUNT = DECREMENT_VALUE;
	*pTPERIOD = DECREMENT_VALUE;
	// Set the interrupt to call timer_ISR()
	*pEVT6 = INTERRUPT_coreTimer;
	// Unmask the core timer interrupt
	*pIMASK |= EVT_IVTMR;
	//Enable timer
	*pTCNTL |= TMREN;
}

/*
 * Returns the core timer value
 */
unsigned int getCoreTimerValue(void)
{
	unsigned int count;
	DISABLE_CORE_TIMER_INTERRUPT;
	count = _coreTimerCount;
	ENABLE_CORE_TIMER_INTERRUPT;
	return count;
}

/*
 * Core timer interrupt routine, increments timer's value.
 * Should happen after every 1 ms.
 */
__attribute__((interrupt_handler))
void INTERRUPT_coreTimer(void)
{
	// do not afraid the overflow as program never runs up to 50 days
	_coreTimerCount++;
}

/*
 * Initialize the Real-time Clock, peripheral timer 4 and core timer
 */
void systemTime_init()
{
	// set up real time clock
	initializeRTC();
	// set up peripheral timer 3 for IR beacon receiver, note that starts measuring from falling edge
	// PERIOD_CNT = 0, PULSE_HI = 0, disable and enable the timer in interrupt
	timer_configureTimer(TIMER3, WDTH_CAP | EMU_RUN, WDTHCAP, PERIPHERAL_CLOCK, 1);
	timer_configureTimerInterrupt(TIMER3);
	timer_enableTimer(TIMER3);
/*	*pPORTF_FER |= TIMER3_PIN;
	// *pTIMER3_PERIOD = ??;
	// *pTIMER3_WIDTH  = ??;
	*pTIMER3_CONFIG |= WDTH_CAP | IRQ_ENA;
	SSYNC;
	*pSIC_IMASK |= IRQ_TIMER3;
	SSYNC;
	*pEVT12 = INTERRUPT_timer;
	SSYNC;
	*pIMASK |= EVT_IVG12;
	SSYNC;*/

    // set up peripheral timer 4 for delay functionality
	timer_configureTimer(TIMER4, PULSE_HI | PWM_OUT | PERIOD_CNT, PWMOUT, PERIPHERAL_CLOCK, PERIPHERAL_CLOCK);
	timer_enableTimer(TIMER4);
	// set up core timer
	initializeCoreTimer();
}

/*
 * Read the RTC counter, returns number of milliseconds since reset
 */
int systemTime_readRTC()
{
    int i1, i2;
    i1 = *pRTC_STAT;
    i2 = (i1 & 0x0000003F) + (((i1 >> 6) & 0x0000003F) * 60) +
        (((i1 >> 12) & 0x0000001F) * 3600) + (((i1 >> 17) & 0x00007FFF) * 86400);
    return (i2 / 33);  // converts tick count to milliseconds
                       //    32,768 / 32.77 = 1,000
}

/*
 * Clear the RTC counter value
 */
void systemTime_clearRTC()
{
    *pRTC_STAT = 0;
    SSYNC;
}

/*
 * Does the delay in milliseconds. Max value is 100000 milliseconds (100 seconds)
 */
void systemTime_delayMS(int delay)
{
    int i0;

    if ((delay < 0) || (delay > 100000))
        return;
    i0 = systemTime_readRTC();
    while (systemTime_readRTC() < (i0 + delay))
        continue;
}

/*
 * Does the delay in microseconds. Max value is 100000 microseconds (0.1 second).
 * CORE_CLOCK (MASTER_CLOCK * VCO_MULTIPLIER / CCLK_DIVIDER) = 22,118,000 * 22
 * PERIPHERAL_CLOCK  (CORE_CLOCK / SCLK_DIVIDER) =  CORE_CLOCK / 4 = 121,649,000
 * *pTIMER4_PERIOD = PERIPHERAL_CLOCK, so TIMER4 should be counting a 121.649MHz rate
 */
void systemTime_delayUS(int delay)
{
    int target, start;

    if ((delay < 0) || (delay > 100000))
    {
    	return;
	}
    start = *pTIMER4_COUNTER;
    target = (((PERIPHERAL_CLOCK / 10000) * delay) / 100) + start;

    if (target > PERIPHERAL_CLOCK)
    {
    	// wait for timer to wrap-around
        target -= PERIPHERAL_CLOCK;
        while (*pTIMER4_COUNTER > target)
        {
        	continue;
        }
    }
    while (*pTIMER4_COUNTER < target)
    {
        continue;
    }
}

/*
 * Does the delay in nanoseconds. Max value is 100000 nanoseconds (.1 milliseconds)
 * Minimum possible delay is approximately 10ns.
 */
void systemTime_delayNS(int delay)
{
    int target, start;

    if ((delay < 10) || (delay > 100000))
    {
        return;
    }
    start = *pTIMER4_COUNTER;
    target = (((PERIPHERAL_CLOCK / 10000) * delay) / 100000) + start;

    if (target > PERIPHERAL_CLOCK)
    {
    	// wait for timer to wrap-around
        target -= PERIPHERAL_CLOCK;
        while (*pTIMER4_COUNTER > target)
        {
        	continue;
    	}
    }
    while (*pTIMER4_COUNTER < target)
    {
    	continue;
    }
}

/*
 * Returns systemTimeBase variable, which determines the scheduler for the main loop
 */
unsigned int systemTime_getSystemTimeBase(void)
{
	return _systemTimeBase;
}

/*
 * Checks if main loop has spent certain amount of time. If not then waist time
 * until it has. The purpose is to make main loop iteration with same duration time.
 */
int systemTime_checkMainLoopDuration(void)
{
	static unsigned int lastTime = 0;
	static unsigned int currentTime = 0;
	static unsigned int multiplier = 0;   // main loop multiplier
	int				    returnTrue = 0;

	_systemTimeBase = 0;
	currentTime = getCoreTimerValue();
	if ((currentTime - lastTime) >= MAIN_LOOP_DURATION )
	{
		lastTime = currentTime;
		multiplier++;
		if (multiplier == LONGEST_TIME_BASE_MULTIPLIER)
		{
			multiplier = 0;
		}
		_systemTimeBase |= SYSTEM_MAIN_LOOP_TIME_BASE;
		returnTrue = 1;
	}
	if (multiplier % 3 == 2)
	{
		_systemTimeBase |= SYSTEM_3x_MAIN_LOOP_TIME_BASE;
	}
	if ((multiplier % LONGEST_TIME_BASE_MULTIPLIER) == (LONGEST_TIME_BASE_MULTIPLIER - 1))
	{
		_systemTimeBase |= SYSTEM_12x_MAIN_LOOP_TIME_BASE;
	}
	if (returnTrue)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

