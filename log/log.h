/*
 * log.h
 *
 *  Created on: 2013Äê11ÔÂ10ÈÕ
 *      Author: Armink
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

void initLogger(void);

typedef struct {
	void (*debug)(const char* format, ...);
	void (*destroy)(void);
}Logger;

/* logger object */
Logger Log;

#endif /* LOG_H_ */
