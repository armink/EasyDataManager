/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: an auto refresher to refresh dynamic Cache data in this library
 * Created on: 2014-01-10
 */

#include "refresher.h"

#ifdef EDM_USING_RTT

#define LOG_TAG    "edm.refresher"
#define assert     ELOG_ASSERT
#define log_e(...) elog_e(LOG_TAG, __VA_ARGS__)
#define log_w(...) elog_w(LOG_TAG, __VA_ARGS__)
#define log_i(...) elog_i(LOG_TAG, __VA_ARGS__)

#if EDM_DEBUG
    #define log_d(...) elog_d(LOG_TAG, __VA_ARGS__)
#else
    #define log_d(...)
#endif

static RefresherErrCode add(pRefresher const refresher, const char* name, int8_t priority, uint32_t period,
        int16_t times, bool newThread, uint32_t satckSize, void (*refreshProcess)(void *arg));
static RefresherErrCode del(pRefresher const refresher, const char* name);
static RefresherErrCode delAll(pRefresher const refresher);
static RefresherErrCode destroy(pRefresher const refresher);
static RefresherErrCode setPeriodAndPriority(pRefresher const refresher, const char* name, uint32_t period,
        int8_t priority);
static RefresherErrCode setTimes(pRefresher const refresher, const char* name, int16_t times);
static void kernel(void* arg);
static pRefreshJob hasJob(pRefresher const refresher, const char* name);
static void newThreadJob(void* arg);
static void addJobToReadyQueue(pRefresher const refresher);
static pRefreshJob selectJobFromReadyQueue(pRefresher const refresher);
static RefresherErrCode delJobInRefreshQueue(pRefresher const refresher, const char* name);
static RefresherErrCode delJobInReadyQueue(pRefresher const refresher, const char* name);

/**
 * initialize the refresher
 *
 * @param refresher the refresher pointer
 * @param stackSize the refresher kernel thread size
 * @param priority the refresher kernel thread priority
 * @param tick the refresher kernel thread tick
 *
 * @return error code
 */
RefresherErrCode initRefresher(pRefresher const refresher, uint32_t stackSize, uint8_t priority, uint32_t tick) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    assert(refresher != NULL);
    /* initialize refresher */
    refresher->tick = tick;
    refresher->add = add;
    refresher->del = del;
    refresher->delAll = delAll;
    refresher->destroy = destroy;
    refresher->setPeriodAndPriority = setPeriodAndPriority;
    refresher->setTimes = setTimes;
    refresher->queueHead = NULL;
    refresher->readyQueueHead = NULL;
    /* create job queue mutex lock */
    refresher->queueLock = rt_mutex_create("refresher", RT_IPC_FLAG_FIFO);
    assert(refresher->queueLock != NULL);
    /* create and start kernel thread */
    refresher->kernelID = rt_thread_create("refresher", kernel, refresher, stackSize, priority, tick);
    assert(refresher->kernelID != NULL);
    rt_thread_startup(refresher->kernelID);
    log_d("initialize refresher success");
    return errorCode;
}

/**
 * This function is refresher kernel thread.Run all of job what newThread is false.
 *
 * @param arg the kernel arguments
 *
 * @see newThreadJob
 */
static void kernel(void* arg) {
    pRefresher refresher = (pRefresher) arg;
    pRefreshJob job = NULL;
    uint32_t startTime, runningTime;
    while (1) {
        startTime = rt_tick_get();
        /* Search and add ready job to ready queue. */
        addJobToReadyQueue(refresher);
        /* Ready queue may have more than one job want to run.*/
        while (1) {
            /* Select a job form ready job queue */
            job = selectJobFromReadyQueue(refresher);
            /* run job */
            if (job != NULL) {
                job->refreshProcess(job);
                /* Give a little time to OS.Make sure OS Real-time. */
                rt_thread_delay(0);
            } else {
                break;
            }
        }
        /* calculate refreshProcess running time */
        runningTime = rt_tick_get() - startTime;
        /* delay sometime.Make sure kernel be runned in a tick time. */
        if (runningTime < refresher->tick) {
            rt_thread_delay(refresher->tick - runningTime);
        } else {/* refreshProcess running time was greater than refresh time.So it's no need delay */
            rt_thread_delay(0);
        }
    }
}

/**
 * Search and add ready job to ready queue.
 *
 * @param refresher the refresher pointer
 *
 */
