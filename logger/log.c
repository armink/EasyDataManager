/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a logger in this library
 * Created on: 2013-11-10
 */

#include "log.h"

#if defined(EDM_USING_PTHREAD)
static pthread_mutex_t printLock;
#elif defined(EDM_USING_RTT)
static rt_mutex_t printLock;
#endif


static uint8_t isOpenPrint = FALSE;
static uint8_t isInitLog = FALSE;

static void printTime(void);
static void printThreadInfo(void);
static void threadMutexInit(void* mutex);
static void threadMutexLock(void* mutex);
static void threadMutexUnlock(void* mutex);
static void threadMutexDestroy(void* mutex);


/**
 * This function will initialize logger.
 *
 */
void initLogger(uint8_t isOpen) {
	isOpenPrint = isOpen;
	if (isOpen) {
		threadMutexInit(&printLock);
	}
	isInitLog = TRUE;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void debug(const char *file, const long line, const char *format, ...) {
	va_list args;
	if (!isOpenPrint || !isInitLog) {
		return;
	}
	va_start(args, format);
	/* args point to the first variable parameter */
	/* lock the print ,make sure the print data full */
	threadMutexLock(&printLock);
	printTime();
	printThreadInfo();
	printf("(%s:%ld) ", file, line);
	/* must use vprintf to print */
	vprintf(format, args);
	printf("\n");
	threadMutexUnlock(&printLock);
	va_end(args);
}

/**
 * This function destroy the logger.
 *
 */
void destroyLogger(void) {
	if (isOpenPrint) {
		threadMutexDestroy(&printLock);
	}
	isOpenPrint = FALSE;
	isInitLog = FALSE;
}

/**
 * This function is print thread info.
 *
 */
void printThreadInfo(void){

#if defined(EDM_USING_PTHREAD)
#if defined(WIN32) || defined(WIN64)
	printf("tid:%04ld ",GetCurrentThreadId());
#else
	printf("tid:%#x ",pthread_self());
#endif
#elif defined(EDM_USING_RTT)
	printf("thread_name:%s ",rt_thread_self()->name);
#endif

}

/**
 * This function is print time info.
 *
 */
void printTime(void) {
#if defined(EDM_USING_PTHREAD)
#if defined(WIN32) || defined(WIN64)
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);
	printf("%02d-%02d %02d:%02d:%02d.%03d ", currTime.wMonth, currTime.wDay,
			currTime.wHour, currTime.wMinute, currTime.wSecond,
			currTime.wMilliseconds);
#else
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
	if(p==NULL) {
		return;
	}
	printf("%02d-%02d %02d:%02d:%02d.%03d ",p->tm_mon+1 ,p->tm_mday ,p->tm_hour ,p->tm_min,p->tm_sec);
#endif

#elif defined(EDM_USING_RTT)
	printf("tick: %010ld ",rt_tick_get());
#endif
}

/**
 * This function is initialize the printLock mutex.
 *
 * @param mutex the printLock pointer
 *
 */
static void threadMutexInit(void* mutex){
#if defined(EDM_USING_PTHREAD)
	pthread_mutex_init(&((pthread_mutex_t)mutex), NULL);

#elif defined(EDM_USING_RTT)
	(*(rt_mutex_t*)mutex) = rt_mutex_create("printLock",RT_IPC_FLAG_PRIO);
#endif
}

/**
 * This function is lock the printLock mutex.
 *
 * @param mutex the printLock pointer
 *
 */
static void threadMutexLock(void* mutex){
#if defined(EDM_USING_PTHREAD)
	pthread_mutex_lock(&((pthread_mutex_t)mutex));

#elif defined(EDM_USING_RTT)
	rt_mutex_take(*(rt_mutex_t*)mutex,RT_WAITING_FOREVER);
#endif
}

/**
 * This function is unlock the printLock mutex.
 *
 * @param mutex the printLock pointer
 *
 */
static void threadMutexUnlock(void* mutex){
#if defined(EDM_USING_PTHREAD)
	pthread_mutex_unlock(&((pthread_mutex_t)mutex));

#elif defined(EDM_USING_RTT)
	rt_mutex_release(*(rt_mutex_t*)mutex);
#endif
}

/**
 * This function is destroy the printLock mutex.
 *
 * @param mutex the printLock pointer
 *
 */
static void threadMutexDestroy(void* mutex){
#if defined(EDM_USING_PTHREAD)
	pthread_mutex_destroy(&((pthread_mutex_t)mutex));

#elif defined(EDM_USING_RTT)
	rt_mutex_delete(*(rt_mutex_t*)mutex);
#endif
}
