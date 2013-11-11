/*
 *  cache.c
 *  process and manage the cache data
 *  Created on: 2013-10-31
 *      Author: Armink
 *  Note:Cache > CacheData=Data > Value     in logic
 */
#include "cache.h"

static pCacheData findData(pCache const cache, const char* name);
static CacheErrCode addData(pCache const cache, char* name, uint8_t length,
		uint8_t level, uint16_t* value,
		void* (*valueChangedListener)(void *arg));
static CacheErrCode removeData(pCache const cache, const char* name);
static CacheErrCode getValue(pCache const cache, const char* name,
		uint16_t* value);
static CacheErrCode putValue(pCache const cache, const char* name,
		uint16_t* value);
static CacheErrCode getSize(pCache const cache, uint16_t* length,
		uint32_t* size);

/**
 * This function will initialize cache model.
 *
 * @param cache the cache pointer
 * @param name the cache name
 * @param maxThreadNum the cache use max threads number in thread pool
 *
 * @return error code
 */
CacheErrCode initCache(pCache const cache, const char* name,
		uint8_t maxThreadNum) {
	CacheErrCode errorCode = CACHE_NO_ERR;

	if ((name == NULL) || (strlen(name) > CACHE_NAME_MAX)) {
		Log.d("the name of %s can not be create for list", name);
		errorCode = CACHE_NAME_ERROR;
	} else {
		strcpy(cache->name, name);
	}
	cache->find = findData;
	cache->add = addData;
	cache->remove = removeData;
	cache->get = getValue;
	cache->put = putValue;
	cache->getSize = getSize;
	cache->dataHead = NULL;
	cache->dataTail = NULL;
	/* initialize the thread pool */
	cache->pool = (pThreadPool)malloc(sizeof(ThreadPool));
	initThreadPool(cache->pool,maxThreadNum);
	return errorCode;
}

/**
 * This function will find the data in CacheData list.
 *
 * @param cache the cache pointer
 * @param name the name of CacheData
 *
 * @return the CacheData point which has found,If not found will return NULL.
 */
pCacheData findData(pCache const cache, const char* name) {
	pCacheData data = cache->dataHead;

	assert((name != NULL) && (strlen(name) <= CACHE_NAME_MAX));
	//this cache is null
	if (cache->dataHead == NULL) {
		Log.d("the %s's data list is NULL,find data fail",cache->name);
		return NULL;
	}
	/* search the data from list*/
	for (;;) {
		if (!strcmp(data->name, name)) {
			return data;
		} else {
			if (data->next == NULL) {/* list tail */
				Log.d("could not find %s", name);
				break;
			}
			data = data->next;
		}
	}
	return NULL;
}

/**
 * This function will add CacheData to list.
 *
 * @param cache the cache pointer
 * @param name the name of CacheData
 * @param length the value length
 * @param level the refresh level.If level is 0,value will not refresh
 * @param value the value point
 * @param valueChangedListener the changed value's callback function
 *
 * @return error code
 */
CacheErrCode addData(pCache const cache, char* name, uint8_t length,
		uint8_t level, uint16_t* value,
		void* (*valueChangedListener)(void *arg)) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	pCacheData data;

	data = (pCacheData) malloc(sizeof(CacheData));
	assert(data != NULL);

	if (findData(cache,name) != NULL) {/* the data must not exist in list */
		Log.d("the name of %s data is already exist in cache data list", name);
		errorCode = CACHE_NAME_ERROR;
	} else {
		strcpy(data->name, name);
	}

	if (errorCode == CACHE_NO_ERR) {
		if (length > CACHE_LENGTH_MAX) {
			Log.d("the name %s is too long,can't add to list", name);
			errorCode = CACHE_LENGTH_ERROR;
		} else {
			data->length = length;
		}
	}

	if (errorCode == CACHE_NO_ERR) {
		if (level > CACHE_LEVEL_MAX) {
			Log.d("the level %d is greater than max level", level);
			errorCode = CACHE_LEVEL_ERROR;
		} else {
			data->level = level;
		}
	}

	if (errorCode == CACHE_NO_ERR) {
		/* malloc data value space */
		data->value = (uint16_t*) malloc(length * sizeof(uint16_t));
		assert(data->value != NULL);
		memcpy(data->value, value, length * sizeof(uint16_t));
		data->valueChangedListener = valueChangedListener;
		data->next = NULL;
		/* lock the thread pool synchronized lock */
		cache->pool->lock(cache->pool);
		/* if list is NULL ,then head node is equal of tail node*/
		if (cache->dataHead == NULL) {
			cache->dataHead = cache->dataTail = data;
		} else if ((cache->dataHead == cache->dataTail) /* the list has one node */
		&& (cache->dataHead != NULL)) {
			cache->dataHead->next = data;
			cache->dataTail = data;
		} else { /* the list has more than one node*/
			cache->dataTail->next = data;
			cache->dataTail = data;
		}
		/* unlock the thread pool synchronized lock */
		cache->pool->unlock(cache->pool);
		Log.d("add %s to data list is success", name);
	} else if (errorCode != CACHE_NO_ERR) {
		free(data);
		data = NULL;
	}
	return errorCode;
}

