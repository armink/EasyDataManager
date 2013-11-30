/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : main.c
* Create  : ЛЊаж Email:591881218@qq.com
************************************************************************************************************************
*/

#include <os.h>
#include <pc.h>
#include <system.h>

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  OK                      0

#define  RANDOM				  0x31
#define  RADIAN				  0x32
#define  STACK			      0x33
#define  EXEC                 0x34

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

CPU_BOOLEAN  ExecEn;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

extern  void  Random(void);
extern  void  Radian(void);
extern  void  Stack(void);
extern  void  Exec(void);

static  void  OSStartDispInit(void);

/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

int main(int argc, char *argv[])
{
	OS_ERR   err;
	CPU_INT  key;


	VCInit();	                                           /* Initialize VC                            */
	CPU_Init();                                            /* Initialize CPU                           */
	OSInit((OS_ERR *)&err);                                /* Initialize uC/OS-III                     */

	PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */
	PC_ElapsedInit();                                      /* Initialize elapsed time module           */

BACK:
    OSStartDispInit();

    while(!PC_GetKey(&key));                               /* Waiting for the key been pressed         */

	switch(key)
	{
	case RANDOM:
		ExecEn=FALSE;
		Random();
		break;

	case RADIAN:
		ExecEn=FALSE;
		Radian();
		break;

	case STACK:
		ExecEn=FALSE;
		Stack();
		break;

	case EXEC:
		ExecEn=TRUE;
		Exec();
		break;

	default: 
		PC_DispStr(2, 20, "You have input a wrong key, press any key to continue!                         ", DISP_FGND_WHITE + DISP_BGND_BLACK);
		PC_DispStr(0, 21, "                                                                               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
		system("pause");
		PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);  /* Clear the screen                         */
		goto BACK;
		break;
	}

    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

	OSStart((OS_ERR *)&err);                               /* Start multitasking                       */
	return OK;
}

/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static void OSStartDispInit(void)
{
	PC_DispStr(0, 0,  "*                                                                              ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 1,  "*******************************************************************************", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 2,  "*                                 uC/OS-III                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 3,  "*                           The Real-Time Kernel                               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 4,  "*                                                                              ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 5,  "*          (c) Copyright 2009-2011, Jean J. Labrosse, Weston, FL               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 6,  "*                           All Rights Reserved                                ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 7,  "*                                                                              ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 8,  "* Filename    : uCOS-III for VC.exe                                            ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 9,  "* Create      : ЛЊаж Email:591881218@qq.com                                    ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 10, "*******************************************************************************", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 11, "*                                                                              ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 12, "                                                                               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 13, "Press '1' to '4' keys to choose the object below:                              ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 15, "1) Random.exe (ROUND-ROBIN SCHEDULING & PREEMPTIVE SCHEDULING)                 ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 16, "2) Radian.exe (Ix86L-FP & PREEMPTIVE SCHEDULING)                               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 17, "3) Stack.exe (PREEMPTIVE SCHEDULING)                                           ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 18, "4) Exec.exe (PREEMPTIVE SCHEDULING)                                            ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 19, "                                                                               ", DISP_FGND_WHITE + DISP_BGND_BLACK);
	PC_DispStr(0, 20, ":                                                                              ", DISP_FGND_WHITE + DISP_BGND_BLACK);
}