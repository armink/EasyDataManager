/*
*********************************************************************************************************
*                                               uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                          (c) Copyright 2004-2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/CPU is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/CPU in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to
*               help you experience uC/CPU.  The fact that the source code is provided
*               does NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           CORE CPU MODULE
*
* Filename      : cpu_core.h
* Version       : V1.18
* Programmer(s) : SR
*                 ITJ
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  CPU_CORE_MODULE_PRESENT
#define  CPU_CORE_MODULE_PRESENT


/*$PAGE*/
/*
*********************************************************************************************************
*                                   CORE CPU MODULE VERSION NUMBER
*
* Note(s) : (1) (a) The core CPU module software version is denoted as follows :
*
*                       Vx.yy
*
*                           where
*                                   V               denotes 'Version' label
*                                   x               denotes major software version revision number
*                                   yy              denotes minor software version revision number
*
*               (b) The software version label #define is formatted as follows :
*
*                       ver = x.yy * 100
*
*                           where
*                                   ver             denotes software version number scaled as an integer value
*                                   x.yy            denotes software version number
*********************************************************************************************************
*/

#define  CPU_CORE_VERSION                                117u   /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   CPU_CORE_MODULE
#define  CPU_CORE_EXT
#else
#define  CPU_CORE_EXT  extern
#endif


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <cpu_cfg.h>

#include  <lib_mem.h>
#include  <lib_str.h>


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           CPU ERROR CODES
*********************************************************************************************************
*/

#define  CPU_ERR_NONE                                      0
#define  CPU_ERR_NULL_PTR                                 10

#define  CPU_ERR_NAME_SIZE                               100


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      CPU ERROR CODES DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  CPU_ERR;


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
CPU_CORE_EXT  CPU_CHAR  CPU_Name[CPU_CFG_NAME_SIZE];
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  CPU_Init   (void);


#if (CPU_CFG_NAME_EN == DEF_ENABLED)                            /* ------------------ CPU NAME FNCTS ------------------ */
void  CPU_NameClr(void);

void  CPU_NameGet(CPU_CHAR  *pname,
                  CPU_ERR   *perr);

void  CPU_NameSet(CPU_CHAR  *pname,
                  CPU_ERR   *perr);
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  CPU_CFG_NAME_EN
#error  "CPU_CFG_NAME_EN          not #define'd in 'cpu_cfg.h'"
#error  "                   [MUST be  DEF_ENABLED ]           "
#error  "                   [     ||  DEF_DISABLED]           "

#elif  ((CPU_CFG_NAME_EN != DEF_ENABLED ) && \
        (CPU_CFG_NAME_EN != DEF_DISABLED))
#error  "CPU_CFG_NAME_EN    illegally #define'd in 'cpu_cfg.h'"
#error  "                   [MUST be  DEF_ENABLED ]           "
#error  "                   [     ||  DEF_DISABLED]           "

#else

#ifndef  CPU_CFG_NAME_SIZE
#error  "CPU_CFG_NAME_SIZE        not #define'd in 'cpu_cfg.h' "
#error  "                   [CPU_CFG_NAME_SIZE  MUST be >=   1]"
#error  "                   [                        && <= 255]"

#elif  ((CPU_CFG_NAME_SIZE <                   1) || \
        (CPU_CFG_NAME_SIZE > DEF_INT_08U_MAX_VAL))
#error  "CPU_CFG_NAME_SIZE  illegally #define'd in 'cpu_cfg.h' "
#error  "                   [CPU_CFG_NAME_SIZE  MUST be >=   1]"
#error  "                   [                        && <= 255]"
#endif

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of CPU core module include.                      */

