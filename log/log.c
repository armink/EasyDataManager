/*
 * log.c
 *
 *  Created on: 2013Äê11ÔÂ10ÈÕ
 *      Author: Armink
 */

#include "log.h"

static pthread_mutex_t printLock;
static uint8_t isOpenLog = FALSE;
static uint8_t isInitLog = FALSE;

static void debug(const char* format, ...);
static void destroy(void);

/**
 * This function will initialize logger.
 *
 */
void initLogger() {
	isOpenLog = TRUE;
	pthread_mutex_init(&printLock, NULL);
	isInitLog = TRUE;
	Log.debug = debug;
	Log.destroy = destroy;
}

/**
 * This function is print debug info.
 *
 * @param format output format
 * @param ... args
 *
 */
void debug(const char* format, ...) {
	va_list args;
	if (!isInitLog) {
		printf("Logger is not initialize \n");
		return;
	}
	if (!isOpenLog) {
		return;
	}
	/* make args point the first variable parameter */
	va_start(args, format);
	/* lock the print ,make sure the print data full */
	pthread_mutex_lock(&printLock);
	printf("%s %s %#x ", __DATE__, __TIME__, pthread_self());
	/* must use vprintf to print */
	vprintf(format, args);
	pthread_mutex_unlock(&printLock);
	va_end(args);
}

void destroy(void) {
	isOpenLog = FALSE;
	isInitLog = FALSE;
}
