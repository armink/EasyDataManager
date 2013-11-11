/*
 * main.c
 *
 *  Created on: 2013-10-30
 *      Author: Armink
 */
#include "cache.h"
#include "pthread_pool.h"
#include "log.h"
#include <stdio.h>

void *valueChangedListener1(void *arg) {
	pCacheData data = (pCacheData)arg;
	Log.d("this is valueChangedListener1,the data %s was changed", data->name);
	sleep(5);
	return NULL;
}

void *valueChangedListener2(void *arg) {
	pCacheData data = (pCacheData)arg;
	Log.d("this is valueChangedListener2,the data %s was changed", data->name);
	sleep(5);
	return NULL;
}

void testCache(void){
	Cache cache;
	uint16_t cacheLength,valueTemp[CACHE_LENGTH_MAX];
	uint32_t cacheSize;
	initCache(&cache,"cache",4);
	valueTemp[0] = 0;
	valueTemp[1] = 1;
	valueTemp[2] = 2;
	valueTemp[3] = 3;
	cache.add(&cache,"温度",1,1,valueTemp,valueChangedListener1);
	cache.add(&cache,"压力",2,2,valueTemp,valueChangedListener2);
	cache.add(&cache,"湿度",3,3,valueTemp,NULL);
	cache.add(&cache,"PM2.5",4,4,valueTemp,NULL);
	cache.getSize(&cache,&cacheLength,&cacheSize);
	cache.get(&cache,"温度",valueTemp);
	cache.get(&cache,"压力",valueTemp);
	cache.get(&cache,"湿度",valueTemp);
	cache.get(&cache,"PM2.5",valueTemp);
	cache.remove(&cache,"温度");
//	cache.remove(&cache,"压力");
//	cache.remove(&cache,"湿度");
//	cache.remove(&cache,"PM2.5");
//	cache.remove(&cache,"PM2.5");
	cache.get(&cache,"PM2.5",valueTemp);
//	cache.add(&cache,"PM2.5",4,4,valueTemp,valueChangedListener);
	cache.get(&cache,"PM2.5",valueTemp);
	cache.getSize(&cache,&cacheLength,&cacheSize);
	valueTemp[0] = 3;
	valueTemp[1] = 2;
	valueTemp[2] = 1;
	valueTemp[3] = 0;
	cache.put(&cache,"温度",valueTemp);
	cache.put(&cache,"压力",valueTemp);
	cache.get(&cache,"PM2.5",valueTemp);
	cache.remove(&cache,"PM2.5");
	cache.get(&cache,"PM2.5",valueTemp);
	cache.getSize(&cache,&cacheLength,&cacheSize);

	sleep(10);
	cache.pool->destroy(cache.pool);
}

void *testProcess(void *arg) {
	Log.d("this thread arg is %d", *(uint8_t *) arg);
	sleep(5);
	return NULL;
}

void testThreadPoll(void){
	uint8_t i , dataTemp[10];
	ThreadPool pool;
	initThreadPool(&pool, 4);
	for (i = 0; i < 10; i++) {
		dataTemp[i] = i;
		pool.addTask(&pool,testProcess,&dataTemp[i]);
	}
	sleep(20);
	pool.destroy(&pool);
}
int main()
{
	/* 关闭printf缓冲输出 */
	setbuf(stdout, NULL);
	initLogger(TRUE);
	testCache();
//	testThreadPoll();
	destroyLogger();
	return 0;
}
