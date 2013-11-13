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

#define LogD(...) debug( __FILE__, __LINE__, __VA_ARGS__)

void initLogger(uint8_t isOpen);
void destroyLogger(void);
void debug(const char *file, const long line, const char *format, ...);

#endif /* LOG_H_ */
