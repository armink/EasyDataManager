/*
 * cache.c
 *
 *  Created on: 2013-10-31
 *      Author: Armink
 *  Note:Cache > CacheData=Data > Value in logic
 */
#include "cache.h"

/**
 * This function will initialize cache model.
 *
 * @param cache the cache pointer
 *
 * @return error code
 */
CacheErrCode initCache(pCache const cache) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	cache->headData = NULL;
	cache->tailData = NULL;
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
	pCacheData data = cache->headData;

	assert((name != NULL) && (strlen(name) <= CACHE_NAME_MAX));
	//this cache is null
	if (cache->headData == NULL) {
		return NULL;
	}
	/* search the data from list*/
	for (;;) {
		if (!strcmp(data->name, name)) {
			return data;
		} else {
			if (data->next == NULL) {/* list tail */
				printf("could not find %s\n", name);
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
		uint8_t level, uint16_t* value, void (*valueChangedListener)(void)) {
	CacheErrCode errorCode = CACHE_NO_ERR;
	pCacheData data;

	data = (pCacheData) malloc(sizeof(CacheData));
	assert(data != NULL);

	if (findData(cache,name) != NULL) {/* the data must not exist in list */
		errorCode = CACHE_NAME_ERROR;
	} else {
		strcpy(data->name, name);
	}

	if (errorCode == CACHE_NO_ERR) {
		if (length > CACHE_LENGTH_MAX) {
			errorCode = CACHE_LENGTH_ERROR;
		} else {
			data->length = length;
		}
	}

	if (errorCode == CACHE_NO_ERR) {
		if (level > CACHE_LEVEL_MAX) {
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
		/* if list is NULL ,then head node is equal of tail node*/
		if (cache->headData == NULL) {
			cache->headData = cache->tailData = data;
		} else if ((cache->headData == cache->tailData) /* the list has one node */
		&& (cache->headData != NULL)) {
			cache->headData->next = data;
			cache->tailData = data;
		} else { /* the list has more than one node*/
			cache->tailData->next = data;
			cache->tailData = data;
		}
	} else if (errorCode != CACHE_NO_ERR) {
		free(data);
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
	pCacheData data = cache->headData, dataTemp;

	assert((name != NULL) && (strlen(name) <= CACHE_NAME_MAX));
	/* check cache initialize */
	if (cache->tailData == NULL) {
		errorCode = CACHE_NOT_INIT;
	}
	/* search the data from list*/
	if (errorCode == CACHE_NO_ERR) {
		if (strcmp(data->name, name)) { /* list head  */
			for (;;) {
				if (data->next == NULL) {/* list tail */
					printf("could not find %s\n", name);
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
		/* delete data is head node */
		if (data == cache->headData) {
			/* the list has one node */
			if ((cache->headData == cache->tailData)
					&& (cache->headData != NULL)) {
				cache->headData = cache->tailData = NULL;
			} else { /* the list has more than one node*/
				cache->headData = data->next;
			}
		} else if (data->next == cache->tailData) {/* delete data is tail node */
			cache->tailData = data;
			cache->tailData->next = NULL;
			data = data->next; /* data will be freed in the end */
		} else {
			dataTemp = data->next;
			data->next = data->next->next;
			data = dataTemp; /* data will be freed in the end */
		}
		free(dataTemp->value);
		free(dataTemp);
		printf("remove %s success\n", name);
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
			printf("get the %s value%d is %d \n", data->name, i, *(dataValue));
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
	pCacheData data = cache->headData;
	uint8_t i;
	uint16_t* dataValue;
	/* the data must exist in list */
	if ((data = findData(cache, name)) == NULL) {
		errorCode = CACHE_NAME_ERROR;
	} else { /* put the value */
		dataValue = data->value;
		for (i = 0; i < data->length; i++) {
			printf("put the %s value%d is %d \n", data->name, i, *(value));
			*(dataValue++) = *(value++);
		}
	}
	return errorCode;
}