/**
 * This function will remove the data from CacheData list.
 *
 * @param cache the cache pointer
 * @param name the cache data name
 *
 * @return error code
 */
CacheErrCode removeData(pCache const cache, const char* name) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	pCacheData data = cache->dataHead, dataTemp;

	assert((name != NULL) && (strlen(name) <= CACHE_NAME_MAX));
	/* check cache initialize */
	if (cache->dataHead == NULL) {
		Log.d("the %s's data list is NULL,remove data fail",cache->name);
		errorCode = CACHE_NO_VALUE;
	}
	/* search the data from list*/
	if (errorCode == CACHE_NO_ERR) {
		if (strcmp(data->name, name)) { /* list head  */
			for (;;) {
				if (data->next == NULL) {/* list tail */
					Log.d("could not find %s", name);
					errorCode = CACHE_NAME_ERROR;
					break;
				} else {
					if (!strcmp(data->next->name, name)) {
						break;
					} else {
						data = data->next;
					}
				}
			}
		}
	}
	if (errorCode == CACHE_NO_ERR) {
		/* lock the thread pool synchronized lock */
		cache->pool->lock(cache->pool);
		/* delete data is head node */
		if (data == cache->dataHead) {
			/* the list has one node */
			if ((cache->dataHead == cache->dataTail)
					&& (cache->dataHead != NULL)) {
				cache->dataHead = cache->dataTail = NULL;
			} else { /* the list has more than one node*/
				cache->dataHead = data->next;
			}
		} else if (data->next == cache->dataTail) {/* delete data is tail node */
			cache->dataTail = data;
			cache->dataTail->next = NULL;
			data = data->next; /* data will be freed in the end */
		} else {
			dataTemp = data->next;
			data->next = data->next->next;
			data = dataTemp; /* data will be freed in the end */
		}
		free(dataTemp->value);
		dataTemp->value = NULL;
		free(dataTemp);
		dataTemp = NULL;
		/* unlock the thread pool synchronized lock */
		cache->pool->unlock(cache->pool);
		Log.d("remove %s data node is success", name);
	}
	return errorCode;
}

/**
 * This function will get the value from CacheData list.
 *
 * @param cache the cache pointer
 * @param name the cache data name
 * @param value the value which has getted
 *
 * @return error code
 */
CacheErrCode getValue(pCache const cache, const char* name, uint16_t* value) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	pCacheData data;
	uint8_t i;
	uint16_t* dataValue;
	/* the data must exist in list */
	if ((data = findData(cache, name)) == NULL) {
		errorCode = CACHE_NAME_ERROR;
	}
	/* return the value */
	if (errorCode == CACHE_NO_ERR) {
		dataValue = data->value;
		for (i = 0; i < data->length; i++) {
			Log.d("get %s value%d is %d ", data->name, i, *(dataValue));
			*(value++) = *(dataValue++);
		}
	}

	return errorCode;
}

/**
 * This function will put the value to CacheData list.
 *
 * @param cache the cache pointer
 * @param name the cache data name
 * @param value the value which will put
 *
 * @return error code
 */
CacheErrCode putValue(pCache const cache, const char* name, uint16_t* value) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	pCacheData data = cache->dataHead;
	uint8_t i, isValueChange = FALSE;
	uint16_t* dataValue;
	/* the data must exist in list */
	if ((data = findData(cache, name)) == NULL) {
		errorCode = CACHE_NAME_ERROR;
	} else { /* put the value */
		dataValue = data->value;
		for (i = 0; i < data->length; i++) {
			Log.d("put %s value%d is %d", data->name, i, *(value));
			if (*(dataValue) != *(value)) {
				isValueChange = TRUE;
			}
			*(dataValue++) = *(value++);
		}
	}
	/* If value was changed.Execution valueChangedListener */
	if ((isValueChange) && (data->valueChangedListener != NULL)) {
		cache->pool->addTask(cache->pool, data->valueChangedListener, data);
	}
	return errorCode;
}

/**
 * This function will get the CacheData list total length.
 *
 * @param cache the cache pointer
 * @param length the total length of list
 *
 * @return error code
 */
CacheErrCode getSize(pCache const cache, uint16_t* length, uint32_t* size) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	pCacheData data = cache->dataHead;
	*length = 0;
	*size = 0;
	for (;;) {
		if (data == NULL) {
			Log.d("the %s's length is %d,size is %ld", cache->name, *length,
					*size);
			break;
		} else {
			(*length)++;
			(*size) += data->length * sizeof(uint16_t);
			data = data->next;
		}
	}
	if ((*length) == 0)
		errorCode = CACHE_NOT_INIT;
	return errorCode;
}
