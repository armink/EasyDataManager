/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: all types in this library
 * Created on: 2013-11-10
 */

#ifndef TYPES_H_
#define TYPES_H_

#define EDM_VERSION                      1L          /**< major version number */
#define EDM_SUBVERSION                  05L          /**< minor version number */
#define EDM_REVISION                    23L          /**< revise version number */

typedef signed   char                   int8_t;      /**<  8bit integer type */
typedef signed   short                  int16_t;     /**< 16bit integer type */
typedef signed   long                   int32_t;     /**< 32bit integer type */
typedef unsigned char                   uint8_t;     /**<  8bit unsigned integer type */
typedef unsigned short                  uint16_t;    /**< 16bit unsigned integer type */
typedef unsigned long                   uint32_t;    /**< 32bit unsigned integer type */
typedef int                             bool_t;      /**< boolean type */

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#endif /* TYPES_H_ */
