/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a thread pool base as pthread
 * Created on: 2013-11-07
 */

#include "pthread_pool.h"

#ifdef EDM_USING_PTHREAD

#define LOG_TAG    "edm.threadpool"
#define assert     ELOG_ASSERT
#define log_e(...) elog_e(LOG_TAG, __VA_ARGS__)
#define log_w(...) elog_w(LOG_TAG, __VA_ARGS__)
#define log_i(...) elog_i(LOG_TAG, __VA_ARGS__)

#if EDM_DEBUG
    #define log_d(...) elog_d(LOG_TAG, __VA_ARGS__)
#else
    #define log_d(...)
#endif
static ThreadPoolErrCode addTask(pThreadPool const pool,
        void *(*process)(void *arg), void *arg);
static ThreadPoolErrCode destroy(pThreadPool pool);
static void* threadJob(void* arg);
static void syncLock(pThreadPool pool);
static void syncUnlock(pThreadPool pool);
static ThreadPoolErrCode delAll(pThreadPool const pool);

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
ThreadPoolErrCode initThreadPool(pThreadPool const pool, const char* name, uint8_t maxThreadNum, uint32_t threadStack) {
    ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
    uint8_t i = 0;

    if (maxThreadNum > THREAD_POOL_MAX_THREAD_NUM) {
        errorCode = THREAD_POOL_MAX_NUM_ERR;
    }
    if (errorCode == THREAD_POOL_NO_ERR) {
        pthread_mutex_init(&(pool->queueLock), NULL);
        pthread_mutex_init(&(pool->userLock), NULL);
        pthread_cond_init(&(pool->queueReady), NULL);
        pool->queueHead = NULL;
        pool->maxThreadNum = maxThreadNum;
        pool->curWaitThreadNum = 0;
        pool->isShutdown = false;
        pool->addTask = addTask;
        pool->delAll = delAll;
        pool->destroy = destroy;
        pool->lock = syncLock;
        pool->unlock = syncUnlock;
        pool->threadID = (pthread_t *) malloc(maxThreadNum * sizeof(pthread_t));
        assert(pool->threadID != NULL);
        for (i = 0; i < maxThreadNum; i++) {
            pthread_create(&(pool->threadID[i]), NULL, threadJob, pool);
            log_d("create thread success.Current total thread number is %d", i + 1);
        }
        log_d("initialize thread pool success!");
    }
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
static ThreadPoolErrCode addTask(pThreadPool const pool, void *(*process)(void *arg),
        void *arg) {
    ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
    pTask member = NULL;
    pTask newtask = (pTask) malloc(sizeof(Task));
    assert(newtask != NULL);
    newtask->process = process;
    newtask->arg = arg;
    newtask->next = NULL;
    /* lock thread pool */
    pthread_mutex_lock(&(pool->queueLock));
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
    pthread_mutex_unlock(&(pool->queueLock));
    /* wake up a waiting thread to process task */
    pthread_cond_signal(&(pool->queueReady));
    log_d("add a task to task queue success.");
    return errorCode;
}

/**
 * This function will delete all wait task.
 *
 * @param pool the ThreadPool pointer
 *
 * @return error code
 */
static ThreadPoolErrCode delAll(pThreadPool const pool) {
    ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;

    //fixme

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
        pool->isShutdown = true;
        /* wake up all thread from broadcast */
        pthread_cond_broadcast(&(pool->queueReady));
        /* wait all thread exit */
        for (i = 0; i < pool->maxThreadNum; i++) {
            log_d("Thread pool will destroy,waiting the thread exit");
            pthread_join(pool->threadID[i], NULL);
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
        /* destroy mutex and conditional variable */
        pthread_mutex_destroy(&(pool->queueLock));
        pthread_mutex_destroy(&(pool->userLock));
        pthread_cond_destroy(&(pool->queueReady));
        /* release memory */
        free(pool);
        pool = NULL;
        log_d("Thread pool destroy success");
    }
    return errorCode;
}

/**
 * This function is thread job.
 *
 * @param arg the thread job arguments
 *
 */
static void* threadJob(void* arg) {
    pThreadPool pool = NULL;
    while (1) {
        pool = (pThreadPool)arg;
        pTask task = NULL;
        /* lock thread pool */
        pthread_mutex_lock(&(pool->queueLock));
        /* If waiting thread number is 0 ,and thread is not shutdown.
         * The thread will block.
         * Before thread block the queueLock will unlock.
         * After thread wake up ,the queueLock will relock.*/
        while (pool->curWaitThreadNum == 0 && !pool->isShutdown) {
            log_d("the thread waiting for task add to task queue");
            pthread_cond_wait(&(pool->queueReady), &(pool->queueLock));
        }
        if (pool->isShutdown) { /* thread pool will shutdown */
            pthread_mutex_unlock(&(pool->queueLock));
            pthread_exit(NULL);
        }
        assert(pool->curWaitThreadNum != 0);
        assert(pool->queueHead != NULL);
        /* load task to thread job */
        pool->curWaitThreadNum--;
        task = pool->queueHead;
        pool->queueHead = task->next;
        pthread_mutex_unlock(&(pool->queueLock));
        /* run task */
        (*(task->process))(task->arg);
        /* release memory */
        free(task);
        task = NULL;
    }
    /* never reach here */
    pthread_exit(NULL);
    return NULL;
}

/**
 * This function will lock the synchronized lock.
 *
 * @param pool the ThreadPool pointer
 *
 */
static void syncLock(pThreadPool pool) {
//    log_d("is syncLock");
    pthread_mutex_lock(&(pool->userLock));
}

/**
 * This function will unlock the synchronized lock.
 *
 * @param pool the ThreadPool pointer
 *
 */
static void syncUnlock(pThreadPool pool) {
//    log_d("is syncUnlock");
    pthread_mutex_unlock(&(pool->userLock));
}

#endif
