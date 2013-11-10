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
#include <time.h>
#include "types.h"

void initLogger(void);

typedef struct {
	void (*debug)(const char* format, ...);
	void (*destroy)(void);
}Logger;

Logger Log;

#endif /* LOG_H_ */
