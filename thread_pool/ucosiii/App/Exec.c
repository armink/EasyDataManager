/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : Exec.c
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

#define  TASK_STK_SIZE          2048 * 10             /* Size of each task's stacks                    */

#define  TASK_CLK_ID                    0             /* Application tasks                             */
#define  TASK_1_ID                      1
#define  TASK_2_ID                      2
#define  TASK_3_ID                      3
#define  TASK_4_ID                      4
#define  TASK_5_ID                      5

#define  TASK_START_PRIO			   20             /* Application tasks priorities                  */
#define  TASK_CLK_PRIO				   21
#define  TASK_1_PRIO				   22
#define  TASK_2_PRIO				   23
#define  TASK_3_PRIO				   24
#define  TASK_4_PRIO				   25
#define  TASK_5_PRIO				   26

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef struct {
    CPU_CHAR    TaskName[30];
    CPU_INT16U  TaskCtr;
    CPU_INT32U  TaskExecTime;
    CPU_INT32U  TaskTotExecTime;
} TASK_USER_DATA;

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

TASK_USER_DATA TaskUserData[6];

OS_Q           Buf_Q;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

		void  Exec(void);
		void  DispTaskExecInfo(void);
		void  UpdateTaskExecInfo(void);
static  void  TaskStart(void *p_arg);
static  void  TaskClk(void *p_arg);
static  void  Task1(void *p_arg);
static  void  Task2(void *p_arg);
static  void  Task3(void *p_arg);
static  void  Task4(void *p_arg);
static  void  Task5(void *p_arg);
static  void  TaskUserDataInit(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartCreateTasks (void);
static  void  DispTaskStat(CPU_INT16S id);

/*
*********************************************************************************************************
*                                                Stack
*********************************************************************************************************
*/

void Exec(void)
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
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
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

	OSQCreate((OS_Q     *)&Buf_Q, 
			  (CPU_CHAR *)"Buf Q", 
			  (OS_MSG_QTY)30, 
			  (OS_ERR   *)&err);

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit((OS_ERR *)&err);
#endif

	TaskUserDataInit();

	TaskStartDispInit();

	TaskStartCreateTasks();

	OSTaskDel(&TaskStartTcb, &err);
}

/*
*********************************************************************************************************
*                                      INITIALIZE THE TASKUSERDATA
*********************************************************************************************************
*/

