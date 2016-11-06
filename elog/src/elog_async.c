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
 * Function: Logs asynchronous output and buffered output.
 * Created on: 2016-11-06
 */

#include <elog.h>
#include <string.h>

#ifdef ELOG_BUFF_OUTPUT_ENABLE
#if !defined(ELOG_BUFF_OUTPUT_BUFF_SIZE)
    #error "Please configure buffer size for buffered output mode (in elog_cfg.h)"
#endif

/* buffered output mode's buffer */
static char log_buf[ELOG_BUFF_OUTPUT_BUFF_SIZE] = { 0 };
/* log buffer current write size */
static size_t buf_write_size = 0;
#endif

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
#endif

void elog_async_output(const char *log, size_t size);

extern void elog_port_output(const char *log, size_t size);
extern void elog_output_lock(void);
extern void elog_output_unlock(void);

#ifdef ELOG_BUFF_OUTPUT_ENABLE
/**
 * output buffered logs when buffer is full
 *
 * @param log will buffered line's log
 * @param size log size
 */
void elog_buf_output(const char *log, size_t size) {
    size_t write_size = 0, write_index = 0;

    while (true) {
        if (buf_write_size + size > ELOG_BUFF_OUTPUT_BUFF_SIZE) {
            write_size = ELOG_BUFF_OUTPUT_BUFF_SIZE - buf_write_size;
            memcpy(log_buf + buf_write_size, log + write_index, write_size);
            write_index += write_size;
            size -= write_size;
            buf_write_size += write_size;
            /* output log */
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
            elog_async_output(log_buf, buf_write_size);
#else
            elog_port_output(log_buf, buf_write_size);
#endif
            /* reset write index */
            buf_write_size = 0;
        } else {
            memcpy(log_buf + buf_write_size, log + write_index, size);
            buf_write_size += size;
            break;
        }
    }
}

/**
 * flush all buffered logs to output device
 */
void elog_flush(void) {
    /* lock output */
    elog_output_lock();
    /* output log */
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    elog_async_output(log_buf, buf_write_size);
#else
    elog_port_output(log_buf, buf_write_size);
#endif
    /* reset write index */
    buf_write_size = 0;
    /* unlock output */
    elog_output_unlock();
}
#endif

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
void elog_async_output(const char *log, size_t size) {

}
#endif
