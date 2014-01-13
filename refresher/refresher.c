/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: an auto refresher to refresh dynamic Cache data in this library
 * Created on: 2014-01-10
 */

#include "refresher.h"

static RefresherErrCode add(pRefresher const refresher, char* name,
		int8_t priority, uint8_t period, uint8_t times, bool_t newThread,
		uint32_t satckSize, void (*refreshProcess)(void *arg));
static RefresherErrCode del(pRefresher const refresher, const char* name);
static RefresherErrCode setPeriodAndPriority(pRefresher const refresher,
		char* name, uint8_t period, int8_t priority);
static void kernel(void* arg);

/**
 * initialize the refresher
 *
 * @param refresher the refresher pointer
 * @param stackSize the refresher kernel thread size
 * @param priority the refresher kernel thread priority
 * @param tick the refresher kernel thread stack tick
 *
 * @return error code
 */
RefresherErrCode initRefresher(pRefresher const refresher, uint32_t stackSize,
		uint8_t priority, uint32_t tick) {
	RefresherErrCode errorCode = REFRESHER_NO_ERR;
	assert(refresher != NULL);
	/* initialize refresher */
	refresher->tick = tick;
	refresher->add = add;
	refresher->del = del;
	refresher->setPeriodAndPriority = setPeriodAndPriority;
	/* create job queue mutex lock */
	refresher->queueLock = rt_mutex_create("refresher", RT_IPC_FLAG_FIFO);
	assert(refresher->queueLock != NULL);
	/* create and start kernel thread */
	refresher->kernel = rt_thread_create("refresher", kernel, refresher,
			stackSize, priority, tick);
	assert(refresher->kernel != NULL);
	rt_thread_startup(refresher->kernel);
	LogD("initialize refresher success");
	return errorCode;
}

/**
 * This function is refresher kernel thread.Run all of job what newThread is false.
 *
 * @param arg the kernel arguments
 *
 */
void kernel(void* arg) {
	//TODO tick轮训延时处理、循环执行任务队列上的任务、注意刷新级别
}

/**
 *	add a job to refresher
 *
 * @param refresher the refresher pointer
 * @param name job name
 * @param priority Job refresh priority.The highest priority is 0.
 * @param period Refresh time = period * refresher tick. @see Refresher.tickTime
 * @param times If it is -1,the job will continuous running.
 * @param newThread If it is TRUE,refresher will new a thread to refresh this job.
 * @param satckSize The new thread job stack size.It is not NULL while newThread is TRUE.
 * @param refreshProcess the job refresh process
 *
 * @return error code
 */
RefresherErrCode add(pRefresher const refresher, char* name, int8_t priority,
		uint8_t period, uint8_t times, bool_t newThread, uint32_t satckSize,
		void (*refreshProcess)(void *arg)) {
	RefresherErrCode errorCode = REFRESHER_NO_ERR;
	pRefreshJob member = NULL;
	pRefreshJob newJob = NULL;

	assert(refresher != NULL);
	assert((name != NULL) && (strlen(name) <= REFRESHER_JOB_NAME_MAX));

	newJob = (pRefreshJob) malloc(sizeof(Refresher));
	assert(newJob != NULL);

	newJob->priority = priority;
	newJob->period = period;
	newJob->times = times;
	newJob->newThread = newThread;
	newJob->refreshProcess = refreshProcess;
	newJob->next = NULL;

	if (newThread) {/* the job need new thread to run */
		newJob->threadID = rt_thread_create(name, refreshProcess,
				refresher, satckSize, priority, refresher->tick);
		assert(newJob->threadID);
	} else {/* the job running kernel thread */
		newJob->threadID = refresher->kernel;
	}
	/* lock job queue */
	rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
	member = refresher->queueHead;
	if (member == NULL) {/* job queue is NULL */
		refresher->queueHead = newJob;
	} else {
		/* look up for queue tail */
		while (member->next != NULL) {
			member = member->next;
		}
		member->next = newJob;
	}
	rt_mutex_release(refresher->queueLock);

	LogD("add a job to refresher success.");
	return errorCode;
}

RefresherErrCode del(pRefresher const refresher, const char* name){
	RefresherErrCode errorCode = REFRESHER_NO_ERR;

	return errorCode;
}

/**
 * set the job period and priority
 *
 * @param refresher the refresher pointer
 * @param name job name
 * @param period job period
 * @param priority job priority
 *
 * @return error code
 */
RefresherErrCode setPeriodAndPriority(pRefresher const refresher, char* name,
		uint8_t period, int8_t priority) {
	RefresherErrCode errorCode = REFRESHER_NO_ERR;
	pRefreshJob member = refresher->queueHead;

	assert(refresher != NULL);
	assert((name != NULL) && (strlen(name) <= REFRESHER_JOB_NAME_MAX));
	/* refresher doesn' have job */
	if (member == NULL) {
		errorCode = REFRESHER_NO_JOB;
	}

	if (errorCode == REFRESHER_NO_ERR) {
		for (;;) {
			if (!strcmp(member->name, name)) {
				member->period = period;
				member->priority = priority;
				break;
			} else {
				if (member->next == NULL) {/* list tail */
					errorCode = REFRESHER_JOB_NAME_ERROR;
				}
				member = member->next;
			}
		}
	}

	return errorCode;
}
