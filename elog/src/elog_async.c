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
#endif

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
static size_t async_get_buf_used(void) {
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
    return ELOG_ASYNC_OUTPUT_BUF_SIZE - async_get_buf_used();
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
    /* lock output */
    elog_output_lock();
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
    /* lock output */
    elog_output_lock();
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
size_t elog_async_get_log(size_t size, char *log) {
    size_t used = 0;
    /* lock output */
    elog_output_lock();
    used = async_get_buf_used();
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
    elog_output_lock();
    return size;
}

void elog_async_output(const char *log, size_t size) {
    async_put_log(log, size);
    //TODO 通知日志输出端
}

ElogErrCode elog_async_init(void) {
}

#endif /* ELOG_ASYNC_OUTPUT_ENABLE */
