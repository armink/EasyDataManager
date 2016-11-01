/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: process and manage the cache data
 * Created on: 2013-10-31
 * Note: Cache > CacheData=Data > Value     in logic
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <edm_def.h>

#if defined(EDM_USING_PTHREAD)
#include "pthread_pool.h"
#elif defined(EDM_USING_RTT)
#include "rtthread_pool.h"
#endif

#define CACHE_NAME_MAX     32       /**< CacheData max name length */
#define CACHE_LENGTH_MAX   64       /**< value max length */

/* Cache error code */
typedef enum{
    CACHE_NO_ERR,                  /**< no error */
    CACHE_NAME_ERROR,              /**< CacheData name has error */
    CACHE_INDEX_ERROR,             /**< value index has error */
    CACHE_LENGTH_ERROR,            /**< value length has error */
    CACHE_NO_VALUE,                /**< value not find */
    CACHE_ILL_ARG,                 /**< illegal argument */
    CACHE_NOT_INIT,                /**< cache not initialize */
    CACHE_NO_SPACE,                /**< not enough space */
}CacheErrCode;

/* CacheData is the dynamic cache data's property. */
typedef struct _CacheData{
    char  name[CACHE_NAME_MAX+1];       /**< the name of CacheData, the end of name is include '\0' */
    uint8_t length;                     /**< value length */
    uint16_t* value;                    /**< the value pointer*/
    void* (*valueChangedListener)(void *arg); /**< it will call when the CacheData's value has changed */
    struct _CacheData* next;            /**< point to next CacheData */
}CacheData, *pCacheData;

/* Cache supply functions set and CacheData list for app */
typedef struct _Cache {
    /**
     * This function will find the data in CacheData list.
     *
     * @param cache the cache pointer
     * @param name the name of CacheData
     *
     * @return the CacheData point which has found,If not found will return NULL.
     */
    pCacheData (*has)(struct _Cache* const cache, const char* name);
    /**
     * This function will add CacheData to list.
     *
     * @param cache the cache pointer
     * @param name the name of CacheData
     * @param length the value length
     * @param value the value point
     * @param valueChangedListener the changed value's callback function
     *
     * @return error code
     */
    CacheErrCode (*add)(struct _Cache* const cache, const char* name,
            uint8_t length, uint16_t* value,
            void* (*valueChangedListener)(void *arg));
    /**
     * This function will delete the data from CacheData list.
     *
     * @param cache the cache pointer
     * @param name the cache data name
     *
     * @return error code
     */
    CacheErrCode (*del)(struct _Cache* const cache, const char* name);
    /**
     * This function will set the value in CacheData list.
     *
     * @param cache the cache pointer
     * @param name the cache data name
     * @param value the value which will set
     *
     * @return error code
     */
    CacheErrCode (*set)(struct _Cache* const cache, const char* name,
            uint16_t* value);
    /**
     * This function will get the value from CacheData list.
     *
     * @param cache the cache pointer
     * @param name the cache data name
     * @param value the value which has getted
     *
     * @return error code
     */
    CacheErrCode (*get)(struct _Cache* const cache, const char* name,
            uint16_t* value);
    /**
     * This function will set the value in CacheData list.
     *
     * @param cache the cache pointer
     * @param name the cache data name
     * @param value the value which will set
     *
     * @return error code
     */
    CacheErrCode (*getSize)(struct _Cache* const cache, uint32_t* length,
            uint32_t* size);
    char name[CACHE_NAME_MAX+1];   /**< the name of CacheData, the end of name is include '\0' */
    pCacheData dataHead;
    pCacheData dataTail;
    pThreadPool pool;
} Cache, *pCache;

CacheErrCode initCache(pCache const cache, const char* name, uint8_t maxThreadNum,
        uint32_t threadStackSize);

#endif /* DM_H_ */
