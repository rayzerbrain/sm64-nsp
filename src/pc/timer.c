#include <stdint.h>
#include <libndls.h>

static volatile unsigned *timer_load = (unsigned *) 0x900C0000;    // timer base at 0x900C0000
static volatile unsigned *timer_val = (unsigned *) 0x900C0004;     // +0x4
static volatile unsigned *timer_control = (unsigned *) 0x900C0008; // +0x8

static unsigned new_control = 0b11000110; // enable, free-running, no-interrupt, (n/a), prescale / 16 (01), 32-bit, wrapping
static unsigned new_load = 0xffffffff;

static unsigned old_control;
static unsigned old_load;

void timer_start(void) {
    old_control = *timer_control;
    old_load = *timer_load;
    *timer_control = old_control & 0b01111111; // disable timer before modifying it

    *timer_load = new_load;
    *timer_control = new_control; // start the timer with new controls + load

    msleep(1); // ensure timer_val is updated to timer_load
}

uint32_t timer_elapsed(void) {
    return new_load - (*timer_val); // value should decrease at rate of 2 / ms, with /16 prescaling 
}

void timer_shutdown(void) {
    *timer_control = new_control & 0b01111111; // disable timer before restoring old control + load
    *timer_load = old_load;
    *timer_control = old_control;
}