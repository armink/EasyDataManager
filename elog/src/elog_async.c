/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2016, Armink, <armink.ztl@gmail.com>
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
 * Function: Logs asynchronous output.
 * Created on: 2016-11-06
 */

#include <elog.h>
#include <string.h>

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
#if !defined(ELOG_ASYNC_OUTPUT_BUF_SIZE)
    #error "Please configure buffer size for asynchronous output mode (in elog_cfg.h)"
#endif

#ifdef ELOG_ASYNC_OUTPUT_USING_PTHREAD
#include <pthread.h>
#include <sched.h>
/* thread default stack size */
#ifndef ELOG_ASYNC_OUTPUT_PTHREAD_STACK_SIZE
#if PTHREAD_STACK_MIN > 4*1024
#define ELOG_ASYNC_OUTPUT_PTHREAD_STACK_SIZE     PTHREAD_STACK_MIN
#else
#define ELOG_ASYNC_OUTPUT_PTHREAD_STACK_SIZE     (4*1024)
#endif
/* thread default priority */
#ifndef ELOG_ASYNC_OUTPUT_PTHREAD_PRIORITY
#define ELOG_ASYNC_OUTPUT_PTHREAD_PRIORITY       (sched_get_priority_max(SCHED_RR) - 1)
#endif
#endif /* ELOG_ASYNC_OUTPUT_USING_PTHREAD */

/* put log notice */
static pthread_cond_t put_notice;
/* put log notice lock */
static pthread_mutex_t put_notice_lock;
/* asynchronous output pthread thread */
static pthread_t async_output_thread;
#endif

/* Initialize OK flag */
static bool init_ok = false;
/* asynchronous output mode's ring buffer */
static char log_buf[ELOG_ASYNC_OUTPUT_BUF_SIZE] = { 0 };
/* log ring buffer write index */
static size_t write_index = 0;
/* log ring buffer read index */
static size_t read_index = 0;

extern void elog_port_output(const char *log, size_t size);
extern void elog_output_lock(void);
extern void elog_output_unlock(void);

/**
 * asynchronous output ring buffer used size
 *
 * @return used size
 */
static size_t elog_async_get_buf_used(void) {
    if (write_index >= read_index) {
        return write_index - read_index;
    } else {
        return ELOG_ASYNC_OUTPUT_BUF_SIZE - (read_index - write_index);
    }
}

/**
 * asynchronous output ring buffer remain space
 *
 * @return remain space
 */
static size_t async_get_buf_space(void) {
    return ELOG_ASYNC_OUTPUT_BUF_SIZE - elog_async_get_buf_used();
}

/**
 * put log to asynchronous output ring buffer
 *
 * @param log put log buffer
 * @param size log size
 *
 * @return put log size, the log which beyond ring buffer space will be dropped
 */
static size_t async_put_log(const char *log, size_t size) {
    size_t space = 0;

    space = async_get_buf_space();
    /* no space */
    if (!space) {
        size = 0;
        goto __exit;
    }
    /* drop some log */
    if (space < size) {
        size = space;
    }

    if (write_index + size < ELOG_ASYNC_OUTPUT_BUF_SIZE) {
        memcpy(log_buf + write_index, log, size);
        write_index += size;
    } else {
        memcpy(log_buf + write_index, log, ELOG_ASYNC_OUTPUT_BUF_SIZE - write_index);
        memcpy(log_buf, log + ELOG_ASYNC_OUTPUT_BUF_SIZE - write_index,
                size - (ELOG_ASYNC_OUTPUT_BUF_SIZE - write_index));
        write_index += size - ELOG_ASYNC_OUTPUT_BUF_SIZE;
    }

__exit:

    return size;
}

/**
 * get log from asynchronous output ring buffer
 *
 * @param log get log buffer
 * @param size log size
 *
 * @return get log size, the log size is less than ring buffer used size
 */
size_t elog_async_get_log(char *log, size_t size) {
    size_t used = 0;
    /* lock output */
    elog_output_lock();
    used = elog_async_get_buf_used();
    /* no log */
    if (!used) {
        size = 0;
        goto __exit;
    }
    /* less log */
    if (used < size) {
        size = used;
    }

    if (read_index + size < ELOG_ASYNC_OUTPUT_BUF_SIZE) {
        memcpy(log, log_buf + read_index, size);
        read_index += size;
    } else {
        memcpy(log, log_buf + read_index, ELOG_ASYNC_OUTPUT_BUF_SIZE - read_index);
        memcpy(log + ELOG_ASYNC_OUTPUT_BUF_SIZE - read_index, log_buf,
                size - (ELOG_ASYNC_OUTPUT_BUF_SIZE - read_index));
        read_index += size - ELOG_ASYNC_OUTPUT_BUF_SIZE;
    }

__exit:
    /* lock output */
    elog_output_unlock();
    return size;
}

void elog_async_output(const char *log, size_t size) {
    async_put_log(log, size);
    /* notify output log thread */
#ifdef ELOG_ASYNC_OUTPUT_USING_PTHREAD
    pthread_cond_signal(&put_notice);
#endif
    //TODO 构思非 pthread 模式的移植方法
}

#ifdef ELOG_ASYNC_OUTPUT_USING_PTHREAD
static void *async_output(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[ELOG_LINE_BUF_SIZE];

    ELOG_ASSERT(init_ok);

    while(true) {
        pthread_mutex_lock(&put_notice_lock);
        /* waiting log */
        pthread_cond_wait(&put_notice, &put_notice_lock);
        /* polling gets and outputs the log */
        while(true) {
            get_log_size = elog_async_get_log(poll_get_buf, ELOG_LINE_BUF_SIZE);
            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
        pthread_mutex_unlock(&put_notice_lock);
        //TODO 测试优先级等属性
    }
    return NULL;
}
#endif

/**
 * asynchronous output mode initialize
 *
 * @return result
 */
ElogErrCode elog_async_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    if (init_ok) {
        return result;
    }

#ifdef ELOG_ASYNC_OUTPUT_USING_PTHREAD
    pthread_attr_t thread_attr;
    struct sched_param thread_sched_param;

    pthread_cond_init(&put_notice, NULL);
    pthread_mutex_init(&put_notice_lock, NULL);

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&thread_attr, ELOG_ASYNC_OUTPUT_PTHREAD_STACK_SIZE);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    thread_sched_param.sched_priority = ELOG_ASYNC_OUTPUT_PTHREAD_PRIORITY;
    pthread_attr_setschedparam(&thread_attr, &thread_sched_param);
    pthread_create(&async_output_thread, &thread_attr, async_output, NULL);
    pthread_attr_destroy(&thread_attr);
#endif

    init_ok = true;

    return result;
}

#endif /* ELOG_ASYNC_OUTPUT_ENABLE */
