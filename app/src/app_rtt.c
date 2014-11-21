/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdio.h>
#include <board.h>
#include "log.h"
#include "cache.h"
#include "refresher.h"
extern int  rt_application_init(void);

extern rt_uint8_t *heap;

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
    /* init board */
    rt_hw_board_init();

    /* show version */
    rt_show_version();

    /* init tick */
    rt_system_tick_init();

    /* init kernel object */
    rt_system_object_init();

    /* init timer system */
    rt_system_timer_init();

#ifdef RT_USING_HEAP
    /* init memory system */
    rt_system_heap_init((void *)heap, (void *)&heap[RT_HEAP_SIZE - 1]);
#endif

    /* init scheduler system */
    rt_system_scheduler_init();

    /* init all device */
#ifdef RT_USING_DEVICE
    rt_device_init_all();
#endif
    /* init application */
    rt_application_init();

    /* init timer thread */
    rt_system_timer_thread_init();

    /* init idle thread */
    rt_thread_idle_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    return ;
}

void *valueChangedListener1(void *arg) {
    pCacheData data = (pCacheData)arg;
    LogD("this is valueChangedListener1,the data %s was changed", data->name);
    rt_thread_delay(1);
    return NULL;
}

void *valueChangedListener2(void *arg) {
    pCacheData data = (pCacheData)arg;
    LogD("this is valueChangedListener2,the data %s was changed", data->name);
    rt_thread_delay(1);
    return NULL;
}

void testCache(void){
    Cache cache;
    uint16_t valueTemp[CACHE_LENGTH_MAX];
    uint32_t cacheLength, cacheSize;
    initCache(&cache, "cache", 4, 512);
    valueTemp[0] = 0;
    valueTemp[1] = 1;
    valueTemp[2] = 2;
    valueTemp[3] = 3;
    //test add data
    cache.add(&cache,"Temp",1,valueTemp,valueChangedListener1);
    cache.add(&cache,"Pressure",2,valueTemp,valueChangedListener2);
    cache.add(&cache,"Humidity",3,valueTemp,NULL);
    cache.add(&cache,"PM2.5",4,valueTemp,NULL);
    //test get data
    cache.getSize(&cache,&cacheLength,&cacheSize);
    cache.get(&cache,"Temp",valueTemp);
    cache.get(&cache,"Pressure",valueTemp);
    cache.get(&cache,"Humidity",valueTemp);
    cache.get(&cache,"PM2.5",valueTemp);
    //test delete data
    cache.del(&cache,"Temp");
    cache.del(&cache,"Pressure");
    cache.del(&cache,"Humidity");
    cache.del(&cache,"PM2.5");
    cache.del(&cache,"PM2.5");
    cache.get(&cache,"PM2.5",valueTemp);
    //test valueChangedListener
    cache.add(&cache,"PM2.5",4,valueTemp,valueChangedListener1);
    cache.add(&cache,"Pressure",2,valueTemp,valueChangedListener2);
    valueTemp[0] = 3;
    valueTemp[1] = 2;
    valueTemp[2] = 1;
    valueTemp[3] = 0;
    cache.set(&cache,"Pressure",valueTemp);
    cache.set(&cache,"PM2.5",valueTemp);
    cache.get(&cache,"PM2.5",valueTemp);
    cache.getSize(&cache,&cacheLength,&cacheSize);
    //test destroy cache
    rt_thread_delay(1000);
    cache.pool->destroy(cache.pool);
}

