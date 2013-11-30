/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : Stack.c
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

#define  TASK_STK_SIZE          2048 * 10             /* Size of each task's stacks                    */

#define  TASK_START_PRIO			   20             /* Application tasks priorities                  */
#define  TASK_CLK_PRIO				   21
#define  TASK_1_PRIO				   22
#define  TASK_2_PRIO				   23
#define  TASK_3_PRIO				   24
#define  TASK_4_PRIO				   25
#define  TASK_5_PRIO				   26

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

CPU_STK        TaskStartStk[TASK_STK_SIZE];           /* Tasks stacks                                  */
CPU_STK        TaskClkStk[TASK_STK_SIZE];
CPU_STK        Task1Stk[TASK_STK_SIZE];
CPU_STK        Task2Stk[TASK_STK_SIZE];
CPU_STK        Task3Stk[TASK_STK_SIZE];
CPU_STK        Task4Stk[TASK_STK_SIZE];
CPU_STK        Task5Stk[TASK_STK_SIZE];

OS_TCB 	       TaskStartTcb;                          /* Tasks TCBs                                    */
OS_TCB         TaskClkTcb;
OS_TCB         Task1Tcb;
OS_TCB         Task2Tcb;
OS_TCB         Task3Tcb;
OS_TCB         Task4Tcb;
OS_TCB         Task5Tcb;

OS_TCB         TaskTcbs[7];

OS_Q           Buf_Q;
OS_SEM         StackSem;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

		void  Stack(void);