static void addJobToReadyQueue(pRefresher const refresher) {
    pRefreshJob job = NULL;
    pReadyJob readyJob = NULL, readyJobTemp = NULL;
    job = refresher->queueHead;
    readyJob = refresher->readyQueueHead;
    /* the ready queue is NULL */
    if (refresher->queueHead == NULL) {
        return;
    }
    /* lock job queue */
    /* decrease all ready job curPeriod */
    rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
    for (;;) {
        /* job find finish */
        if (readyJob == NULL) {
            break;
        } else {
            readyJob->curPeriod--;
            readyJob = readyJob->next;
        }
    }
    /* unlock job queue */
    rt_mutex_release(refresher->queueLock);
    /* search and add ready job to ready queue */
    for (;;) {
        if (job == NULL) {/* job find finish */
            break;
        } else if ((job->newThread == false) && (job->times != 0)) {
            /* If ready job queue is NULL,then add this job to queue immediately.  */
            if (refresher->readyQueueHead != NULL) {
                readyJob = refresher->readyQueueHead;
                for (;;) {
                    /* The ready queue has this job.It isn't need to add.*/
                    if (readyJob == NULL) {/* Find finish.This job isn't in the ready queue.Add it. */
                        break;
                    } else if (!strcmp(readyJob->job->name, job->name)) { /* find success */
                        break;
                    } else {
                        readyJobTemp = readyJob; /* backup ready job */
                        readyJob = readyJob->next;
                    }
                }
            }
            /* Found ready queue finish.If job not found in ready queue,add it. */
            if (readyJob == NULL) {
                /* get tail ready job */
                readyJob = readyJobTemp;
                readyJobTemp = (pReadyJob) rt_malloc(sizeof(ReadyJob));
                if (!readyJobTemp) {
                    log_w("Memory full!");
                    return;
                }
                /* lock job queue */
                rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
                if (refresher->readyQueueHead == NULL) { /* ready job queue head */
                    refresher->readyQueueHead = readyJobTemp;
                } else {
                    readyJob->next = readyJobTemp;
                }
                readyJob = readyJobTemp;
                readyJob->job = job;
                readyJob->curPeriod = job->period;
                readyJob->next = NULL;
                /* unlock job queue */
                rt_mutex_release(refresher->queueLock);
            }
        }
        job = job->next;
    }
}

/**
 * Select a job form ready job queue.Next this job will be run.
 *
 * @param refresher the refresher pointer
 *
 * @return the selected job pointer
 */
static pRefreshJob selectJobFromReadyQueue(pRefresher const refresher) {
    pRefreshJob job = NULL;
    pReadyJob readyJob = refresher->readyQueueHead, readyJobTemp = NULL;
    uint8_t highestPriority = 255;
    /* the ready queue is NULL */
    if (refresher->readyQueueHead == NULL) {
        return NULL;
    }
    for (;;) {
        if (readyJob == NULL) { /* job find finish */
            /* there was no job could be run */
            if (job == NULL) {
                return NULL;
            }
            /* the job running time is 0,then will be stop */
            if (job->times == 0) {
                /* find the job */
                readyJob = refresher->readyQueueHead;
                if (readyJob->job != job) {
                    for (;;) {
                        if (readyJob->next->job == job) {
                            break;
                        } else {
                            readyJob = readyJob->next;
                        }
                    }
                }
                /* lock job queue */
                rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
                /* delete job */
                if ((readyJob->job != job) && (readyJob == refresher->readyQueueHead)) {/* queue head */
                    if (readyJob->next == NULL) { /* queue has one node */
                        refresher->readyQueueHead = NULL;
                        /* job will be freed in the end */
                    } else {
                        refresher->readyQueueHead = refresher->readyQueueHead->next;
                        /* job will be freed in the end */
                    }
                } else if (readyJob->next->next == NULL) { /* queue tail */
                    readyJobTemp = readyJob->next;
                    readyJob->next = NULL;
                    readyJob = readyJobTemp; /* job will be freed in the end */
                } else {
                    readyJobTemp = readyJob->next;
                    readyJob->next = readyJob->next->next;
                    readyJob = readyJobTemp; /* job will be freed in the end */
                }
                /* unlock job queue */
                rt_mutex_release(refresher->queueLock);
                rt_free(readyJob);
                readyJob = NULL;
                return NULL;
            } else if (job->times > 0) {
                /* lock job queue */
                rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
                /* decrease job running times. */
                job->times--;
                /* unlock job queue */
                rt_mutex_release(refresher->queueLock);
                /* Restore the period for this ready job. */
                readyJobTemp->curPeriod = job->period;
                return job;
            } else if (job->times == REFRESHER_JOB_CONTINUES_RUN) {
                /* Restore the period for this ready job. */
                readyJobTemp->curPeriod = job->period;
                return job;
            }
        } else {
            /* find the ready job which highest priority and period equal 0 */
            if ((readyJob->job->priority < highestPriority) && (readyJob->curPeriod == 0)) {
                job = readyJob->job;
                highestPriority = readyJob->job->priority;
                readyJobTemp = readyJob;
            }
            readyJob = readyJob->next;
        }
    }
}

