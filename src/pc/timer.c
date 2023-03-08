#include <stdint.h>
#include <libndls.h>

#define TIMER_BASE ((unsigned*)0x900C0020)

static volatile unsigned *timer_load = TIMER_BASE;
static volatile unsigned *timer_val = 0x900C0024;
static volatile unsigned *timer_control = 0x900C0028;

static unsigned new_control = 0b11000010; // enable, free-running, no-interrupt, (n/a), no-prescale (00), 32-bit, wrapping

static unsigned orig_val;
static unsigned old_control;
static unsigned old_load;

void timer_start(void) {
    old_control = *timer_control;
    old_load = *timer_load;

    *timer_control = old_control & 0b01111111; // disable timer before modifying it
    *timer_control = new_control & 0b01111111; // new controls but disabled
    orig_val = *timer_val;

    *timer_control = new_control; // start the timer with new controls + load
}

uint32_t timer_elapsed(void) {
    return orig_val - (*timer_val); // value should decrease at rate of 32 / ms, with no prescaling
}

void timer_shutdown(void) {
    *timer_control = new_control & 0b01111111;
    *timer_control = old_control & 0b01111111; // disable timer before restoring old control
    *timer_load = old_load;
    *timer_control = old_control;
}