/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a thread pool base as RT-Thread
 * Created on: 2013-11-14
 */

#ifndef RTTHREAD_POOL_H_
#define RTTHREAD_POOL_H_

#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include "log.h"
#include "edm_config.h"

#ifdef EDM_USING_RTT

#define THREAD_POOL_THREADS_INIT_TIME   30    /**< threads initialize average waiting time */
#define THREAD_POOL_MAX_THREAD_NUM      16    /**< thread pool max setting thread number */
#define THREAD_POOL_JOB_PRIORITY        10    /**< thread poll job's priority in rt-thread */
#define THREAD_POOL_JOB_STACK_SIZE     512    /**< thread poll job's stack size rt-thread */
#define THREAD_POOL_JOB_TICK             5    /**< thread poll job's time slice in rt-thread */

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
	rt_mutex_t userLock;        /**< a synchronized lock provided to user */
	rt_mutex_t queueLock;       /**< task queue mutex lock */
	rt_sem_t queueReady;        /**< a semaphore which for task queue ready */
	uint8_t isShutdown;         /**< shutdown state,if shutdown the value will equal TRUE  */
	rt_thread_t* threadID;      /**< thread queue which in thread pool */
	uint8_t maxThreadNum;       /**< the thread max number in thread pool */
	uint8_t curWaitThreadNum;   /**< the current waiting thread number in thread pool */
	ThreadPoolErrCode (*addTask)(struct _ThreadPool* const pool,
			void *(*process)(void *arg), void *arg);
	ThreadPoolErrCode (*destroy)(struct _ThreadPool* pool);
	void (*lock)(struct _ThreadPool* pool);
	void (*unlock)(struct _ThreadPool* pool);
} ThreadPool,*pThreadPool;

ThreadPoolErrCode initThreadPool(pThreadPool const pool, uint8_t maxThreadNum);

#endif

#endif /* RTTHREAD_POOL_H_ */
