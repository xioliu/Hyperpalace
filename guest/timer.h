#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_FREQ 62500000ULL  // QEMU 默认频率 62.5MHz

void timer_init(void);

#endif