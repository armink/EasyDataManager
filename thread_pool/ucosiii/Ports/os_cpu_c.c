/*
*********************************************************************************************************
*                                                uC/OS-III
*                                          The Real-Time Kernel
*
*
*                           (c) Copyright 2009-2010; Micrium, Inc.; Weston, FL
*                    All rights reserved.  Protected by international copyright laws.
*
*                                        80x86/80x88 Specific code
*                                          PROTECTED MEMORY MODEL
*
*                                                VC++ 6.0
*
* File        : OS_CPU_C.C
* By          : Jean J. Labrosse
*
* LICENSING TERMS:
* ---------------
*             uC/OS-III is provided in source form to registered licensees ONLY.  It is
*             illegal to distribute this source code to any third party unless you receive
*             written permission by an authorized Micrium representative.  Knowledge of
*             the source code may NOT be used to develop a similar product.
*
*             Please help us continue to provide the Embedded community with the finest
*             software available.  Your honesty is greatly appreciated.
*
*             You can contact us at www.micrium.com.
*
* For         : 80x86/80x88
* Toolchain   : VC++6.0
* From        : 文佳 Email:ganganwen@163.com & Lingjun Chen(jorya_txj)(www.raw-os.com)
* Modify      : 华兄 Email:591881218@qq.com
*********************************************************************************************************
*/

#if 1
#define   OS_CPU_GLOBALS
#endif

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_cpu_c__c = "$Id: $";
#endif

#include <os.h>
#include <system.h>
#include <stdio.h>
#include <windows.h>


static HANDLE MainHandle;	//主线程句柄
static CONTEXT Context;	    //主线程上下文
CPU_SR FlagEn = DISABLE;	//是否执行时钟调度的标志

#if (OS_MSG_TRACE > 0)
static int OSPrintf(char  *str, ...);
#endif


