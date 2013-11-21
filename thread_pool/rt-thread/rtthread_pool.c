/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a thread pool base as RT-Thread
 * Created on: 2013-11-14
 */
#include "rtthread_pool.h"

#ifdef EDM_USING_RTT

static ThreadPoolErrCode addTask(pThreadPool const pool,
		void *(*process)(void *arg), void *arg);
static ThreadPoolErrCode destroy(pThreadPool pool);
static void* threadJob(void* arg);
static void syncLock(pThreadPool pool);
static void syncUnlock(pThreadPool pool);

/**
 * This function will initialize the thread pool.
 *
 * @param pool the ThreadPool pointer
 * @param maxThreadNum the max thread number in this ThreadPool
 *
 * @return error code
 */
ThreadPoolErrCode initThreadPool(pThreadPool const pool, uint8_t maxThreadNum) {
	ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;

	if (maxThreadNum > THREAD_POOL_MAX_THREAD_NUM) {
		errorCode = THREAD_POOL_MAX_NUM_ERR;
	}
	if (errorCode == THREAD_POOL_NO_ERR) {
//		pthread_mutex_init(&(pool->queueLock), NULL);
//		pthread_mutex_init(&(pool->userLock), NULL);
//		pthread_cond_init(&(pool->queueReady), NULL);
//		pool->queueHead = NULL;
//		pool->maxThreadNum = maxThreadNum;
//		pool->curWaitThreadNum = 0;
//		pool->isShutdown = FALSE;
//		pool->addTask = addTask;
//		pool->destroy = destroy;
//		pool->lock = syncLock;
//		pool->unlock = syncUnlock;
//		pool->threadID = (pthread_t *) malloc(maxThreadNum * sizeof(pthread_t));
//		for (i = 0; i < maxThreadNum; i++) {
//			pthread_create(&(pool->threadID[i]), NULL, threadJob, pool);
//			LogD("create thread success.Current total thread number is %d",
//					i + 1);
//		}
//		LogD("initialize thread pool success!");
	}
	return errorCode;
}



#endif
