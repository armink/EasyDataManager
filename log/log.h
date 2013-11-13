/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a logger in this library
 * Created on: 2013-11-10
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include "types.h"

#include <time.h>
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else

#endif

void initLogger(uint8_t isOpen);
void destroyLogger(void);

typedef struct {
	void (*d)(const char* format, ...);
	void (*i)(const char* format, ...);
	void (*e)(const char* format, ...);
}Logger;

/* logger object */
Logger Log;

#endif /* LOG_H_ */
