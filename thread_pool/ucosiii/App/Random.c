/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : Random.c
* Modify  : ЛЊаж Email:591881218@qq.com
************************************************************************************************************************
*/

#include <os.h>
#include <pc.h>
// ------------------------------
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
// ------------------------------
#pragma comment(lib, "Winmm.lib")
#pragma warning(disable : 4761) 
// ------------------------------

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE			2048 * 10        /* Size of each task's stacks                         */
#define  N_TASKS					   10        /* Number of identical tasks                          */

#define  TASK_START_PRIO			   20        /* Application tasks priorities                       */
#define  TASK_RANDOM_PRIO			   21

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

CPU_STK        TaskStartStk[TASK_STK_SIZE];           /* Tasks stacks                                  */
CPU_STK        TaskRandomStk[N_TASKS][TASK_STK_SIZE]; 

OS_TCB 	       TaskStartTcb;                          /* Tasks TCBs                                    */
OS_TCB         TaskRandomTcb[N_TASKS];

OS_SEM         RandomSem;

CPU_CHAR       TaskRandomData[N_TASKS];               /* Parameters to pass to each task               */

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

		void  Random(void);
static  void  TaskStart(void *p_arg);
static  void  TaskRandom(void *p_arg);
static  void  TaskStartDispInit(void);
static  void  TaskStartCreateTasks (void);

/*
*********************************************************************************************************
*                                               Random
*********************************************************************************************************
*/

void Random(void)
{
	OS_ERR  err;


    OSTaskCreate((OS_TCB     *)&TaskStartTcb,             /* Create the App Start Task.                */
                 (CPU_CHAR   *)"Task Start",
                 (OS_TASK_PTR )TaskStart, 
                 (void       *)0,
                 (OS_PRIO     )TASK_START_PRIO,
                 (CPU_STK    *)&TaskStartStk[0],
                 (CPU_STK_SIZE)TASK_STK_SIZE/10,
                 (CPU_STK_SIZE)TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}

/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/

static void TaskStart(void *p_arg)
{
	OS_ERR  err;


	/* OS_TICKS_PER_SEC = 200, 5ms/tick */
	timeSetEvent(5, 0, OSTickISR, 0, TIME_PERIODIC);       /* Must be first                            */

	p_arg = p_arg;

	OSSemCreate((OS_SEM   *)&RandomSem,
                (CPU_CHAR *)"Random Sem",
                (OS_SEM_CTR)1,
                (OS_ERR   *)&err);                         /* Random number semaphore                  */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit((OS_ERR *)&err);
#endif

	TaskStartDispInit();                                   /* Initialize the display                   */

	TaskStartCreateTasks();                                /* Create all the application tasks         */

	OSTaskDel(&TaskStartTcb, &err);                        /* Delete start task                        */
}

/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static void TaskStartDispInit(void)
{
	PC_DispStr(0, 0,  "                         uC/OS-III,The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 2,  "                                Jean J. Labrosse                                ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 22, "                                                              591881218@qq.com  ", DISP_FGND_WHITE + DISP_BGND_BLACK);
}

/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static void TaskStartCreateTasks(void)
{
	CPU_INT08U  i;
	OS_ERR      err;


	for(i=0; i < N_TASKS; i++)                             /* Create N_TASKS identical tasks           */
	{
		TaskRandomData[i] = '0' + i;                       /* Each task will display its own letter    */

        OSTaskCreate((OS_TCB     *)&TaskRandomTcb[i],                                           
                     (CPU_CHAR   *)"Task Random",
                     (OS_TASK_PTR )TaskRandom, 
                     (void       *)&TaskRandomData[i],
                     (OS_PRIO     )(TASK_RANDOM_PRIO),     /* Notice ...                               */
                     (CPU_STK    *)&TaskRandomStk[i][0],
                     (CPU_STK_SIZE)TASK_STK_SIZE/10,
                     (CPU_STK_SIZE)TASK_STK_SIZE,
                     (OS_MSG_QTY  )0,
                     (OS_TICK     )5,
                     (void       *)0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);
	}

                                                        /* #define  OS_CFG_SCHED_ROUND_ROBIN_EN     1u */
	OSSchedRoundRobinCfg(DEF_ENABLED, 10, &err);        /* CONFIGURE ROUND-ROBIN SCHEDULING PARAMETERS */
}

/*
*********************************************************************************************************
*                                             TASKRANDOMS
*********************************************************************************************************
*/

static void TaskRandom(void *p_arg)
{
	CPU_INT16S  x,y;
	OS_ERR      err;
	CPU_TS      ts;
	CPU_CHAR    num;
	

	num = *(CPU_CHAR *)p_arg;

	while(1)
	{
		OSSemPend((OS_SEM *)&RandomSem,
		          (OS_TICK )0,
		          (OS_OPT  )OS_OPT_PEND_BLOCKING,
		          (CPU_TS *)&ts,
		          (OS_ERR *)&err);                /* Acquire semaphore to perform random numbers       */

		x = rand()%80;                            /* Find X position where task number will appear     */
		y = rand()%16;                            /* Find Y position where task number will appear     */

		OSSemPost((OS_SEM *)&RandomSem,     
		          (OS_OPT  )OS_OPT_POST_1,
		          (OS_ERR *)&err);                /* Release semaphore                                 */

		PC_DispChar(x, y + 4, num, DISP_FGND_WHITE + DISP_BGND_BLACK);

		OSSchedRoundRobinYield(&err);             /* Give up the CPU                                   */
	}
}
