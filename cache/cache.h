/*
 * cahce.h
 *
 *  Created on: 2013-10-31
 *      Author: Armink
 *  Note:Cache > CacheData=Data > Value in logic
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef signed   char                   int8_t;      /**<  8bit integer type */
typedef signed   short                  int16_t;     /**< 16bit integer type */
typedef signed   long                   int32_t;     /**< 32bit integer type */
typedef unsigned char                   uint8_t;     /**<  8bit unsigned integer type */
typedef unsigned short                  uint16_t;    /**< 16bit unsigned integer type */
typedef unsigned long                   uint32_t;    /**< 32bit unsigned integer type */
typedef int                             bool_t;      /**< boolean type */

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#define CACHE_NAME_MAX     20       /**< CacheData max name length */
#define CACHE_LENGTH_MAX   64       /**< value max length */
#define CACHE_LEVEL_MAX    10       /**< value max level */

/* value error code */
typedef enum{
	CACHE_NO_ERR,                  /**< no error */
	CACHE_NAME_ERROR,              /**< CacheData name has error */
	CACHE_INDEX_ERROR,             /**< value index has error */
	CACHE_LENGTH_ERROR,            /**< value length has error */
	CACHE_LEVEL_ERROR,             /**< value level has error */
	CACHE_NO_VALUE,                /**< value not find */
	CACHE_ILL_ARG,                 /**< illegal argument */
	CACHE_NOT_INIT,                /**< cache not initialize */
	CACHE_NO_SPACE,                /**< not enough space */
}CacheErrCode;

/* CacheData is the dynamic cache data's property. */
typedef struct _CacheData{
	char  name[CACHE_NAME_MAX];         /**< the name of CacheData */
	uint8_t length;                     /**< value length */
	uint8_t level;                      /**< refresh level.If level is 0,value will not refresh */
	uint16_t* value;                    /**< the value pointer*/
	void (*valueChangedListener)(void); /**< It will call when the CacheData's value has changed */
	struct _CacheData* next;            /**< point to next CacheData */
}CacheData, *pCacheData;

/* Cache supply functions set for app */
typedef struct {
	pCacheData headData ;
	pCacheData tailData ;
} Cache, *pCache;

CacheErrCode initCache(pCache const cache);
pCacheData findData(pCache const cache, const char* name);
CacheErrCode addData(pCache const cache, char* name, uint8_t length,
		uint8_t level, uint16_t* value, void (*valueChangedListener)(void));
CacheErrCode removeData(pCache const cache, const char* name);
CacheErrCode getValue(pCache const cache, const char* name, uint16_t* value);
CacheErrCode putValue(pCache const cache, const char* name, uint16_t* value);

#endif /* DM_H_ */
