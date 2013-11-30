#include <ucos_ii.h>
/*$PAGE*/
/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

  INT32U            OSCtxSwCtr;               /* Counter of number of context switches           */

#if (OS_EVENT_EN > 0) && (OS_MAX_EVENTS > 0)
  OS_EVENT         *OSEventFreeList;          /* Pointer to list of free EVENT control blocks    */
  OS_EVENT          OSEventTbl[OS_MAX_EVENTS];/* Table of EVENT control blocks                   */
#endif

#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)
  OS_FLAG_GRP       OSFlagTbl[OS_MAX_FLAGS];  /* Table containing event flag groups              */
  OS_FLAG_GRP      *OSFlagFreeList;           /* Pointer to free list of event flag groups       */
#endif

#if OS_TASK_STAT_EN > 0
  INT8S             OSCPUUsage;               /* Percentage of CPU used                          */
  INT32U            OSIdleCtrMax;             /* Max. value that idle ctr can take in 1 sec.     */
  INT32U            OSIdleCtrRun;             /* Val. reached by idle ctr at run time in 1 sec.  */
  BOOLEAN           OSStatRdy;                /* Flag indicating that the statistic task is rdy  */
  OS_STK            OSTaskStatStk[OS_TASK_STAT_STK_SIZE];      /* Statistics task stack          */
#endif

  INT8U             OSIntNesting;             /* Interrupt nesting level                         */
  INT8U             OSIntExitY;

  INT8U             OSLockNesting;            /* Multitasking lock nesting level                 */

  INT8U             OSPrioCur;                /* Priority of current task                        */
  INT8U             OSPrioHighRdy;            /* Priority of highest priority task               */

  INT8U             OSRdyGrp;                        /* Ready list group                         */
  INT8U             OSRdyTbl[OS_RDY_TBL_SIZE];       /* Table of tasks which are ready to run    */

  BOOLEAN           OSRunning;                       /* Flag indicating that kernel is running   */

  INT8U             OSTaskCtr;                       /* Number of tasks created                  */

  INT32U            OSIdleCtr;                                 /* Idle counter                   */

  OS_STK            OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];      /* Idle task stack                */


  OS_TCB           *OSTCBCur;                        /* Pointer to currently running TCB         */
  OS_TCB           *OSTCBFreeList;                   /* Pointer to list of free TCBs             */
  OS_TCB           *OSTCBHighRdy;                    /* Pointer to highest priority TCB R-to-R   */
  OS_TCB           *OSTCBList;                       /* Pointer to doubly linked list of TCBs    */
  OS_TCB           *OSTCBPrioTbl[OS_LOWEST_PRIO + 1];/* Table of pointers to created TCBs        */
  OS_TCB            OSTCBTbl[OS_MAX_TASKS + OS_N_SYS_TASKS];   /* Table of TCBs                  */

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)
  OS_MEM           *OSMemFreeList;            /* Pointer to free list of memory partitions       */
  OS_MEM            OSMemTbl[OS_MAX_MEM_PART];/* Storage for memory partition manager            */
#endif

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0)
  OS_Q             *OSQFreeList;              /* Pointer to list of free QUEUE control blocks    */
  OS_Q              OSQTbl[OS_MAX_QS];        /* Table of QUEUE control blocks                   */
#endif

#if OS_TIME_GET_SET_EN > 0   
  volatile  INT32U  OSTime;                   /* Current value of system time (in ticks)         */
#endif
