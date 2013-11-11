/*
 * pthread_pool.h
 *
 *  Created on: 2013-11-9
 *      Author: Armink
 */

#ifndef PTHREAD_POOL_H_
#define PTHREAD_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include "log.h"

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#define sleep(n) Sleep(1000 * (n))
#else
#include <unistd.h>
#endif

#define THREAD_POOL_MAX_THREAD_NUM      16    /**< thread pool max setting thread number */

/* thread pool error code */
typedef enum{
	THREAD_POOL_NO_ERR,                 /**< no error */
	THREAD_POOL_MAX_NUM_ERR,            /**< max thread number out of range */
	THREAD_POOL_ALREADY_SHUTDOWN_ERR,   /**< thread pool already shutdown */
}ThreadPoolErrCode;

/* a task queue which run in thread pool */
typedef struct _Task {
	void *(*process)(void *arg); /**< task callback function */
	void *arg;                   /**< task callback function's arguments */
	struct _Task *next;
} Task, *pTask;

/* thread pool struct */
typedef struct _ThreadPool{
	pTask queueHead;            /**< task queue which place all waiting task */
	pthread_mutex_t userLock;   /**< a synchronized lock provided to user */
	pthread_mutex_t queueLock;  /**< task queue mutex lock */
	pthread_cond_t queueReady;  /**< a conditional variable which for task queue ready */
	uint8_t isShutdown;         /**< shutdown state,if shutdown the value will equal TRUE  */
	pthread_t* threadID;        /**< thread queue which in thread pool */
	uint8_t maxThreadNum;       /**< the thread max number in thread pool */
	uint8_t curWaitThreadNum;   /**< the current waiting thread number in thread pool */
	ThreadPoolErrCode (*addTask)(struct _ThreadPool* const pool,
			void *(*process)(void *arg), void *arg);
	ThreadPoolErrCode (*destroy)(struct _ThreadPool* pool);
	void (*lock)(struct _ThreadPool* pool);
	void (*unlock)(struct _ThreadPool* pool);
} ThreadPool,*pThreadPool;

ThreadPoolErrCode initThreadPool(pThreadPool const pool, uint8_t maxThreadNum);

#endif /* PTHREAD_POOL_H_ */
