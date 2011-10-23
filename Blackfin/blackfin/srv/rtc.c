#include "rtc.h"

/*
 * Initialize the Real-time Clock
 */
void rtc_init() {
    *pRTC_ICTL = 0;  // disable interrupts
    SSYNC;
    *pRTC_PREN = 0;  // disable prescaler - clock counts at 32768 Hz
    SSYNC;
    *pRTC_STAT = 0;  // clear counter
    SSYNC;
}

/*
 * Read the RTC counter, returns number of milliseconds since reset
 */
int rtc_read() {
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
void rtc_clear() {
    *pRTC_STAT = 0;
    SSYNC;
}

void rtc_delayMS(int delay) {  // delay up to 100000 millisecs (100 secs)
    int i0;

    if ((delay < 0) || (delay > 100000))
        return;
    i0 = rtc_read();
    while (rtc_read() < (i0 + delay))
        continue;
}

void rtc_delayUS(int delay) {  // delay up to 100000 microseconds (.1 sec)
    // CORE_CLOCK (MASTER_CLOCK * VCO_MULTIPLIER / CCLK_DIVIDER) = 22,118,000 * 22
    // PERIPHERAL_CLOCK  (CORE_CLOCK / SCLK_DIVIDER) =  CORE_CLOCK / 4 = 121,649,000
    // *pTIMER4_PERIOD = PERIPHERAL_CLOCK, so TIMER4 should be counting a 121.649MHz rate
    int target, start;

    if ((delay < 0) || (delay > 100000))
        return;
    start = *pTIMER4_COUNTER;
    target = (((PERIPHERAL_CLOCK / 10000) * delay) / 100) + start;

    if (target > PERIPHERAL_CLOCK) {  // wait for timer to wrap-around
        target -= PERIPHERAL_CLOCK;
        while (*pTIMER4_COUNTER > target)
            continue;
    }
    while (*pTIMER4_COUNTER < target)
        continue;
}

void rtc_delayNS(int delay) {  // delay up to 100000 nanoseconds (.1 millisec)
    // minimum possible delay is approx 10ns
    int target, start;

    if ((delay < 10) || (delay > 100000))
        return;

    start = *pTIMER4_COUNTER;
    target = (((PERIPHERAL_CLOCK / 10000) * delay) / 100000) + start;

    if (target > PERIPHERAL_CLOCK) {  // wait for timer to wrap-around
        target -= PERIPHERAL_CLOCK;
        while (*pTIMER4_COUNTER > target)
            continue;
    }
    while (*pTIMER4_COUNTER < target)
        continue;
}

