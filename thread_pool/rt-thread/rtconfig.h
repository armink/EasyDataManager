/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/* SECTION: port for visual studio */
#ifdef _MSC_VER
#undef RT_USING_NEWLIB
#undef RT_USING_MINILIBC
#define NORESOURCE  //RT_VESRION in winuser.h
#define _CRT_ERRNO_DEFINED  //errno macro redefinition

#define RT_HEAP_SIZE   (1024*1024*2)

/* disable some warning in MSC */
#pragma warning(disable:4273)	/* to ignore: warning C4273: inconsistent dll linkage */
#pragma warning(disable:4312)   /* to ignore: warning C4312: 'type cast' : conversion from 'rt_uint32_t' to 'rt_uint32_t *' */
#pragma warning(disable:4311)   /* to ignore: warning C4311: 'type cast' : pointer truncation from 'short *__w64 ' to 'long' */
#pragma warning(disable:4996)   /* to ignore: warning C4996: The POSIX name for this item is deprecated. */
#pragma warning(disable:4267)   /* to ignore: warning C4267: conversion from 'size_t' to 'rt_size_t', possible loss of data */
#pragma warning(disable:4244)   /* to ignore: warning C4244: '=' : conversion from '__w64 int' to 'rt_size_t', possible loss of data */
#else
/* SECTION: port for other ide */
#define RT_HEAP_SIZE   (1024*1024*2)
#endif


/* SECTION: basic kernel options */
/* RT_NAME_MAX*/
#define RT_NAME_MAX	16

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	4

/* PRIORITY_MAX */
#define RT_THREAD_PRIORITY_MAX  32

/* Tick per Second */
#define RT_TICK_PER_SECOND	1000

/* SECTION: RT_DEBUG */
/* Thread Debug */
#define RT_DEBUG
#define RT_THREAD_DEBUG

#define RT_USING_OVERFLOW_CHECK

/* Using Hook */
#define RT_USING_HOOK

/* Using Software Timer */
/* #define RT_USING_TIMER_SOFT */
#define RT_TIMER_THREAD_PRIO		4
#define RT_TIMER_THREAD_STACK_SIZE	512
#define RT_TIMER_TICK_PER_SECOND	10

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex */
#define RT_USING_MUTEX

/* Using Event */
#define RT_USING_EVENT

/* Using MailBox */
#define RT_USING_MAILBOX

/* Using Message Queue */
#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
/* #define RT_USING_MEMPOOL */

/* Using Dynamic Heap Management */
#define RT_USING_HEAP

/* Using Small MM */
#define RT_USING_SMALL_MEM
/* #define RT_TINY_SIZE */

#define RT_USING_CONSOLE

/* SECTION: Device System */
/* Using Device System */
//#define RT_USING_DEVICE
/* #define RT_USING_UART1 */

/* SECTION: Console options */
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128

/* SECTION: component options */
#define RT_USING_COMPONENTS_INIT

///* SECTION: finsh, a C-Express shell */
//#define RT_USING_FINSH
///* Using symbol table */
//#define FINSH_USING_SYMTAB
//#define FINSH_USING_DESCRIPTION

#endif
