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

/* refresher error code */
typedef enum{
	REFRESHER_NO_ERR,                 /**< no error */
}RefresherErrCode;

/* RefreshJob is an auto refresh job for a Cache data. */
typedef struct _RefreshJob{
	char  name[CACHE_NAME_MAX];         /**< the name of the CacheData is refreshed.@see CacheData */
	int8_t level;                       /**< refresh level.If level is -1,RefreshJob will stop work */
	uint8_t period;                     /**< refresh time = period * refresher tick. @see Refresher */
	void* (*refreshProcess)(void *arg); /**< it will call when the RefreshJob need wrok */
	struct _RefreshJob* next;           /**< point to next CacheData */
} RefreshJob , *pRefreshJob;

/* Refresher supply functions set and RefreshJob list for app */
typedef struct _Refresher {
	RefresherErrCode (*start)(struct _Refresher* const refresher);
	RefresherErrCode (*stop)(struct _Refresher* const refresher);
	RefresherErrCode (*add)(struct _Refresher* const refresher, char* name,
			int8_t level, uint8_t period, void* (*refreshProcess)(void *arg));
	RefresherErrCode (*del)(struct _Refresher* const refresher,
			const char* name);
	RefresherErrCode (*setLevel)(struct _Refresher* const refresher, char* name,
			int8_t level);
	RefresherErrCode (*setPeriod)(struct _Refresher* const refresher,
			char* name, uint8_t period);
	RefresherErrCode (*setProcess)(struct _Refresher* const refresher,
			char* name, void* (*refreshProcess)(void *arg));
	RefresherErrCode (*control)(struct _Refresher* const refresher, char* name,
			int8_t level, uint8_t period, void* (*refreshProcess)(void *arg));
	uint32_t tickTime;                  /**< the Refresher work tick time. unit:Millisecond */
	char name[CACHE_NAME_MAX];          /**< the name of Refresher */
	pRefreshJob queueHead;              /**< the refresh job queue */
} Refresher, *pRefresher;


#endif /* REFRESHER_H_ */
