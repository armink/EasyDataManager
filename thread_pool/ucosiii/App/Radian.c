/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : Radian.c
* Modify  : ЛЊаж Email:591881218@qq.com
************************************************************************************************************************
*/

#include <os.h>
#include <pc.h>
// ------------------------------
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
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
#define  TASK_CLK_PRIO				   23
#define  TASK_RADIAN_PRIO			   26

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

CPU_STK        TaskStartStk[TASK_STK_SIZE];      /* Tasks stacks                                       */
CPU_STK        TaskClkStk[TASK_STK_SIZE];
CPU_STK        TaskRadianStk[N_TASKS][TASK_STK_SIZE]; 

OS_TCB 	       TaskStartTcb;                     /* Tasks TCBs                                         */
OS_TCB         TaskClkTcb;
OS_TCB         TaskRadianTcb[N_TASKS];

CPU_INT08U     TaskRadianData[N_TASKS];          /* Parameters to pass to each task                    */

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

		void  Radian(void);
static  void  TaskStart(void *p_arg);
static  void  TaskClk(void *p_arg);
static  void  TaskRadian(void *p_arg);
static  void  TaskStartDispInit(void);
static  void  TaskStartCreateTasks (void);

/*
*********************************************************************************************************
*                                               Radian
*********************************************************************************************************
*/

void Radian(void)
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
	PC_DispStr(0, 5,  "TaskPrio      Angle   cos(Angle)   sin(Angle)                                   ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 6,  "--------      -----   ----------   ----------                                   ", DISP_FGND_WHITE + DISP_BGND_BLACK);
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


    OSTaskCreate((OS_TCB     *)&TaskClkTcb,
                 (CPU_CHAR   *)"Task Clk",
                 (OS_TASK_PTR )TaskClk, 
                 (void       *)0,
                 (OS_PRIO     )TASK_CLK_PRIO,
                 (CPU_STK    *)&TaskClkStk[0],
                 (CPU_STK_SIZE)TASK_STK_SIZE/10,
                 (CPU_STK_SIZE)TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err); 

	for(i=0; i < N_TASKS; i++)
	{
		TaskRadianData[i] = i + 1;

        OSTaskCreate((OS_TCB     *)&TaskRadianTcb[i],                                           
                     (CPU_CHAR   *)"Task Radian",
                     (OS_TASK_PTR )TaskRadian, 
                     (void       *)&TaskRadianData[i],
                     (OS_PRIO     )(i + TASK_RADIAN_PRIO),
                     (CPU_STK    *)&TaskRadianStk[i][0],
                     (CPU_STK_SIZE)TASK_STK_SIZE/10,
                     (CPU_STK_SIZE)TASK_STK_SIZE,
                     (OS_MSG_QTY  )0,
                     (OS_TICK     )0,
                     (void       *)0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                     (OS_ERR     *)&err);
	}
}

/*
*********************************************************************************************************
*                                               CLOCK TASK
*********************************************************************************************************
*/

static void TaskClk(void *p_arg)
{
    CPU_CHAR  str[40];
	OS_ERR    err;


    p_arg = p_arg;

    while(1) 
	{
        PC_GetDateTime(str);
        PC_DispStr(60, 20, str, DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_DLY, (OS_ERR *)&err);
    }
}

/*
*********************************************************************************************************
*                                             TASKRADIANS
*********************************************************************************************************
*/

static void TaskRadian(void *p_arg)
{
    FP64        x;
    FP64        y;
    FP64        angle;
    FP64        radians;
    CPU_CHAR    str[81];
    CPU_INT16S  ypos;
    CPU_INT08U  taskPrio;
	CPU_INT08U  dly;
	OS_ERR      err;
	CPU_SR_ALLOC();


    taskPrio = *(CPU_INT08U *)p_arg + TASK_RADIAN_PRIO - 1;
    ypos = (CPU_INT16S)(*(CPU_INT08U *)p_arg + 7);

	CPU_CRITICAL_ENTER();
    angle = (FP64)(*(CPU_INT08U *)p_arg) * (FP64)36.0;
	CPU_CRITICAL_EXIT();

    while(1) 
	{
		CPU_CRITICAL_ENTER();
        radians = (FP64)2.0 * (FP64)3.141592 * angle / (FP64)360.0;
        x       = cos(radians);
        y       = sin(radians);

        sprintf(str, "   %2d       %8.3f  %8.3f     %8.3f", taskPrio, angle, x, y);
        PC_DispStr(0, ypos, str, DISP_FGND_WHITE + DISP_BGND_BLACK);

        if (angle >= (FP64)360.0) {
            angle  =   (FP64)0.0;
        } else {
            angle +=   (FP64)0.01;
        }
		CPU_CRITICAL_EXIT();

		CPU_CRITICAL_ENTER();
		dly = rand()%100 + 1;
		CPU_CRITICAL_EXIT();
		
        OSTimeDly(dly, OS_OPT_TIME_DLY, (OS_ERR *)&err);  /* Delay XXX clock tick                         */
    }
}
