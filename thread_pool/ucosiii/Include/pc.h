/*
*********************************************************************************************************
*                                          PC SUPPORT FUNCTIONS
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File   : pc.h
* By     : Jean J. Labrosse
* Modify : ЛЊаж Email:591881218@qq.com
*********************************************************************************************************
*/

#ifndef PC_H
#define PC_H

#include <os.h>
#include <windows.h>

/*$PAGE*/
/*
*********************************************************************************************************
*                                               CONSTANTS
*                                    COLOR ATTRIBUTES FOR VGA MONITOR
*
* Description: These #defines are used in the PC_Disp???() functions.  The 'color' argument in these
*              function MUST specify a 'foreground' color, a 'background' and whether the display will
*              blink or not.  If you don't specify a background color, BLACK is assumed.  You would 
*              specify a color combination as follows:
*
*              PC_DispChar(0, 0, 'A', DISP_FGND_WHITE + DISP_BGND_BLUE + DISP_BLINK);
*
*              To have the ASCII character 'A' blink with a white letter on a blue background.
*********************************************************************************************************
*/

#define DISP_FGND_BLACK           0x00
#define DISP_FGND_BLUE            FOREGROUND_BLUE
#define DISP_FGND_GREEN           FOREGROUND_GREEN
#define DISP_FGND_RED             FOREGROUND_RED
#define DISP_FGND_CYAN            (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define DISP_FGND_YELLOW          (FOREGROUND_RED  | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define DISP_FGND_WHITE           (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
#define DISP_FGND_GRAY            (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

#define DISP_BGND_BLACK           0x00
#define DISP_BGND_BLUE            BACKGROUND_BLUE
#define DISP_BGND_GREEN           BACKGROUND_GREEN
#define DISP_BGND_RED             BACKGROUND_RED
#define DISP_BGND_CYAN            (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define DISP_BGND_YELLOW          (BACKGROUND_RED  | BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define DISP_BGND_WHITE           (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)
#define DISP_BGND_GRAY            (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
#define DISP_BGND_LIGHT_GRAY      (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)

/*$PAGE*/
/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void    PC_DispChar(CPU_INT16S x, CPU_INT16S y, CPU_CHAR c, CPU_INT16U color);
void    PC_DispClrScr(CPU_INT16U bgnd_color);
void    PC_DispStr(CPU_INT16S x, CPU_INT16S y, CPU_CHAR *s, CPU_INT16U color);

void    PC_ElapsedInit(void);
void    PC_ElapsedStart(CPU_INT08U n);
CPU_INT32U PC_ElapsedStop(CPU_INT08U n);

void    PC_GetDateTime(CPU_CHAR *s);
CPU_BOOLEAN PC_GetKey(CPU_INT *c);

#endif
