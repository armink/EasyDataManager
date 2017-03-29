/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: all definition in this library
 * Created on: 2013-11-10
 */

#ifndef _EDM_DEF_H_
#define _EDM_DEF_H_

#include "edm_config.h"

/**
 * Log default configuration for EasyLogger.
 * NOTE: Must defined before including the <elog.h> and after including the <xx_cfg.h>.
 */
#if !defined(LOG_TAG)
    #define LOG_TAG                    "edm"
#endif
#undef LOG_LVL
#if defined(EDM_LOG_LVL)
    #define LOG_LVL                    EDM_LOG_LVL
#endif

#include <elog.h>

#include <stdint.h>
#include <stdbool.h>

#define EDM_VERSION                              "4.0.1"

#endif /* _EDM_DEF_H_ */
