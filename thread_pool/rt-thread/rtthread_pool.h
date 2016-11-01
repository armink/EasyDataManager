/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a thread pool base as RT-Thread
 * Created on: 2013-11-14
 */

#ifndef RTTHREAD_POOL_H_
#define RTTHREAD_POOL_H_

#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include <edm_def.h>

#ifdef EDM_USING_RTT

#define THREAD_POOL_THREADS_INIT_TIME     30    /**< threads initialize average waiting time */
#define THREAD_POOL_MAX_THREAD_NUM        16    /**< thread pool max setting thread number */
#define THREAD_POOL_JOB_DEFAULT_PRIORITY  10    /**< thread poll job's priority in rt-thread */
#define THREAD_POOL_JOB_TICK               5    /**< thread poll job's time slice in rt-thread */
#define THREAD_POOL_NAME_MAX     RT_NAME_MAX    /**< thread poll max name length */

/* thread pool error code */
typedef enum {
    THREAD_POOL_NO_ERR,                 /**< no error */
    THREAD_POOL_ALREADY_SHUTDOWN_ERR,   /**< thread pool already shutdown */
    THREAD_POOL_MEM_FULL_ERR,           /**< memory full */
} ThreadPoolErrCode;

/* a task queue which run in thread pool */
typedef struct _Task {
    void *(*process)(void *arg); /**< task callback function */
    void *arg;                   /**< task callback function's arguments */
    struct _Task *next;
} Task, *pTask;

/* thread pool struct */
typedef struct _ThreadPool{
    char name[THREAD_POOL_NAME_MAX + 1];/**< the name of ThreadPool, the end of name is include '\0' */
    pTask queueHead;                    /**< task queue which place all waiting task */
    rt_mutex_t userLock;                /**< a synchronized lock provided to user */
    rt_mutex_t queueLock;               /**< task queue mutex lock */
    rt_sem_t queueReady;                /**< a semaphore which for task queue ready */
    uint8_t isShutdown;                 /**< shutdown state,if shutdown the value will equal TRUE  */
    rt_thread_t* threadID;              /**< thread queue which in thread pool */
    uint8_t maxThreadNum;               /**< the thread max number in thread pool */
    uint8_t curWaitThreadNum;           /**< the current waiting thread number in thread pool */
    /**
     * This function will add a task to thread pool.
     *
     * @param pool the ThreadPool pointer
     * @param process task function pointer
     * @param arg task function arguments
     *
     * @return error code
     */
    ThreadPoolErrCode (*addTask)(struct _ThreadPool* const pool,
            void *(*process)(void *arg), void *arg);
    /**
     * This function will delete all task.
     *
     * @param pool
     *
     * @return error code
     */
    ThreadPoolErrCode (*delAll)(struct _ThreadPool* const pool);
    /**
     * This function will destroy thread pool.
     *
     * @param pool the ThreadPool pointer
     *
     * @return error code
     */
    ThreadPoolErrCode (*destroy)(struct _ThreadPool* pool);
    /**
     * This function will lock the synchronized lock.
     *
     * @param pool the ThreadPool pointer
     *
     */
    void (*lock)(struct _ThreadPool* pool);
    /**
     * This function will unlock the synchronized lock.
     *
     * @param pool the ThreadPool pointer
     *
     */
    void (*unlock)(struct _ThreadPool* pool);
} ThreadPool,*pThreadPool;

ThreadPoolErrCode initThreadPool(pThreadPool const pool, const char* name, uint8_t maxThreadNum,
        uint32_t threadStackSize);

#endif

#endif /* RTTHREAD_POOL_H_ */
