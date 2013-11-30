/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : system.h
* Create  : ЛЊаж Email:591881218@qq.com
************************************************************************************************************************
*/

#ifndef SYSTEM_H
#define SYSTEM_H

#define  OS_MSG_TRACE                   1u

#define  DISABLE						0u
#define  ENABLE							1u

#define  FP_SAVE_RESTORE_EN             1u

#define  FP_STK_SIZE                  256u
#define  FP_BYTES          FP_STK_SIZE * 4 //256 * 4Bytes = 1024Bytes

extern CPU_BOOLEAN ExecEn;
extern void DispTaskExecInfo(void);
extern void	UpdateTaskExecInfo(void);
extern void VCInit(void);

#endif