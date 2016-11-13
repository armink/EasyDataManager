/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */

#include <time.h>
#include <stdio.h>
#include <edm_def.h>
#include <elog.h>

#if defined(EDM_USING_PTHREAD)
#include <pthread.h>
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif
static pthread_mutex_t _printLock;
static pthread_mutex_t* printLock = &_printLock;
#elif defined(EDM_USING_RTT)
#include <rthw.h>
#include <rtthread.h>
static struct rt_mutex _printLock;
static rt_mutex_t printLock = &_printLock;
static struct rt_semaphore output_notice;

static void async_output(void *arg);
#endif

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_init(printLock, NULL);

#elif defined(EDM_USING_RTT)
    rt_thread_t async_thread = NULL;

    rt_mutex_init(printLock, "elog", RT_IPC_FLAG_PRIO);
    rt_sem_init(&output_notice, "elog async", 0, RT_IPC_FLAG_PRIO);

    async_thread = rt_thread_create("elog_async", async_output, NULL, 1024, RT_THREAD_PRIORITY_MAX - 1, 10);
    if (async_thread) {
        rt_thread_startup(async_thread);
    }
#endif

    printf("Welcome to use Easy Data Manager(V%s) by armink. E-Mail: armink.ztl@gmail.com \n", EDM_VERSION);

    return result;
}

#if defined(EDM_USING_RTT)
void rt_hw_console_output(const char *str) {
    printf("%s", str);
}

void elog_async_output_notice(void) {
    rt_sem_release(&output_notice);
}

static void async_output(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];

    while(true) {
        /* waiting log */
        rt_sem_take(&output_notice, RT_WAITING_FOREVER);
        /* polling gets and outputs the log */
        while(true) {
            get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
}
#endif

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* output to terminal */
#if defined(EDM_USING_PTHREAD)
    printf("%.*s", size, log);
#elif defined(EDM_USING_RTT)
    printf("%.*s", size, log);
#endif
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_lock(printLock);

#elif defined(EDM_USING_RTT)
    rt_mutex_take(printLock, RT_WAITING_FOREVER);
#endif
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_unlock(printLock);

#elif defined(EDM_USING_RTT)
    rt_mutex_release(printLock);
#endif
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    static char cur_system_time[32] = { 0 };
#if defined(EDM_USING_PTHREAD)
#if defined(WIN32) || defined(WIN64)
    SYSTEMTIME currTime;
    GetLocalTime(&currTime);
    snprintf(cur_system_time, 32, "%02d-%02d %02d:%02d:%02d.%03d ", currTime.wMonth, currTime.wDay,
            currTime.wHour, currTime.wMinute, currTime.wSecond,
            currTime.wMilliseconds);
#else
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep);
    if(p==NULL) {
        return;
    }
    snprintf(cur_system_time, 32, "%02d-%02d %02d:%02d:%02d.%03d ",p->tm_mon+1 ,p->tm_mday ,p->tm_hour ,p->tm_min,p->tm_sec);
#endif

#elif defined(EDM_USING_RTT)
    rt_snprintf(cur_system_time, 16, "tick:%010d", rt_tick_get());
#endif
    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    static char cur_thread_name[64] = { 0 };
#if defined(EDM_USING_PTHREAD)
#if defined(WIN32) || defined(WIN64)
    snprintf(cur_thread_name, 64, "tid:%04ld ",GetCurrentThreadId());
#else
    snprintf(cur_thread_name, 64, "tid:%#x ",pthread_self());
#endif
#elif defined(EDM_USING_RTT)
    snprintf(cur_thread_name, 64, "thread_name:%s ",rt_thread_self()->name);
#endif
    return cur_thread_name;
}
