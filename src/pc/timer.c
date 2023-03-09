#include <stdint.h>
#include <libndls.h>

#define TIMER_BASE ((unsigned*)0x900C0020)

static volatile unsigned *timer_load = TIMER_BASE;
static volatile unsigned *timer_val = 0x900C0024;
static volatile unsigned *timer_control = 0x900C0028;

static unsigned initial_val = 0xffffffff;
static unsigned new_control = 0b11001010; // enable, free-running, no-interrupt, (n/a), no-prescale (00), 32-bit, wrapping

static unsigned old_load;

void timer_start(void) {
    *timer_control = *timer_control & 0b01111111; // disable timer before modifying it

    if (is_cx2) 
        new_control = 0b11001010; // enable, free-running, no-interrupt, (n/a), no-prescale (00), 32-bit, wrapping
    else
        new_control = 0b11000010; // same except no prescaling, cx ii timer runs 368x faster for some reason

    *timer_control = new_control & 0b01111111; // new controls but disabled

    old_load = *timer_load;
    *timer_load = initial_val;

    *timer_control = new_control; // start the timer with new controls + load
}

uint32_t timer_elapsed_ms(void) {
    uint32_t delta = initial_val - (*timer_val);

    if (is_cx2)
        return delta / 46; // 46000 decrements per second with full prescaling (period / 256)
    else
        return delta / 32; // 32000 decrements per second with no prescaling (period / 1)
}

uint32_t _timer_val(void) {
    return *timer_val;
}

void timer_restart(void) {
    initial_val = *timer_val;
}

void timer_shutdown(void) {
    *timer_load = old_load;
}