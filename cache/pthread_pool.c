/*
 * pthread_pool.c
 * a thread pool base as pthread
 *  Created on: 2013-11-7
 *      Author: Armink
 */

#include "pthread_pool.h"

static ThreadPoolErrCode addTask(pThreadPool const pool,
		void *(*process)(void *arg), void *arg);
static ThreadPoolErrCode destroy(pThreadPool pool);
static void* threadJob(void* arg);

/**
 * This function will initialize the thread pool.
 *
 * @param pool the ThreadPool pointer
 * @param maxThreadNum the max thread number in this ThreadPool
 *
 * @return error code
 */
ThreadPoolErrCode init(pThreadPool const pool, uint8_t maxThreadNum) {
	ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
	uint8_t i = 0;

	if (maxThreadNum > THREAD_POOL_MAX_THREAD_NUM) {
		errorCode = THREAD_POOL_MAX_NUM_ERR;
	}
	if (errorCode == THREAD_POOL_NO_ERR) {
		pthread_mutex_init(&(pool->queueLock), NULL);
		pthread_cond_init(&(pool->queueReady), NULL);
		pool->queueHead = NULL;
		pool->maxThreadNum = maxThreadNum;
		pool->curWaitThreadNum = 0;
		pool->isShutdown = FALSE;
		pool->addTask = addTask;
		pool->destroy = destroy;
		pool->threadID = (pthread_t *) malloc(maxThreadNum * sizeof(pthread_t));
		for (i = 0; i < maxThreadNum; i++) {
			pthread_create(&(pool->threadID[i]), NULL, threadJob, pool);
			printf("creat thread %#x in thread pool success.Current total thread number is %d%d\n",pool->threadID[i],i);
		}
		printf("initialize thread poll success!\n");
	}
	return errorCode;
}

/**
 * This function will add a task to thread pool.
 *
 * @param pool the ThreadPool pointer
 * @param process task function pointer
 * @param arg task function arguments
 *
 * @return error code
 */
ThreadPoolErrCode addTask(pThreadPool const pool, void *(*process)(void *arg),
		void *arg) {
	ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
	pTask member = NULL;
	pTask newtask = (pTask) malloc(sizeof(Task));
	newtask->process = process;
	newtask->arg = arg;
	newtask->next = NULL;
	/* lock thread poll */
	pthread_mutex_lock(&(pool->queueLock));
	member = pool->queueHead;
	/* task queue is NULL */
	if (member == NULL) {
		pool->queueHead = newtask;
	} else {
		/* look up for queue tail */
		while (member->next != NULL) {
			member = member->next;
		}
		member->next = newtask;
	}
	/* add current waiting thread number */
	pool->curWaitThreadNum++;
	pthread_mutex_unlock(&(pool->queueLock));
	/* wake up a waiting thread to process task */
	pthread_cond_signal(&(pool->queueReady));
	printf("add a task to taskQueue success.Current task total number is %d\n",pool->curWaitThreadNum);
	return errorCode;
}

/**
 * This function will destroy thread pool.
 *
 * @param pool the ThreadPool pointer
 *
 * @return error code
 */
ThreadPoolErrCode destroy(pThreadPool pool) {
	ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
	pTask head = NULL;
	uint8_t i;
	if (pool->isShutdown) {/* thread already shutdown */
		errorCode = THREAD_POOL_ALREADY_SHUTDOWN_ERR;
	}
	if (errorCode == THREAD_POOL_NO_ERR) {
		pool->isShutdown = 1;
		/* wake up all thread from broadcast */
		pthread_cond_broadcast(&(pool->queueReady));
		/* wait all thread exit */
		for (i = 0; i < pool->maxThreadNum; i++) {
			printf("Thread pool will destroy,Waiting the thread %#x exit\n",pool->threadID[i]);
			pthread_join(pool->threadID[i], NULL);
		}
		/* release memory */
		free(pool->threadID);
		pool->threadID = NULL;
		/* destroy task queue */
		while (pool->queueHead != NULL) {
			head = pool->queueHead;
			pool->queueHead = pool->queueHead->next;
			free(head);
		}
		/* destroy mutex and conditional variable */
		pthread_mutex_destroy(&(pool->queueLock));
		pthread_cond_destroy(&(pool->queueReady));
		/* release memory */
		free(pool);
		pool = NULL;
		printf("Thread pool destroy success\n");
	}
	return errorCode;
}

/**
 * This function is thread job.
 *
 * @param pool the ThreadPool pointer
 * @param arg the thread job arguments
 *
 */
void* threadJob(void* arg) {
	pThreadPool pool = NULL;
	while (1) {
		pool = (pThreadPool)arg;
		pTask task = NULL;
		/* lock thread poll */
		pthread_mutex_lock(&(pool->queueLock));
		/* If waiting thread number is 0 ,and thread is not shutdown.
		 * The thread will block.
		 * Before thread bolck the queueLock will unlock.
		 * After thread wake up ,the queueLock will relock.*/
		while (pool->curWaitThreadNum == 0 && !pool->isShutdown) {
			printf("the thread %#x waiting for task add to taskQueue\n",pthread_self());
			pthread_cond_wait(&(pool->queueReady), &(pool->queueLock));
		}
		if (pool->isShutdown) { /* thread pool will shutdown */
			pthread_mutex_unlock(&(pool->queueLock));
			pthread_exit(NULL);
		}
		assert(pool->curWaitThreadNum != 0);
		assert(pool->queueHead != NULL);
		/* load task to thread job */
		pool->curWaitThreadNum--;
		task = pool->queueHead;
		pool->queueHead = task->next;
		pthread_mutex_unlock(&(pool->queueLock));
		/* run task */
		(*(task->process))(task->arg);
		/* release memory */
		free(task);
		task = NULL;
	}
	/* never reach here */
	pthread_exit(NULL);
	return NULL;
}
