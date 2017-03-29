/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: test the library
 * Created on: 2013-10-30
 */

#define LOG_TAG    "app"

#include <elog.h>
#include "cache.h"
#include "pthread_pool.h"
#include <stdio.h>

//#if defined(WIN32) || defined(WIN64)
//#include <windows.h>
//#define sleep(n) Sleep(1000 * (n))
//#else
//#include <unistd.h>
//#endif

void *valueChangedListener1(void *arg) {
    pCacheData data = (pCacheData)arg;
    log_d("this is valueChangedListener1,the data %s was changed", data->name);
//    sleep(1);
    return NULL;
}

void *valueChangedListener2(void *arg) {
    pCacheData data = (pCacheData)arg;
    log_d("this is valueChangedListener2,the data %s was changed", data->name);
//    sleep(1);
    return NULL;
}

void testCache(void){
    Cache cache;
    uint16_t cacheLength,valueTemp[CACHE_LENGTH_MAX];
    uint32_t cacheSize;
    initCache(&cache, "cache", 4, 512);
    valueTemp[0] = 0;
    valueTemp[1] = 1;
    valueTemp[2] = 2;
    valueTemp[3] = 3;
    cache.add(&cache,"温度",1,valueTemp,valueChangedListener1);
    cache.add(&cache,"压力",2,valueTemp,valueChangedListener2);
    cache.add(&cache,"湿度",3,valueTemp,NULL);
    cache.add(&cache,"PM2.5",4,valueTemp,NULL);
    cache.getSize(&cache,&cacheLength,&cacheSize);
    cache.get(&cache,"温度",valueTemp);
    cache.get(&cache,"压力",valueTemp);
    cache.get(&cache,"湿度",valueTemp);
    cache.get(&cache,"PM2.5",valueTemp);
    cache.del(&cache,"温度");
    cache.del(&cache,"压力");
    cache.del(&cache,"湿度");
    cache.del(&cache,"PM2.5");
    cache.del(&cache,"PM2.5");
    cache.get(&cache,"PM2.5",valueTemp);
    cache.add(&cache,"PM2.5",4,valueTemp,valueChangedListener1);
    cache.get(&cache,"PM2.5",valueTemp);
    cache.getSize(&cache,&cacheLength,&cacheSize);
    valueTemp[0] = 3;
    valueTemp[1] = 2;
    valueTemp[2] = 1;
    valueTemp[3] = 0;
    cache.set(&cache,"温度",valueTemp);
    cache.set(&cache,"压力",valueTemp);
    cache.set(&cache,"PM2.5",valueTemp);
//    cache.remove(&cache,"PM2.5");
    cache.get(&cache,"PM2.5",valueTemp);
    cache.getSize(&cache,&cacheLength,&cacheSize);

//    sleep(5);
    cache.pool->destroy(cache.pool);
}

void *testProcess(void *arg) {
    log_d("this thread arg is %d", *(uint8_t *) arg);
//    sleep(5);
    return NULL;
}

int main()
{
    /* close printf buffer */
    setbuf(stdout, NULL);
    /* EasyLogger library initialize */
    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
    elog_set_filter_lvl(ELOG_LVL_VERBOSE);
//    elog_set_text_color_enabled(true);
    /* start EasyLogger */
    elog_start();
//    while (1) {
    testCache();
//        sleep(1);
//    }
    return 0;
}
