#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void tmr_init(void); // init, calibrate, + start timer
void tmr_stop(void); // stop (disable) timer
void tmr_start(void); // start (enable) timer
void tmr_restart(void); // reset elapsed ms to 0
uint32_t tmr_ms(void); // elapsed ms
void tmr_shutdown(void);

#endif