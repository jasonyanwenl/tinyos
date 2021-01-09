#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H

#include "stdint.h"
#include "thread.h"
#include "debug.h"

void timer_init(void);
void mtime_sleep(uint32_t m_seconds);

#endif