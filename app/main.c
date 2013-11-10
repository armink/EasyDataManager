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

void testCache(void){
	Cache cache;
	uint16_t cacheLength,valueTemp[CACHE_LENGTH_MAX];
	uint32_t cacheSize;
	initCache(&cache,"cache");
	valueTemp[0] = 0;
	valueTemp[1] = 1;
	valueTemp[2] = 2;
	valueTemp[3] = 3;
	cache.add(&cache,"温度",1,1,valueTemp,NULL);
	cache.add(&cache,"压力",2,2,valueTemp,NULL);
	cache.add(&cache,"湿度",3,3,valueTemp,NULL);
	cache.add(&cache,"PM2.5",4,4,valueTemp,NULL);
	cache.getSize(&cache,&cacheLength,&cacheSize);
	cache.get(&cache,"温度",valueTemp);
	cache.get(&cache,"压力",valueTemp);
	cache.get(&cache,"湿度",valueTemp);
	cache.get(&cache,"PM2.5",valueTemp);
	cache.remove(&cache,"温度");
	cache.remove(&cache,"压力");
	cache.remove(&cache,"湿度");
	cache.remove(&cache,"PM2.5");
	cache.remove(&cache,"PM2.5");
	cache.get(&cache,"PM2.5",valueTemp);
	cache.add(&cache,"PM2.5",4,4,valueTemp,NULL);
	cache.get(&cache,"PM2.5",valueTemp);
	cache.getSize(&cache,&cacheLength,&cacheSize);
	valueTemp[0] = 3;
	valueTemp[1] = 2;
	valueTemp[2] = 1;
	valueTemp[3] = 0;
	cache.put(&cache,"PM2.5",valueTemp);
	cache.get(&cache,"PM2.5",valueTemp);
	cache.remove(&cache,"PM2.5");
	cache.get(&cache,"PM2.5",valueTemp);
	cache.getSize(&cache,&cacheLength,&cacheSize);
//	cache.put(&cache,"PM2.5",valueTemp);
//	cache.get(&cache,"温度",valueTemp);
//	cache.get(&cache,"压力",valueTemp);
//	cache.get(&cache,"湿度",valueTemp);
//	cache.get(&cache,"PM2.5",valueTemp);
//
//	cache.remove(&cache,"PM2.5");
//	cache.get(&cache,"PM2.5",valueTemp);
//	cache.remove(&cache,"温度");
//	cache.findData(&cache,"温度");
//	cache.add(&cache,"温度",1,1,valueTemp,NULL);
//	cache.get(&cache,"温度",valueTemp);

}

void *testProcess(void *arg) {
	Log.debug("this thread arg is %d\n", *(uint8_t *) arg);
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
	initLogger();

	testThreadPoll();
	return 0;
}
