/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: an auto refresher to refresh dynamic Cache data in this library
 * Created on: 2014-01-10
 */

#ifndef REFRESHER_H_
#define REFRESHER_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "log.h"
#include "cache.h"
#include "edm_config.h"

#if defined(EDM_USING_PTHREAD)
#include "pthread_pool.h"
#elif defined(EDM_USING_RTT)
#include "rtthread_pool.h"
#endif

#define REFRESHER_JOB_NAME_MAX     CACHE_NAME_MAX       /**< refresher job max name length */

/* refresher error code */
typedef enum{
	REFRESHER_NO_ERR,                 /**< no error */
	REFRESHER_NO_JOB,                 /**< refresher doesn't have job */
	REFRESHER_JOB_NAME_ERROR,         /**< job has name error */
}RefresherErrCode;

/* RefreshJob is an auto refresh job for a Cache data. */
typedef struct _RefreshJob{
	char  name[CACHE_NAME_MAX];         /**< the name of the CacheData is refreshed.@see CacheData */
	int8_t times;                       /**< job running times.If it is -1,the job will continuous running. */
	uint8_t priority;                   /**< refresh priority.The highest priority is 0. */
	uint8_t period;                     /**< refresh time = period * refresher tick. @see Refresher.tickTime */
	bool_t newThread;                   /**< time-consuming or block job set it true will be better. */
	rt_thread_t threadID;               /**< job running thread ID */
	void (*refreshProcess)(void *arg); /**< it will call when the RefreshJob need wrok */
	struct _RefreshJob* next;           /**< point to next RefreshJob */
} RefreshJob , *pRefreshJob;

/* Refresher supply functions set and RefreshJob list for app */
typedef struct _Refresher {
	RefresherErrCode (*add)(struct _Refresher* const refresher, char* name,
			int8_t priority, uint8_t period, uint8_t times, bool_t newThread,
			uint32_t satckSize, void (*refreshProcess)(void *arg));
	RefresherErrCode (*del)(struct _Refresher* const refresher,
			const char* name);
	RefresherErrCode (*setPeriodAndPriority)(struct _Refresher* const refresher,
			char* name, uint8_t period, int8_t priority);
	uint32_t tick;                      /**< the Refresher running tick time. unit:Millisecond */
	rt_thread_t kernel;                 /**< the Refresher kernel thread ID,running all nonblock job */
	pRefreshJob queueHead;              /**< the refresh job queue */
	rt_mutex_t queueLock;               /**< the job queue mutex lock */
} Refresher, *pRefresher;

RefresherErrCode initRefresher(pRefresher const refresher, uint32_t stackSize,
		uint8_t priority, uint32_t tick);

#endif /* REFRESHER_H_ */
