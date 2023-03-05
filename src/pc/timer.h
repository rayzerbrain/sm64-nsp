#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_start(void);
void timer_shutdown(void);
uint32_t timer_elapsed(void);

#endif