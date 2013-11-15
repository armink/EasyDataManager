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

ThreadPoolErrCode initThreadPool(pThreadPool const pool, uint8_t maxThreadNum) {
	ThreadPoolErrCode errorCode = THREAD_POOL_NO_ERR;
	return errorCode;
}

#endif