/**
 * the newThread job will use this thread
 *
 * @param arg the job arguments
 *
 * @see kernel
 */
static void newThreadJob(void* arg) {
    pRefresher refresher = (pRefresher) arg;
    rt_thread_t thread = rt_thread_self();
    pRefreshJob job = NULL;
    uint32_t startTime, runningTime;
    /* get job object */
    job = hasJob(refresher, thread->name);
    assert(job != NULL);
    while (1) {
        /* backup current system time */
        startTime = rt_tick_get();
        /* If job running times is 0 , the job will stop*/
        if (job->times != 0) {
            /* run job refreshProcess */
            job->refreshProcess(job);
            /* If job times is REFRESHER_JOB_CONTINUES_RUN.Job will be in continuous running mode. */
            if (job->times != REFRESHER_JOB_CONTINUES_RUN) {
                /* lock job queue */
                rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
                job->times--;
                /* unlock job queue */
                rt_mutex_release(refresher->queueLock);
            }
        }
        /* calculate refreshProcess running time */
        runningTime = rt_tick_get() - startTime;
        /* delay sometime.Make sure job be runned in setting time. */
        if (runningTime < job->period * refresher->tick) {
            rt_thread_delay(job->period * refresher->tick - runningTime);
        } else {/* refreshProcess running time was greater than refresh time.So it's no need delay */
            rt_thread_delay(0);
        }
    }
}

/**
 * find the job in refresher's job queue
 *
 * @param refresher the refresher pointer
 * @param name job name
 *
 * @return job pointer when find success
 */
static pRefreshJob hasJob(pRefresher const refresher, const char* name) {
    pRefreshJob member = refresher->queueHead;
    assert((name != NULL) && (strlen(name) <= REFRESHER_JOB_NAME_MAX));
    /* job queue is empty */
    if (refresher->queueHead == NULL) {
        return NULL;
    }
    for (;;) {
        if (!strcmp(member->name, name)) {
            return member;
        } else {
            if (member->next == NULL) {/* queue tail */
                return NULL;
            }
            member = member->next;
        }
    }
}

/**
 * add a job to refresher
 *
 * @param refresher the refresher pointer
 * @param name job name
 * @param priority Job refresh priority.The highest priority is 0.
 * @param period Refresh time = period * refresher tick. @see Refresher.tickTime
 * @param times If it is REFRESHER_JOB_CONTINUES_RUN,the job will continuous running.
 * @param newThread If it is TRUE,refresher will new a thread to refresh this job.
 * @param satckSize The new thread job stack size.It is not NULL while newThread is TRUE.
 * @param refreshProcess the job refresh process
 *
 * @return error code
 */
static RefresherErrCode add(pRefresher const refresher, const char* name, int8_t priority, uint32_t period,
        int16_t times, bool newThread, uint32_t satckSize, void (*refreshProcess)(void *arg)) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    pRefreshJob member = NULL;
    pRefreshJob newJob = NULL;

    assert(refresher != NULL);
    assert(period > 0);
    assert((times >= 0) || (times == -1));

    if (hasJob(refresher, name) != NULL) {/* the job is already exist in job queue */
        log_d("the name of %s job is already exist in refresher", name);
        return REFRESHER_JOB_NAME_ERROR;
    } else {
        newJob = (pRefreshJob) rt_malloc(sizeof(RefreshJob));
        if (!newJob) {
            log_w("Memory full!");
            return REFRESHER_MEM_FULL_ERR;
        }
        strcpy(newJob->name, name);
    }

    newJob->priority = priority;
    newJob->period = period;
    newJob->times = times;
    newJob->newThread = newThread;
    newJob->refreshProcess = refreshProcess;
    newJob->next = NULL;

    if (newThread) {/* the job need new thread to run */
        newJob->threadID = rt_thread_create(name, newThreadJob, refresher, satckSize, priority, refresher->tick);
        if (!newJob->threadID) {
            log_w("Memory full!");
            rt_free(newJob);
            return REFRESHER_MEM_FULL_ERR;
        }
        rt_thread_startup(newJob->threadID);
    } else {/* the job running kernel thread */
        newJob->threadID = refresher->kernelID;
    }
    /* lock job queue */
    rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
    member = refresher->queueHead;
    if (member == NULL) {/* job queue is NULL */
        refresher->queueHead = newJob;
    } else {
        /* look up for queue tail */
        while (member->next != NULL) {
            member = member->next;
        }
        member->next = newJob;
    }
    /* unlock job queue */
    rt_mutex_release(refresher->queueLock);
    log_d("add a job to refresher success.");

    return errorCode;
}