static void TaskUserDataInit(void)
{
	CPU_INT08U  i,j;


	for(i = 0; i < 6; i++)
	{
		for(j = 0; j < 30; j++)
		{
			TaskUserData[i].TaskName[j] = '\0';
		}

		TaskUserData[i].TaskCtr         = 0;
		TaskUserData[i].TaskExecTime    = 0;
		TaskUserData[i].TaskTotExecTime = 0;
	}
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
    PC_DispStr(0, 7,  "Task Name         Counter  Exec.Time(uS)   Tot.Exec.Time(uS)  %Tot.             ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr(0, 8,  "----------------- -------  -------------   -----------------  -----             ", DISP_FGND_WHITE + DISP_BGND_BLACK);
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


	strcpy(TaskUserData[TASK_CLK_ID].TaskName, "Clock Task");
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

	strcpy(TaskUserData[TASK_1_ID].TaskName, "MsgQ Rx Task");
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

	strcpy(TaskUserData[TASK_2_ID].TaskName, "MsgQ Tx Task #2");
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

	strcpy(TaskUserData[TASK_3_ID].TaskName, "MsgQ Tx Task #3");
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

	strcpy(TaskUserData[TASK_4_ID].TaskName, "MsgQ Tx Task #4");
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

	strcpy(TaskUserData[TASK_5_ID].TaskName, "TimeDlyTask");
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
	OS_ERR      err;


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
	OS_ERR       err;
	CPU_CHAR    *msg;
	CPU_TS       ts;
	OS_MSG_SIZE  msg_size;


	p_arg = p_arg;

	while(1)
	{
		msg = (CPU_CHAR *)OSQPend((OS_Q        *)&Buf_Q, 
			                      (OS_TICK      )0, 
			                      (OS_OPT       )OS_OPT_PEND_NON_BLOCKING,
	                              (OS_MSG_SIZE *)&msg_size,
				                  (CPU_TS      *)&ts,
				                  (OS_ERR      *)&err);

		if(msg != NULL)
			PC_DispStr(70, 11, msg, DISP_FGND_WHITE + DISP_BGND_BLACK);

		OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task2(void *p_arg)
{
	OS_ERR    err;
	CPU_CHAR  msg[20];


	p_arg = p_arg;
	strcpy(&msg[0], "Task 2");

	while(1)
	{
		OSQPost((OS_Q      *)&Buf_Q, 
			    (void      *)&msg[0], 
				(OS_MSG_SIZE)sizeof(CPU_CHAR),
                (OS_OPT     )OS_OPT_POST_FIFO,
				(OS_ERR    *)&err);

		OSTimeDlyHMSM(0, 0, 0, 800, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task3(void *p_arg)
{
	OS_ERR    err;
	CPU_CHAR  msg[20];


	p_arg = p_arg;
	strcpy(&msg[0], "Task 3");

	while(1)
	{
		OSQPost((OS_Q      *)&Buf_Q, 
			    (void      *)&msg[0], 
				(OS_MSG_SIZE)sizeof(CPU_CHAR),
                (OS_OPT     )OS_OPT_POST_LIFO,
				(OS_ERR    *)&err);

		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task4(void *p_arg)
{
	OS_ERR    err;
	CPU_CHAR  msg[20];


	p_arg = p_arg;
	strcpy(&msg[0], "Task 4");

	while(1)
	{
		OSQPost((OS_Q      *)&Buf_Q, 
			    (void      *)&msg[0], 
				(OS_MSG_SIZE)sizeof(CPU_CHAR),
                (OS_OPT     )OS_OPT_POST_FIFO,
				(OS_ERR    *)&err);

		OSTimeDlyHMSM(0, 0, 0, 300, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}

static void Task5(void *p_arg)
{
	OS_ERR  err;


	p_arg = p_arg;

	while(1)
	{
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_DLY, (OS_ERR *)&err);
	}
}


static void DispTaskStat(CPU_INT16S id)
{
    CPU_CHAR  str[80];


    sprintf(str, "%-18s %05u      %5u          %10ld",
            TaskUserData[id].TaskName,
            TaskUserData[id].TaskCtr,
            TaskUserData[id].TaskExecTime,
            TaskUserData[id].TaskTotExecTime);

    PC_DispStr(0, id + 10, str, DISP_FGND_WHITE + DISP_BGND_BLACK);
}


void DispTaskExecInfo(void)
{
    CPU_CHAR    str[80];
    CPU_INT16S  i;
    CPU_INT32U  total;
    CPU_INT08U  pct;


    total = 0L;                                          /* Totalize TOT. EXEC. TIME for each task */
    for (i = 0; i < 6; i++) {
        total += TaskUserData[i].TaskTotExecTime;
        DispTaskStat(i);                                 /* Display task data                      */
    }
    if (total > 0) {
        for (i = 0; i < 6; i++) {                        /* Derive percentage of each task         */
            pct = (CPU_INT08U)(100 * TaskUserData[i].TaskTotExecTime / total);
            sprintf(str, "%3d %%", pct);
            PC_DispStr(62, i + 10, str, DISP_FGND_WHITE + DISP_BGND_BLACK);
        }
    }
    if (total > 1000000000L) {                           /* Reset total time counters at 1 billion */
        for (i = 0; i < 6; i++) {
            TaskUserData[i].TaskTotExecTime = 0L;
        }
    }
}


void UpdateTaskExecInfo(void)
{
	CPU_INT16S  id;
    CPU_INT32U  time;


    time = PC_ElapsedStop(1);                      /* This task is done                                  */
    PC_ElapsedStart(1);                            /* Start for next task                                */
  
	switch(OSTCBCurPtr->Prio)
	{
	case TASK_CLK_PRIO:
		id = TASK_CLK_ID;
		break;

	case TASK_1_PRIO:
		id = TASK_1_ID;
		break;

	case TASK_2_PRIO:
		id = TASK_2_ID;
		break;

	case TASK_3_PRIO:
		id = TASK_3_ID;
		break;

	case TASK_4_PRIO:
		id = TASK_4_ID;
		break;

	case TASK_5_PRIO:
		id = TASK_5_ID;
		break;

	default:
		break;
	}

	if(id == TASK_CLK_ID   ||
	   id == TASK_1_ID     ||
	   id == TASK_2_ID     ||
	   id == TASK_3_ID     ||
	   id == TASK_4_ID     ||
	   id == TASK_5_ID)
	{
		TaskUserData[id].TaskCtr++;                /* Increment task counter                             */
		TaskUserData[id].TaskExecTime = time;      /* Update the task's execution time                   */
		TaskUserData[id].TaskTotExecTime += time;  /* Update the task's total execution time             */
	}
}


