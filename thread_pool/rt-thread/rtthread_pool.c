/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a thread pool base as RT-Thread
 * Created on: 2013-11-14
 */
#include "rtthread_pool.h"

#ifdef EDM_USING_RTT

static ThreadPoolErrCode addTask(pThreadPool const pool,
        void *(*process)(void *arg), void *arg);
static ThreadPoolErrCode destroy(pThreadPool pool);
static void threadJob(void* arg);
static void syncLock(pThreadPool pool);
static void syncUnlock(pThreadPool pool);

/**
 * This function will initialize the thread pool.
 *
 * @param pool the ThreadPool pointer
 * @param name the ThreadPool name
 * @param maxThreadNum the max thread number in this ThreadPool
 * @param threadStackSize the thread stack size in this ThreadPool
 *
 * @return error code
 */
ThreadPoolErrCode initThreadPool(pThreadPool const pool, const char* name, uint8_t maxThreadNum,
        uint32_t threadStackSize) {
    ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
    char jobName[THREAD_POOL_NAME_MAX] = {0};
    uint8_t i ;

    RT_ASSERT(name);
    RT_ASSERT(strlen(name) <= THREAD_POOL_NAME_MAX);
    RT_ASSERT(maxThreadNum <= THREAD_POOL_MAX_THREAD_NUM);

    strcpy(pool->name, name);
    strcpy(jobName, name);
    strcpy(&jobName[strlen(jobName)],"_job");

    pool->queueLock = rt_mutex_create("queueLock", RT_IPC_FLAG_FIFO);
    RT_ASSERT(pool->queueLock != NULL);
    pool->userLock = rt_mutex_create("userLock", RT_IPC_FLAG_FIFO);
    RT_ASSERT(pool->userLock != NULL);
    pool->queueReady = rt_sem_create("queueReady", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(pool->queueReady != NULL);
    pool->queueHead = NULL;
    pool->maxThreadNum = maxThreadNum;
    pool->curWaitThreadNum = 0;
    pool->isShutdown = FALSE;
    pool->addTask = addTask;
    pool->destroy = destroy;
    pool->lock = syncLock;
    pool->unlock = syncUnlock;
    pool->threadID = (rt_thread_t*) malloc(maxThreadNum * sizeof(rt_thread_t));
    RT_ASSERT(pool->threadID != NULL);
    for (i = 0; i < maxThreadNum; i++) {
        jobName[strlen(jobName)] = '0' + i;
        pool->threadID[i] = rt_thread_create(jobName, threadJob, pool, threadStackSize,
        THREAD_POOL_JOB_DEFAULT_PRIORITY, THREAD_POOL_JOB_TICK * i);
        RT_ASSERT(pool->threadID[i] != NULL);
        rt_thread_startup(pool->threadID[i]);
        jobName[strlen(jobName) - 1] = '\0';
        LogD("create thread success.Current total thread number is %d", i + 1);
        rt_thread_delay(THREAD_POOL_THREADS_INIT_TIME);
    }
    LogD("initialize thread pool success!");

    return errorCode;
}

/**
 * This function will add a task to thread pool.
 *
 * @param pool the ThreadPool pointer
 * @param process task function pointer
 * @param arg task function arguments
 *
 * @return error code
 */
static ThreadPoolErrCode addTask(pThreadPool const pool,
        void *(*process)(void *arg), void *arg) {
    ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
    pTask member = NULL;
    pTask newtask = (pTask) malloc(sizeof(Task));
    RT_ASSERT(newtask != NULL);
    newtask->process = process;
    newtask->arg = arg;
    newtask->next = NULL;
    /* lock thread pool */
    rt_mutex_take(pool->queueLock, RT_WAITING_FOREVER);
    member = pool->queueHead;
    /* task queue is NULL */
    if (member == NULL) {
        pool->queueHead = newtask;
    } else {
        /* look up for queue tail */
        while (member->next != NULL) {
            member = member->next;
        }
        member->next = newtask;
    }
    /* add current waiting thread number */
    pool->curWaitThreadNum++;
    rt_mutex_release(pool->queueLock);
    /* wake up a waiting thread to process task */
    rt_sem_release(pool->queueReady);
    LogD("add a task to task queue success.");
    return errorCode;
}

/**
 * This function will destroy thread pool.
 *
 * @param pool the ThreadPool pointer
 *
 * @return error code
 */
static ThreadPoolErrCode destroy(pThreadPool pool) {
    ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
    pTask head = NULL;
    uint8_t i;
    if (pool->isShutdown) {/* thread already shutdown */
        errorCode = THREAD_POOL_ALREADY_SHUTDOWN_ERR;
    }
    if (errorCode == THREAD_POOL_NO_ERR) {
        pool->isShutdown = TRUE;
        /* wake up all thread from broadcast */
        /* delete mutex and semaphore then all waiting thread will wake up */
        rt_mutex_delete(pool->queueLock);
        rt_sem_delete(pool->queueReady);
        /* wait all thread exit */
        for (i = 0; i < pool->maxThreadNum; i++) {
            rt_thread_delete(pool->threadID[i]);
        }
        /* release memory */
        free(pool->threadID);
        pool->threadID = NULL;
        /* destroy task queue */
        while (pool->queueHead != NULL) {
            head = pool->queueHead;
            pool->queueHead = pool->queueHead->next;
            free(head);
        }
        /* destroy mutex */
        rt_mutex_delete(pool->userLock);
        /* release memory */
        free(pool);
        pool = NULL;
        LogD("Thread pool destroy success");
    }
    return errorCode;
}

/**
 * This function is thread job.
 *
 * @param arg the thread job arguments
 *
 */
static void threadJob(void* arg) {
    pThreadPool pool = NULL;
    pTask task = NULL;
    LogD("threadJob is running");
    while (1) {
        pool = (pThreadPool) arg;
        /* lock thread pool */
        rt_mutex_take(pool->queueLock, RT_WAITING_FOREVER);
        /* If waiting thread number is 0 ,and thread is not shutdown.
         * The thread will block.
         * Before thread block the queueLock will unlock.
         * After thread wake up ,the queueLock will relock.*/
        while (pool->curWaitThreadNum == 0 && !pool->isShutdown) {
            LogD("the thread waiting for task add to task queue");
            /* ququeReady is NULL,the thread will block */
            if(pool->queueReady->value == 0){
                rt_mutex_release(pool->queueLock);
                rt_sem_take(pool->queueReady, RT_WAITING_FOREVER);
                rt_mutex_take(pool->queueLock, RT_WAITING_FOREVER);
            }else{/* ququeReady is not NULL,the ququeReady semaphore will decrease */
                rt_sem_take(pool->queueReady, RT_WAITING_FOREVER);
            }
        }
        if (pool->isShutdown) { /* thread pool will shutdown */
            rt_mutex_release(pool->queueLock);
            return;
        }
        RT_ASSERT(pool->curWaitThreadNum != 0);
        RT_ASSERT(pool->queueHead != NULL);
        /* load task to thread job */
        pool->curWaitThreadNum--;
        task = pool->queueHead;
        pool->queueHead = task->next;
        rt_mutex_release(pool->queueLock);
        /* run task */
        (*(task->process))(task->arg);
        /* release memory */
        free(task);
        task = NULL;
    }
}

/**
 * This function will lock the synchronized lock.
 *
 * @param pool the ThreadPool pointer
 *
 */
static void syncLock(pThreadPool pool) {
    rt_mutex_take(pool->userLock, RT_WAITING_FOREVER);
}

/**
 * This function will unlock the synchronized lock.
 *
 * @param pool the ThreadPool pointer
 *
 */
static void syncUnlock(pThreadPool pool) {
    rt_mutex_release(pool->userLock);
}

#endif