void VCInit(void)
{
	HANDLE cp,ct;


	cp = GetCurrentProcess();	//得到当前进程句柄
	ct = GetCurrentThread();	//得到当前线程伪句柄

	SetProcessAffinityMask(cp, 1L); //Select the first CPU

#if  FP_SAVE_RESTORE_EN > 0u
	Context.ContextFlags = CONTEXT_FULL; //Don't need to save CONTEXT_FLOATING_POINT
#else
	Context.ContextFlags = (CONTEXT_FULL | CONTEXT_FLOATING_POINT); //Important
#endif

	DuplicateHandle(cp, ct, cp, &MainHandle, 0, TRUE, 2);	//伪句柄转换，得到线程真句柄
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          OSPrintf()
*
* Description: This function is analog of printf.
*
* Arguments  : str      is a pointer to format string output.
*********************************************************************************************************
*/

#if (OS_MSG_TRACE > 0)
static int OSPrintf(char  *str, ...)
{
    va_list  param;
    int      ret;
	CPU_SR_ALLOC();


	CPU_CRITICAL_ENTER();
    va_start(param, str);
    ret = vprintf(str, param);
    va_end(param);
	CPU_CRITICAL_EXIT();

    return (ret);
}
#endif


/*$PAGE*/
/*
**********************************************************************************************************
*                                       INITIALIZE A TASK'S STACK
*
* Description: This function is called by OS_Task_Create() or OSTaskCreateExt() to initialize the stack
*              frame of the task being created. This function is highly processor specific.
*
* Arguments  : p_task       Pointer to the task entry point address.
*
*              p_arg        Pointer to a user supplied data area that will be passed to the task
*                               when the task first executes.
*
*              p_stk_base   Pointer to the base address of the stack.
*
*              stk_size     Size of the stack, in number of CPU_STK elements.
*
*              opt          Options used to alter the behavior of OS_Task_StkInit().
*                            (see OS.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when task starts executing.
*
*              2) All tasks run in Thread mode, using process stack.
**********************************************************************************************************
*/

CPU_STK  *OSTaskStkInit (OS_TASK_PTR    p_task,
                         void          *p_arg,
                         CPU_STK       *p_stk_base,
                         CPU_STK       *p_stk_limit,
                         CPU_STK_SIZE   stk_size,
                         OS_OPT         opt)
{
	CPU_INT32U  *stk;								/* console 下寄存器为32位宽									 */
	CPU_INT32U   ptos = (CPU_INT32U)(p_stk_base + stk_size - 2);


    opt    = opt;									/* 'opt' is not used, prevent warning						 */
    stk    = (CPU_INT32U *)ptos;					/* Load stack pointer										 */
    *--stk = (CPU_INT32U)p_arg;					    /* Simulate call to function with argument					 */
    *--stk = (CPU_INT32U)0x00000000;				/*子程序是从当前esp＋4处取得传入的参数，所以此处要空出4个字节*/
    *--stk = (CPU_INT32U)p_task;					/* Put pointer to task   on top of stack					 */
    *--stk = (CPU_INT32U)0x00000202;				/* EFL = 0X00000202											 */
    *--stk = (CPU_INT32U)0xAAAAAAAA;                /* EAX = 0xAAAAAAAA											 */
    *--stk = (CPU_INT32U)0xCCCCCCCC;                /* ECX = 0xCCCCCCCC											 */
    *--stk = (CPU_INT32U)0xDDDDDDDD;                /* EDX = 0xDDDDDDDD											 */
    *--stk = (CPU_INT32U)0xBBBBBBBB;                /* EBX = 0xBBBBBBBB											 */
    *--stk = (CPU_INT32U)0x00000000;                /* ESP = 0x00000000							                 */
    *--stk = (CPU_INT32U)0x11111111;                /* EBP = 0x11111111											 */
    *--stk = (CPU_INT32U)0x22222222;                /* ESI = 0x22222222											 */
    *--stk = (CPU_INT32U)0x33333333;                /* EDI = 0x33333333											 */

	if(opt & OS_OPT_TASK_SAVE_FP)                   /* 创建任务时通过设置Opt选择使用浮点栈                       */
	{
		stk -= FP_STK_SIZE;

		asm("\
			push %eax;\
			movl %eax,stk;\
			fsave eax;\
			pop %eax;\
			");
	}
                             
    return ((CPU_STK *)stk);
}


/*$PAGE*/
/*
/**********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; The stack frame is assumed to look as follows:
;
; OSTCBHighRdy->OSTCBStkPtr --> EDI                               (Low memory)                           
;                               ESI
;                               EBP
;                               ESP
;                               EBX
;                               EDX
;                               ECX
;                               EAX
;                               Flags to load in PSW
;                               OFFSET  of task code address
;								4 empty byte
;                               OFFSET  of 'pdata'				  (High memory)
;                                              
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;**********************************************************************************************************
*/

void OSStartHighRdy(void)
{
	OSTaskSwHook();
	OSRunning = TRUE;

	asm("\
		movl %ebx,OSTCBCurPtr;\
		movl %esp,%ebx;\
		");

	if(OSTCBCurPtr->Opt & OS_OPT_TASK_SAVE_FP)
	{
		asm("\
			frstor esp;\
			addl %esp, FP_BYTES;\
			");
	}

	asm("\
		pop ad;\
		pop fd;\
		ret;\
		");
}


/*$PAGE*/
/*
/*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;                                           void OSCtxSw(void)
;
; Note(s): 1) 此函数为手动任务切换函数
;
;          2) The stack frame of the task to suspend looks as follows:
;
;                 SP -> OFFSET  of task to suspend    (Low memory)
;                       PSW     of task to suspend    (High memory)
;
;          3) The stack frame of the task to resume looks as follows:
;
; OSTCBHighRdy->OSTCBStkPtr --> EDI                               (Low memory)                           
;                               ESI
;                               EBP
;                               ESP
;                               EBX
;                               EDX
;                               ECX
;                               EAX
;                               Flags to load in PSW
;                               OFFSET  of task code address	  (High memory)
;                                    
;**********************************************************************************************************
*/

void OSCtxSw(void)
{
	asm("\
		lea NEXTSTART,%eax;\
		push %eax;\
		pushf;\
		pusha;\
		");

	if(OSTCBCurPtr->Opt & OS_OPT_TASK_SAVE_FP)
	{
		asm("\
			addl %esp, -FP_BYTES;\
			fsave esp;\
			");
	}

	asm("\
		movl %ebx,OSTCBCurPtr;\
		movl %ebx,%esp;\
		");

	OSTaskSwHook();
	OSTCBCurPtr = OSTCBHighRdyPtr;
	OSPrioCur   = OSPrioHighRdy;

	FlagEn = ENABLE;            //允许时钟中断

	asm("\
		movl %ebx, OSTCBCurPtr;\
		movl %esp, %ebx;\
		");

	if(OSTCBCurPtr->Opt & OS_OPT_TASK_SAVE_FP)
	{
		asm("\
			frstor esp;\
			addl %esp, FP_BYTES;\
			");
	}

	asm("\
		pop ad;\
		pop fd;\
		ret;\
		");

NEXTSTART:					    //任务切换回来的运行地址
	return;
}


/*$PAGE*/
/*
/*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From an ISR)
;                                        void OSIntCtxSw(void)
;
; Note(s): 1) Context保存了主线程所有的上下文，因此切换起来比较简单，先保存相应寄存器
			  然后把要运行的任务的上下文填入Context结构并保存，完成切换，堆栈内容如下：
;
; OSTCBHighRdy->OSTCBStkPtr --> EDI                               (Low memory)                           
;                               ESI
;                               EBP
;                               ESP
;                               EBX
;                               EDX
;                               ECX
;                               EAX
;                               Flags to load in PSW
;                               OFFSET  of task code address	  (High memory)
;***********************************************************************************************************
*/

void OSIntCtxSw(void)
{
	CPU_INT32U          *top;

#if  FP_SAVE_RESTORE_EN == 0u
	FLOATING_SAVE_AREA  *FloatSave;
#endif


	OSTaskSwHook();

	top       = (CPU_INT32U *)Context.Esp;				    //获得主线程(当前任务)堆栈指针

#if  FP_SAVE_RESTORE_EN == 0u
	FloatSave = (FLOATING_SAVE_AREA *)(&Context.FloatSave); //获取预先保存的浮点数现场
#endif

	//在任务栈中保存所有的寄存器
	*--top = Context.Eip;
	*--top = Context.EFlags;
	*--top = Context.Eax;
	*--top = Context.Ecx;
	*--top = Context.Edx;
	*--top = Context.Ebx;
	*--top = Context.Esp;									//错误的SP，没有意义
	*--top = Context.Ebp;
	*--top = Context.Esi;
	*--top = Context.Edi;

	if(OSTCBCurPtr->Opt & OS_OPT_TASK_SAVE_FP)
	{
		top -= FP_STK_SIZE;									//调整任务栈栈顶指针

#if  FP_SAVE_RESTORE_EN > 0u
		asm("\
			push %eax;\
			movl %eax,top;\
			fsave eax;\
			pop %eax;\
			");
#else
		memcpy(top, FloatSave, sizeof(FLOATING_SAVE_AREA)); //保存浮点数现场到任务栈
#endif
	}

	OSTCBCurPtr->StkPtr = (CPU_STK *)top;					//保存任务栈栈顶指针

	OSTCBCurPtr = OSTCBHighRdyPtr;							//获得当前就绪的最高优先级任务的TCB
	OSPrioCur   = OSPrioHighRdy;							//获得当前就绪的最高优先级任务的优先级

	top = (CPU_INT32U *)(OSTCBCurPtr->StkPtr);				//获得将要执行的任务的堆栈指针

	if(OSTCBCurPtr->Opt & OS_OPT_TASK_SAVE_FP)
	{
#if  FP_SAVE_RESTORE_EN > 0u
		asm("\
			push %eax;\
			movl %eax,top;\
			frstor eax;\
			pop %eax;\
			");
#else
		memcpy(FloatSave, top, sizeof(FLOATING_SAVE_AREA)); //从任务栈拷贝浮点数现场
                                                            //恢复浮点数现场
		Context.FloatSave = *(FLOATING_SAVE_AREA *)FloatSave;
#endif

		top += FP_STK_SIZE;									//调整任务栈栈顶指针
	}

	//从任务栈中恢复所有的寄存器
	Context.Edi    = *top++;
	Context.Esi    = *top++;
	Context.Ebp    = *top++;
	Context.Esp    = *top++;								//错误的SP，没有意义
	Context.Ebx    = *top++;
	Context.Edx    = *top++;
	Context.Ecx    = *top++;
	Context.Eax    = *top++;
	Context.EFlags = *top++;
	Context.Eip    = *top++;

	Context.Esp = (unsigned long)top;						//获得正确的esp

#if 0
	/*SetThreadContext(mainhandle, &Context);*/				//保存主线程上下文，不再使用
#endif
}


/*$PAGE*/
/*
/*********************************************************************************************************
;                                            HANDLE TICK ISR
;
; Description:  此函数为时钟调度关键程序，通过timeSetEvent函数来产生时钟节拍，timeSetEvent将产生一个
;				线程调用此函数，此函数将与主任务同步运行，因此并不是中断主程序来运行此函数的，但此函数
;               将模拟中断的产生，如果发现中断处于关闭状态，则退出
;
; Arguments  : none
;
; Returns    : none
;
; Note(s)    :  此函数与中断函数最大的不同是没有立即保存寄存器，而且不能用iret指令，所以OSIntCtxSw()实现
                也不一样，它是要返回调用函数的
;**********************************************************************************************************
*/

void CALLBACK OSTickISR(unsigned int a,unsigned int b,unsigned long c,unsigned long d,unsigned long e)
{
	BOOL  Value;


	if(!FlagEn)
	{
		return;								//如果当前中断被屏蔽则返回
	}

	Value = SuspendThread(MainHandle);	    //中止主线程的运行，模拟中断产生，但没有保存寄存器

	if(Value < 0)                           //挂起线程失败，返回
	{
		return;
	}

	if(!FlagEn)
	{										//在SuspendThread完成以前，FlagEn可能被再次改掉
		do {                                //模拟中断返回，主线程得以继续执行
			Value = ResumeThread(MainHandle);
		} while(Value > 0);

		return;								//如果当前中断被屏蔽则返回
	}

	GetThreadContext(MainHandle, &Context);	//获得主线程上下文，为切换任务做准备

	OSIntNestingCtr++;						//中断嵌套层数加1

	OSTimeTick();							//uC/OS内部时钟中断服务子程序
	OSIntExit();							//由于不能使用中断返回指令，所以此函数是要返回的

	SetThreadContext(MainHandle, &Context); //加载主线程的上下文

	do {
		Value = ResumeThread(MainHandle);   //模拟中断返回，主线程得以继续执行
	} while(Value > 0);
}


/*
*********************************************************************************************************
*                                           IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : None.
*
* Note(s)    : None.
*********************************************************************************************************
*/

void  OSIdleTaskHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u

#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : None.
*
* Note(s)    : None.
*********************************************************************************************************
*/

void  OSInitHook (void)
{

}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-III's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : None.
*
* Note(s)    : None.
*********************************************************************************************************
*/

void  OSStatTaskHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u

	if(ExecEn == TRUE)
		DispTaskExecInfo();

#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being created.
*
* Note(s)    : None.
*********************************************************************************************************
*/

void  OSTaskCreateHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u

#else
    (void)p_tcb;                                            /* Prevent compiler warning                               */
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being deleted.
*
* Note(s)    : None.
*********************************************************************************************************
*/

void  OSTaskDelHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u

#else
    (void)p_tcb;                                            /* Prevent compiler warning                               */
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : p_tcb        Pointer to the task control block of the task that is returning.
*
* Note(s)    : None.
*********************************************************************************************************
*/

void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
#if OS_CFG_APP_HOOKS_EN > 0u

#else
    (void)p_tcb;                                            /* Prevent compiler warning                               */
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : None.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdyPtr' points to the TCB of the task
*                 that will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCurPtr' points
*                 to the task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

void  OSTaskSwHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u

	if(ExecEn == TRUE)
		UpdateTaskExecInfo();

#else
    (void)p_tcb;                                            /* Prevent compiler warning                               */
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : None.
*
* Note(s)    : 1) This function is assumed to be called from the Tick ISR.
*********************************************************************************************************
*/

void  OSTimeTickHook (void)
{
#if OS_CFG_APP_HOOKS_EN > 0u

#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          SYS TICK HANDLER
*
* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/OS-II tick
*              interrupt.
*
* Arguments  : None.
*
* Note(s)    : 1) This function MUST be placed on entry 15 of the Cortex-M3 vector table.
*********************************************************************************************************
*/

void  OS_CPU_SysTickHandler (void)
{

}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         INITIALIZE SYS TICK
*
* Description: Initialize the SysTick.
*
* Arguments  : cnts         Number of SysTick counts between two OS tick interrupts.
*
* Note(s)    : 1) This function MUST be called after OSStart() & after processor initialization.
*********************************************************************************************************
*/

void  OS_CPU_SysTickInit (CPU_INT32U  cnts)
{

}
