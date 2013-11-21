/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdio.h>
#include <board.h>
#include "log.h"
extern int  rt_application_init(void);

extern rt_uint8_t *heap;

//struct rt_thread thread_SysMonitor;
//ALIGN(RT_ALIGN_SIZE)
//rt_uint8_t thread_SysMonitor_stack[512];

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
    /* init board */
    rt_hw_board_init();

    /* show version */
    rt_show_version();

    /* init tick */
    rt_system_tick_init();

    /* init kernel object */
    rt_system_object_init();

    /* init timer system */
    rt_system_timer_init();

#ifdef RT_USING_HEAP
    /* init memory system */
    rt_system_heap_init((void *)heap, (void *)&heap[RT_HEAP_SIZE - 1]);
#endif

    /* init scheduler system */
    rt_system_scheduler_init();

    /* init all device */
#ifdef RT_USING_DEVICE
    rt_device_init_all();
#endif
    /* init application */
    rt_application_init();

    /* init timer thread */
    rt_system_timer_thread_init();

    /* init idle thread */
    rt_thread_idle_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    return ;
}

//***************************系统监控线程***************************
//函数定义: void thread_entry_SysRunLed(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink   2013-08-02   Company: BXXJS
//******************************************************************
void thread_entry_SysMonitor(void* parameter) {
	uint8_t i = 0;
	initLogger(TRUE);
	for (i = 0; i < 10; i++) {
		LogD("hello, world2");
		rt_thread_delay(1000);
	}
	destroyLogger();
	exit(0);
}

int rt_application_init() {

	static struct rt_thread thread_SysMonitor;
	ALIGN(RT_ALIGN_SIZE)
	static rt_uint8_t thread_SysMonitor_stack[512];

	rt_thread_init(&thread_SysMonitor,
                   "SysMonitor",
                   thread_entry_SysMonitor,
                   RT_NULL,
                   thread_SysMonitor_stack,
                   sizeof(thread_SysMonitor_stack),
				   4,20);
    rt_thread_startup(&thread_SysMonitor);

//	rt_thread_t tid;
//	tid = rt_thread_create("SysMonitor", thread_entry_SysMonitor, RT_NULL, 2048,
//			4, 20);
//	if (tid != RT_NULL)
//		rt_thread_startup(tid);

	return 0;
}

int main(void)
{
	/* close printf buffer */
	setbuf(stdout, NULL);

    /* disable interrupt first */
    rt_hw_interrupt_disable();

    /* startup RT-Thread RTOS */
    rtthread_startup();

    return 0;
}


/*@}*/