static  void  TaskStart(void *p_arg);
static  void  TaskClk(void *p_arg);
static  void  Task1(void *p_arg);
static  void  Task2(void *p_arg);
static  void  Task3(void *p_arg);
static  void  Task4(void *p_arg);
static  void  Task5(void *p_arg);
static  void  TaskTcbsInit(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartCreateTasks (void);

/*
*********************************************************************************************************
*                                                Stack
*********************************************************************************************************
*/

void Stack(void)
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
	CPU_INT08U  Cnt;
	CPU_CHAR    str[30];
    OS_ERR      err;


	/* OS_TICKS_PER_SEC = 200, 5ms/tick */
	timeSetEvent(5, 0, OSTickISR, 0, TIME_PERIODIC);       /* Must be first                            */

	p_arg = p_arg;
	Cnt = 0;

	OSQCreate((OS_Q     *)&Buf_Q, 
			  (CPU_CHAR *)"Buf Q", 
			  (OS_MSG_QTY)30, 
			  (OS_ERR   *)&err);

	OSSemCreate((OS_SEM   *)&StackSem,
                (CPU_CHAR *)"Stack Sem",
                (OS_SEM_CTR)1,
                (OS_ERR   *)&err);

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit((OS_ERR *)&err);
#endif

	TaskStartDispInit();

	TaskStartCreateTasks();

	TaskTcbsInit();

	while(1)
	{
		Cnt++;
		Cnt = Cnt%10;

		sprintf(str, "%d", Cnt);
		PC_DispStr(70, 10, str, DISP_FGND_WHITE + DISP_BGND_BLACK);

		OSTimeDly(30, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

/*
*********************************************************************************************************
*                                             TASKTCBSINIT
*********************************************************************************************************
*/

static void TaskTcbsInit(void)
{
	TaskTcbs[0] = TaskStartTcb;
	TaskTcbs[1] = TaskClkTcb;
	TaskTcbs[2] = Task1Tcb;
	TaskTcbs[3] = Task2Tcb;
	TaskTcbs[4] = Task3Tcb;
	TaskTcbs[5] = Task4Tcb;
	TaskTcbs[6] = Task5Tcb;
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
	PC_DispStr(0, 7,  "Task           Total Stack  Free Stack  Used Stack  ExecTime (uS)               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 8,  "-------------  -----------  ----------  ----------  -------------               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 10, "TaskStart():                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 11, "TaskClk()  :                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 12, "Task1()    :                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 13, "Task2()    :                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 14, "Task3()    :                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 15, "Task4()    :                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 16, "Task5()    :                                                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 22, "                                                              591881218@qq.com  ", DISP_FGND_WHITE + DISP_BGND_BLACK);
}

/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static void TaskStartCreateTasks(void)
{
	OS_ERR  err;


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

    OSTaskCreate((OS_TCB     *)&Task1Tcb,
                 (CPU_CHAR   *)"Task1",
                 (OS_TASK_PTR )Task1, 
                 (void       *)0,
                 (OS_PRIO     )TASK_1_PRIO,
                 (CPU_STK    *)&Task1Stk[0],
                 (CPU_STK_SIZE)TASK_STK_SIZE/10,
                 (CPU_STK_SIZE)TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err); 

    OSTaskCreate((OS_TCB     *)&Task2Tcb,
                 (CPU_CHAR   *)"Task2",
                 (OS_TASK_PTR )Task2, 
                 (void       *)0,
                 (OS_PRIO     )TASK_2_PRIO,
                 (CPU_STK    *)&Task2Stk[0],
                 (CPU_STK_SIZE)TASK_STK_SIZE/10,
                 (CPU_STK_SIZE)TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err); 

    OSTaskCreate((OS_TCB     *)&Task3Tcb,
                 (CPU_CHAR   *)"Task3",
                 (OS_TASK_PTR )Task3, 
                 (void       *)0,
                 (OS_PRIO     )TASK_3_PRIO,
                 (CPU_STK    *)&Task3Stk[0],
                 (CPU_STK_SIZE)TASK_STK_SIZE/10,
                 (CPU_STK_SIZE)TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err); 

    OSTaskCreate((OS_TCB     *)&Task4Tcb,
                 (CPU_CHAR   *)"Task4",
                 (OS_TASK_PTR )Task4, 
                 (void       *)0,
                 (OS_PRIO     )TASK_4_PRIO,
                 (CPU_STK    *)&Task4Stk[0],
                 (CPU_STK_SIZE)TASK_STK_SIZE/10,
                 (CPU_STK_SIZE)TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err); 

    OSTaskCreate((OS_TCB     *)&Task5Tcb,
                 (CPU_CHAR   *)"Task5",
                 (OS_TASK_PTR )Task5, 
                 (void       *)0,
                 (OS_PRIO     )TASK_5_PRIO,
                 (CPU_STK    *)&Task5Stk[0],
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
*                                                  TASKS
*********************************************************************************************************
*/

static void Task1(void *p_arg)
{
	OS_ERR        err;
    CPU_STK_SIZE  FreeStk;
    CPU_STK_SIZE  UsedStk;
	CPU_INT32U    time;                    /* Execution time (in uS)                                  */
	CPU_INT16S    i;
	CPU_CHAR      str[80];


	p_arg = p_arg;

	while(1)
	{
		for (i = 0; i < 7; i++) {
			PC_ElapsedStart(1);

			OSTaskStkChk((OS_TCB       *)&TaskTcbs[i],
				         (CPU_STK_SIZE *)&FreeStk,
					     (CPU_STK_SIZE *)&UsedStk,
					     (OS_ERR       *)&err);

			time = PC_ElapsedStop(1);

			if (err == OS_ERR_NONE) {
				sprintf(str, "%4ld        %4ld        %4ld        %6d",
						TaskTcbs[i].StkSize,
						FreeStk,
						UsedStk,
						time);

				PC_DispStr(19, 10 + i, str, DISP_FGND_WHITE + DISP_BGND_BLACK);
			}
		}
		OSTimeDly(50, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task2(void *p_arg)
{
	OS_ERR  err;


	p_arg = p_arg;

	while(1)
	{
        PC_DispChar(70, 13, '|',  DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(150, OS_OPT_TIME_DLY, (OS_ERR *)&err);
        PC_DispChar(70, 13, '/',  DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(150, OS_OPT_TIME_DLY, (OS_ERR *)&err);
        PC_DispChar(70, 13, '-',  DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(150, OS_OPT_TIME_DLY, (OS_ERR *)&err);
        PC_DispChar(70, 13, '\\', DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(150, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task3(void *p_arg)
{
	CPU_INT32U  i;
	CPU_INT32U  dummy[2048];
	OS_ERR      err;


    p_arg = p_arg;

	for (i = 0; i < 2048; i++) {       /* Use up the stack with 'junk'                                 */
		dummy[i] = '?';
	}

	while(1)
	{
		PC_DispChar(70, 14, '|',  DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(25, OS_OPT_TIME_DLY, (OS_ERR *)&err);
        PC_DispChar(70, 14, '\\', DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(25, OS_OPT_TIME_DLY, (OS_ERR *)&err);
        PC_DispChar(70, 14, '-',  DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(25, OS_OPT_TIME_DLY, (OS_ERR *)&err);
        PC_DispChar(70, 14, '/',  DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDly(25, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task4(void *p_arg)
{
	CPU_CHAR  txmsg;
	OS_ERR      err;


	p_arg = p_arg;
	txmsg = 'A';

	while(1)
	{
		OSQPost((OS_Q      *)&Buf_Q, 
			    (void      *)&txmsg, 
				(OS_MSG_SIZE)sizeof(CPU_CHAR),
                (OS_OPT     )OS_OPT_POST_FIFO,
				(OS_ERR    *)&err);

        txmsg++;                                 /* Next message to send                               */
        if (txmsg > 'Z') {
            txmsg = 'A';                         /* Start new series of messages                       */
        }

		OSSemPend((OS_SEM *)&StackSem,
		          (OS_TICK )0,
		          (OS_OPT  )OS_OPT_PEND_BLOCKING,
		          (CPU_TS  )0,
		          (OS_ERR *)&err); 
	}
}

static void Task5(void *p_arg)
{
	CPU_CHAR  *rxmsg;
	OS_ERR       err;
	CPU_TS       ts;
	OS_MSG_SIZE  msg_size;


	p_arg = p_arg;

	while(1)
	{
		rxmsg = (CPU_CHAR *)OSQPend((OS_Q        *)&Buf_Q, 
			                        (OS_TICK      )0, 
			                        (OS_OPT       )OS_OPT_PEND_BLOCKING,
	                                (OS_MSG_SIZE *)&msg_size,
				                    (CPU_TS      *)&ts,
				                    (OS_ERR      *)&err);

        PC_DispChar(70, 16, *rxmsg, DISP_FGND_WHITE + DISP_BGND_BLACK);

		OSSemPost((OS_SEM *)&StackSem,     
		          (OS_OPT  )OS_OPT_POST_1,
		          (OS_ERR *)&err);

        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, (OS_ERR *)&err);  /* Wait one second               */
	}
}