/**
 * delete a job in refresh queue
 *
 * @param refresher the refresher pointer
 * @param name job name
 *
 * @return error code
 *
 * @see delJobInReadyQueue
 */
static RefresherErrCode delJobInRefreshQueue(pRefresher const refresher, const char* name) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    pRefreshJob member = refresher->queueHead, memberTemp = NULL;

    assert(refresher != NULL);
    assert((name != NULL) && (strlen(name) <= REFRESHER_JOB_NAME_MAX));

    /* job queue doesn' have job */
    if (member == NULL) {
        errorCode = REFRESHER_NO_JOB;
    }
    /* find the job in job queue */
    if (errorCode == REFRESHER_NO_ERR) {
        if (strcmp(member->name, name)) { /* queue head */
            for (;;) {
                if (member->next == NULL) { /* find finish */
                    errorCode = REFRESHER_JOB_NAME_ERROR;
                    break;
                } else {
                    if (!strcmp(member->next->name, name)) {
                        /* backup last node */
                        memberTemp = member;
                        /* the delete object is member->next */
                        member = member->next;
                        break;
                    } else {
                        member = member->next;
                    }
                }
            }
        }
    }
    /* lock job queue */
    rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
    /* delete job and free ram */
    if (errorCode == REFRESHER_NO_ERR) {
        if (member == refresher->queueHead) {/* delete job is queue head */
            /* the job queue has one node */
            if (member->next == NULL) {
                refresher->queueHead = NULL;
                /* job will be freed in the end */
            } else { /* the job queue has more than one node*/
                refresher->queueHead = refresher->queueHead->next;
                /* job will be freed in the end */
            }
        } else if (member->next == NULL) {/* delete job is tail node */
            memberTemp->next = NULL; /* job will be freed in the end */
        } else {
            memberTemp->next = memberTemp->next->next; /* job will be freed in the end */
        }
        /* the new thread job will detach thread in OS */
        if (member->newThread) {
            rt_thread_delete(member->threadID);
        }
        rt_free(member);
        member = NULL;
    } else {
        errorCode = REFRESHER_JOB_NAME_ERROR;
    }
    /* unlock job queue */
    rt_mutex_release(refresher->queueLock);
    return errorCode;
}

/**
 * Delete a job in ready queue.It will stop job then delete job.
 *
 * @param refresher the refresher pointer
 * @param name job name
 *
 * @return error code
 *
 * @see delJobInRefreshQueue
 */
static RefresherErrCode delJobInReadyQueue(pRefresher const refresher, const char* name) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    pReadyJob member = refresher->readyQueueHead, memberTemp = NULL;

    assert(refresher != NULL);
    assert((name != NULL) && (strlen(name) <= REFRESHER_JOB_NAME_MAX));

    /* job queue doesn' have job */
    if (member == NULL) {
        errorCode = REFRESHER_NO_JOB;
    }
    /* find the job in job queue */
    if (errorCode == REFRESHER_NO_ERR) {
        if (strcmp(member->job->name, name)) { /* queue head */
            for (;;) {
                if (member->next == NULL) { /* find finish */
                    errorCode = REFRESHER_JOB_NAME_ERROR;
                    break;
                } else {
                    if (!strcmp(member->next->job->name, name)) {
                        /* backup last node */
                        memberTemp = member;
                        /* the delete object is member->next */
                        member = member->next;
                        break;
                    } else {
                        member = member->next;
                    }
                }
            }
        }
    }
    /* lock job queue */
    rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
    /* delete job and free ram */
    if (errorCode == REFRESHER_NO_ERR) {
        if (member == refresher->readyQueueHead) {/* delete job is queue head */
            /* the job queue has one node */
            if (member->next == NULL) {
                refresher->readyQueueHead = NULL;
                /* job will be freed in the end */
            } else { /* the job queue has more than one node*/
                refresher->readyQueueHead = refresher->readyQueueHead->next;
                /* job will be freed in the end */
            }
        } else if (member->next == NULL) {/* delete job is tail node */
            memberTemp->next = NULL; /* job will be freed in the end */
        } else {
            memberTemp->next = memberTemp->next->next; /* job will be freed in the end */
        }
        /* make job stop and free it in ready queue */
        member->job->times = 0;
        rt_free(member);
        member = NULL;
    } else {
        errorCode = REFRESHER_JOB_NAME_ERROR;
    }
    /* unlock job queue */
    rt_mutex_release(refresher->queueLock);
    return errorCode;
}