void testCachePerformance(uint32_t dataTotalNum){
    long i, lastTime;
    uint16_t valueTemp[CACHE_LENGTH_MAX];
    Cache cache;
    char dataName[] = "0000000000000000000\0";
    valueTemp[0] = 0;
    valueTemp[1] = 1;
    valueTemp[2] = 2;
    valueTemp[3] = 3;
    printf("Start test cache performance using %ld data, please wait...\n", dataTotalNum);
    initCache(&cache, "cache", 4, 512);
    lastTime = rt_tick_get();
    for (i = 0; i < dataTotalNum; i++) {
        ltoa(i, dataName, 10);
        cache.add(&cache, dataName, 1, valueTemp, NULL);
    }
    printf("1.Test cache add data finish.This system can add %ld data to cache in 1s.\n",dataTotalNum*1000/(rt_tick_get() - lastTime));
    lastTime = rt_tick_get();
    for(i = 0 ;i < dataTotalNum;i++){
        ltoa(i,dataName,10);
        cache.set(&cache,dataName,valueTemp);
    }
    printf("2.Test cache set data finish.This system can set %ld data value in 1s.\n",dataTotalNum*1000/(rt_tick_get() - lastTime));
    lastTime = rt_tick_get();
    for(i = 0 ;i < dataTotalNum;i++){
        ltoa(i,dataName,10);
        cache.get(&cache,dataName,valueTemp);
    }
    printf("3.Test cache get data finish.This system can get %ld data value in 1s.\n",dataTotalNum*1000/(rt_tick_get() - lastTime));
    lastTime = rt_tick_get();
    for(i = 0 ;i < dataTotalNum;i++){
        ltoa(i,dataName,10);
        cache.del(&cache,dataName);
    }
    printf("4.Test cache delete data finish.This system can delete %ld data value in 1s.\n",dataTotalNum*1000/(rt_tick_get() - lastTime));

}

void tempRefreshProcess(void *arg){
    LogD("Temp refresh is process");
    rt_thread_delay(13);
}

void pressureRefreshProcess(void *arg){
    LogD("Pressure refresh is process");
    rt_thread_delay(20);
}

void testRefresher(){
    Refresher refresher;
    initRefresher(&refresher,512,5,50);
    refresher.add(&refresher,"Temp",8,2,-1,FALSE,512,tempRefreshProcess);
    refresher.add(&refresher,"Pressure",10,4,-1,FALSE,512,pressureRefreshProcess);
//    refresher.add(&refresher,"Temp",8,2,4,TRUE,512,tempRefreshProcess);
//    refresher.add(&refresher,"Pressure",10,4,4,TRUE,512,pressureRefreshProcess);
    rt_thread_delay(2000);
    refresher.setPeriodAndPriority(&refresher,"Pressure",1,1);
    LogD("change period and priority ");
    rt_thread_delay(2000);
    refresher.setTimes(&refresher,"Pressure",1);
    LogD("setTimes");
    rt_thread_delay(2000);
    LogD("will delete job");
    refresher.del(&refresher,"Temp");
    LogD("delete job1");
    refresher.del(&refresher,"Temp");
    LogD("delete job2");
    refresher.del(&refresher,"Pressure");
    LogD("delete job3");
    rt_thread_delay(5000);
}

void thread_entry_SysMonitor(void* parameter) {
    uint8_t i = 0;

    initLogger(TRUE);

    testCache();

    testRefresher();

//    destroyLogger();
//    testCachePerformance(20000);
    for (i = 0; i < 3; i++) {
        LogD("hello, world2");
        rt_thread_delay(1000);
    }
    exit(0);
}

int rt_application_init() {

    static struct rt_thread thread_SysMonitor;
    ALIGN(RT_ALIGN_SIZE)
    static rt_uint8_t thread_SysMonitor_stack[512];

//    rt_thread_init(&thread_SysMonitor,
//                   "SysMonitor1",
//                   thread_entry_SysMonitor,
//                   RT_NULL,
//                   thread_SysMonitor_stack,
//                   sizeof(thread_SysMonitor_stack),
//                   4,20);
//    rt_thread_startup(&thread_SysMonitor);

    rt_thread_t tid;
    tid = rt_thread_create("SysMonitor2", thread_entry_SysMonitor, RT_NULL, 2048,
            4, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

int main(void)
{
    /* close printf buffer */
    setbuf(stdout, NULL);

    /* disable interrupt first */
    rt_hw_interrupt_disable();

    /* startup RT-Thread RTOS */
    rtthread_startup();

    return 0;
}


/*@}*/