/**
 * delete a job in refresher.@see delJobInReadyQueue @see delJobInRefreshQueue
 *
 * @param refresher the refresher pointer
 * @param name job name
 *
 * @return error code
 */
static RefresherErrCode del(pRefresher const refresher, const char* name) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    delJobInReadyQueue(refresher, name);
    errorCode = delJobInRefreshQueue(refresher, name);
    return errorCode;
}

/**
 * deleted all jobs in refresher.
 *
 * @param refresher the refresher pointer
 *
 * @return error code
 */
static RefresherErrCode delAll(pRefresher const refresher) {
    RefresherErrCode errorCode = REFRESHER_NO_JOB;
    rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
    for (;;) {
        if (refresher->readyQueueHead != NULL) {
            rt_free(refresher->readyQueueHead);
            refresher->readyQueueHead = refresher->readyQueueHead->next;
        } else {
            break;
        }
    }
    for (;;) {
        if (refresher->queueHead != NULL) {
            if (refresher->queueHead->newThread) {
                rt_thread_delete(refresher->queueHead->threadID);
            }
            rt_free(refresher->queueHead);
            refresher->queueHead = refresher->queueHead->next;
        } else {
            break;
        }
    }
    rt_mutex_release(refresher->queueLock);
    log_d("deleted all job in refresher.");
    return errorCode;
}

/**
 * destroy refresher
 *
 * @param refresher
 * @return error code
 */
static RefresherErrCode destroy(pRefresher const refresher) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;

    refresher->delAll(refresher);
    rt_thread_delete(refresher->kernelID);
    rt_mutex_delete(refresher->queueLock);
    log_d("refresher destroy success.");
    return errorCode;
}

/**
 * set the job period and priority
 *
 * @param refresher the refresher pointer
 * @param name job name
 * @param period job period
 * @param priority job priority
 *
 * @return error code
 */
static RefresherErrCode setPeriodAndPriority(pRefresher const refresher, const char* name, uint32_t period,
        int8_t priority) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    pRefreshJob member = refresher->queueHead;

    assert(refresher != NULL);
    assert(period > 0);

    /* job queue doesn't have job */
    if (member == NULL) {
        errorCode = REFRESHER_NO_JOB;
    }

    if (errorCode == REFRESHER_NO_ERR) {
        /* lock job queue */
        rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
        if ((member = hasJob(refresher, name)) != NULL) {
            member->period = period;
            member->priority = priority;
            /* the new thread job will change thread priority */
            if (member->newThread) {
                rt_thread_control(member->threadID,
                RT_THREAD_CTRL_CHANGE_PRIORITY, &member->priority);
            }
        } else {
            errorCode = REFRESHER_JOB_NAME_ERROR;
        }
        /* unlock job queue */
        rt_mutex_release(refresher->queueLock);
    }

    return errorCode;
}

/**
 * set the job running times
 *
 * @param refresher the refresher pointer
 * @param name job name
 * @param times job running times
 *
 * @return error code
 */
static RefresherErrCode setTimes(pRefresher const refresher, const char* name, int16_t times) {
    RefresherErrCode errorCode = REFRESHER_NO_ERR;
    pRefreshJob member = refresher->queueHead;

    assert(refresher != NULL);
    assert((times >= 0) || (times == -1));

    /* job queue doesn' have job */
    if (member == NULL) {
        errorCode = REFRESHER_NO_JOB;
    }

    if (errorCode == REFRESHER_NO_ERR) {
        /* lock job queue */
        rt_mutex_take(refresher->queueLock, RT_WAITING_FOREVER);
        if ((member = hasJob(refresher, name)) != NULL) {
            member->times = times;
        } else {
            errorCode = REFRESHER_JOB_NAME_ERROR;
        }
        /* unlock job queue */
        rt_mutex_release(refresher->queueLock);
    }

    return errorCode;
}

#endif
