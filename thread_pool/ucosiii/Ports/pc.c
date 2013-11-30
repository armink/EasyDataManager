/*
*********************************************************************************************************
*                                          PC SUPPORT FUNCTIONS
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File   : PC.C
* By     : Jean J. Labrosse
* Modify : ЛЊаж Email:591881218@qq.com
*********************************************************************************************************
*/

#include <pc.h>
#include <stdio.h>
#include <conio.h>

#define DISP_MAX_X						80       /* Maximum number of columns                          */
#define DISP_MAX_Y						25       /* Maximum number of rows                             */

#define PC_CHECK_RECURSIVE_CALLS     FALSE
#define NTIMERS							15

static CPU_BOOLEAN lock = FALSE;
static HANDLE hStdOut = NULL;

static CPU_INT32U PC_ElapsedOverhead=0;
static LARGE_INTEGER PC_StartTime[NTIMERS], PC_StopTime[NTIMERS], PC_Frequency;


/*$PAGE*/
/*
*********************************************************************************************************
*                           DISPLAY A SINGLE CHARACTER AT 'X' & 'Y' COORDINATE
*
* Description : This function writes a single character anywhere on the PC's screen.  This function
*               writes directly to video RAM instead of using the BIOS for speed reasons.  It assumed 
*               that the video adapter is VGA compatible.  Video RAM starts at absolute address 
*               0x000B8000.  Each character on the screen is composed of two bytes: the ASCII character 
*               to appear on the screen followed by a video attribute.  An attribute of 0x07 displays 
*               the character in WHITE with a black background.
*
* Arguments   : x      corresponds to the desired column on the screen.  Valid columns numbers are from
*                      0 to 79.  Column 0 corresponds to the leftmost column.
*               y      corresponds to the desired row on the screen.  Valid row numbers are from 0 to 24.
*                      Line 0 corresponds to the topmost row.
*               c      Is the ASCII character to display.  You can also specify a character with a 
*                      numeric value higher than 128.  In this case, special character based graphics
*                      will be displayed.
*               color  specifies the foreground/background color to use (see PC.H for available choices)
*                      and whether the character will blink or not.
*
* Returns     : None
*********************************************************************************************************
*/

void PC_DispChar(CPU_INT16S x, CPU_INT16S y, CPU_CHAR c, CPU_INT16U color)
{   
	COORD pos;
	CPU_SR_ALLOC();


	CPU_CRITICAL_ENTER();
    if (PC_CHECK_RECURSIVE_CALLS && lock)		// Check and avoid recursive calls
    {   
		MessageBox(NULL, "Recursive call in PC_DispChar()", "uC/OS-III", MB_OK);
		CPU_CRITICAL_EXIT();
        exit(-1);
    } 
	else if (lock)
    {
		CPU_CRITICAL_EXIT();
		return;
    } 
	else
    { 
		lock = TRUE;
    }
	CPU_CRITICAL_EXIT();

	CPU_CRITICAL_ENTER();
    if (hStdOut==NULL)							// Get the handle for the standard output
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CPU_CRITICAL_EXIT();

    if ((x>DISP_MAX_X) || (y>DISP_MAX_Y))		// Check for valid cursor position
        MessageBox(NULL, "Invalid screen position in PC_DispChar()", "uC/OS-III", MB_OK);

    pos.X = x;									// Set cursor position
    pos.Y = y;

	CPU_CRITICAL_ENTER();
    SetConsoleCursorPosition(hStdOut, pos);
    SetConsoleTextAttribute(hStdOut, color);	// Set text color
    putchar(c);									// Display character
	CPU_CRITICAL_EXIT();

	CPU_CRITICAL_ENTER();
    lock = FALSE;
	CPU_CRITICAL_EXIT();
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              CLEAR SCREEN
*
* Description : This function clears the PC's screen by directly accessing video RAM instead of using
*               the BIOS.  It assumed that the video adapter is VGA compatible.  Video RAM starts at
*               absolute address 0x000B8000.  Each character on the screen is composed of two bytes:
*               the ASCII character to appear on the screen followed by a video attribute.  An attribute
*               of 0x07 displays the character in WHITE with a black background.
*
* Arguments   : color   specifies the foreground/background color combination to use 
*                       (see PC.H for available choices)
*
* Returns     : None
*********************************************************************************************************
*/

void PC_DispClrScr(CPU_INT16U bgnd_color)
{   
	COORD pos;
	CPU_SR_ALLOC();


	CPU_CRITICAL_ENTER();
    if (PC_CHECK_RECURSIVE_CALLS && lock)			// Check and avoid recursive calls
    {   
		MessageBox(NULL, "Recursive call in PC_DispClrScr()", "uC/OS-III", MB_OK);
		CPU_CRITICAL_EXIT();
        exit(-1);
    } 
	else if (lock)
    {	
		CPU_CRITICAL_EXIT();
		return;
    } 
	else
    {   
		lock = TRUE;
    }
	CPU_CRITICAL_EXIT();

	CPU_CRITICAL_ENTER();
    if (hStdOut==NULL)								// Get the handle for the standard output
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CPU_CRITICAL_EXIT();

    pos.X = 0;
    pos.Y = 0;

	CPU_CRITICAL_ENTER();
    SetConsoleCursorPosition(hStdOut, pos);			// Set cursor position to top of window
    SetConsoleTextAttribute(hStdOut, bgnd_color);	// Set text color
    system("cls");									// Clear the screen
	CPU_CRITICAL_EXIT();

	CPU_CRITICAL_ENTER();
    lock = FALSE;
	CPU_CRITICAL_EXIT();
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                 DISPLAY A STRING  AT 'X' & 'Y' COORDINATE
*
* Description : This function writes an ASCII string anywhere on the PC's screen.  This function writes
*               directly to video RAM instead of using the BIOS for speed reasons.  It assumed that the 
*               video adapter is VGA compatible.  Video RAM starts at absolute address 0x000B8000.  Each 
*               character on the screen is composed of two bytes: the ASCII character to appear on the 
*               screen followed by a video attribute.  An attribute of 0x07 displays the character in 
*               WHITE with a black background.
*
* Arguments   : x      corresponds to the desired column on the screen.  Valid columns numbers are from
*                      0 to 79.  Column 0 corresponds to the leftmost column.
*               y      corresponds to the desired row on the screen.  Valid row numbers are from 0 to 24.
*                      Line 0 corresponds to the topmost row.
*               s      Is the ASCII string to display.  You can also specify a string containing 
*                      characters with numeric values higher than 128.  In this case, special character 
*                      based graphics will be displayed.
*               color  specifies the foreground/background color to use (see PC.H for available choices)
*                      and whether the characters will blink or not.
*
* Returns     : None
*********************************************************************************************************
*/

void PC_DispStr(CPU_INT16S x, CPU_INT16S y, CPU_CHAR *s, CPU_INT16U color)
{   
	COORD pos;
	CPU_SR_ALLOC();


	CPU_CRITICAL_ENTER();
    if (PC_CHECK_RECURSIVE_CALLS && lock)		// Check and avoid recursive calls
    {   
		MessageBox(NULL, "Recursive call in PC_DispStr()", "uC/OS-III", MB_OK);
		CPU_CRITICAL_EXIT();
        exit(-1);
    } 
	else if (lock)
    {	
		CPU_CRITICAL_EXIT();
		return;
    } 
	else
    {   
		lock = TRUE;
    }
	CPU_CRITICAL_EXIT();

	CPU_CRITICAL_ENTER();
    if (hStdOut==NULL)							// Get the handle for the standard output
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CPU_CRITICAL_EXIT();

    if ((x>DISP_MAX_X) || (y>DISP_MAX_Y))		// Check for valid cursor position
        MessageBox(NULL, "Invalid screen position in PC_DispStr()", "uC/OS-III", MB_OK);

    pos.X = x;									// Set cursor position
    pos.Y = y;
	
	CPU_CRITICAL_ENTER();
    SetConsoleCursorPosition(hStdOut, pos);
    SetConsoleTextAttribute(hStdOut, color);	// Set text color
    puts(s);									// Display text string
	CPU_CRITICAL_EXIT();

	CPU_CRITICAL_ENTER();
    lock = FALSE;
	CPU_CRITICAL_EXIT();
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       ELAPSED TIME INITIALIZATION
*
* Description : This function initialize the elapsed time module by determining how long the START and
*               STOP functions take to execute.  In other words, this function calibrates this module
*               to account for the processing time of the START and STOP functions.
*
* Arguments   : None.
*
* Returns     : None.
*********************************************************************************************************
*/

void PC_ElapsedInit(void)
{   
	static CPU_BOOLEAN initDone = FALSE;
	CPU_SR_ALLOC();


	CPU_CRITICAL_ENTER();
    QueryPerformanceFrequency(&PC_Frequency);	// Get the CPU frequency ( ticks/s)
	CPU_CRITICAL_EXIT();

    if (initDone)
    	return;

	CPU_CRITICAL_ENTER();
    PC_ElapsedOverhead = 0;						// Measure the overhead of PC_ElapsedStart
	CPU_CRITICAL_EXIT();

    PC_ElapsedStart(0);							// ... and PC_ElapsedStop

	CPU_CRITICAL_ENTER();
    PC_ElapsedOverhead = (CPU_INT32U) PC_ElapsedStop(0);
	CPU_CRITICAL_EXIT();

    initDone = TRUE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         INITIALIZE PC'S TIMER #2
*
* Description : This function initialize the PC's Timer #2 to be used to measure the time between events.
*               Timer #2 will be running when the function returns.
*
* Arguments   : None.
*
* Returns     : None.
*********************************************************************************************************
*/

void PC_ElapsedStart(CPU_INT08U n)
{   
	CPU_SR_ALLOC();


	if (n >= NTIMERS)
        return;

	CPU_CRITICAL_ENTER();
    QueryPerformanceCounter(&PC_StartTime[n]);			// Read the CPU counter and store it
	CPU_CRITICAL_EXIT();
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                 STOP THE PC'S TIMER #2 AND GET ELAPSED TIME
*
* Description : This function stops the PC's Timer #2, obtains the elapsed counts from when it was
*               started and converts the elapsed counts to micro-seconds.
*
* Arguments   : None.
*
* Returns     : The number of micro-seconds since the timer was last started.
*
* Notes       : - The returned time accounts for the processing time of the START and STOP functions.
*               - 54926 represents 54926S-16 or, 0.838097 which is used to convert timer counts to
*                 micro-seconds.  The clock source for the PC's timer #2 is 1.19318 MHz (or 0.838097 uS)
*********************************************************************************************************
*/

CPU_INT32U PC_ElapsedStop(CPU_INT08U n)
{   
	LARGE_INTEGER PC_diffTime;
	CPU_SR_ALLOC();


    if (n >= NTIMERS)
        return 0;

	CPU_CRITICAL_ENTER();
    QueryPerformanceCounter(&PC_StopTime[n]);			// Read the CPU counter and store it
    PC_diffTime.QuadPart = PC_StopTime[n].QuadPart - PC_StartTime[n].QuadPart; //Compute the difference and
														// ... convert it into microseconds
	CPU_CRITICAL_EXIT();

    return (CPU_INT32U)((PC_diffTime.QuadPart * (__int64)1000000) / PC_Frequency.QuadPart - PC_ElapsedOverhead); // uS
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       GET THE CURRENT DATE AND TIME
*
* Description: This function obtains the current date and time from the PC.
*
* Arguments  : s     is a pointer to where the ASCII string of the current date and time will be stored.
*                    You must allocate at least 21 bytes (includes the NUL) of storage in the return 
*                    string.  The date and time will be formatted as follows:
*
*                        "YYYY-MM-DD  HH:MM:SS"
*
* Returns    : none
*********************************************************************************************************
*/

void PC_GetDateTime(CPU_CHAR *s)
{   
	SYSTEMTIME now;


    GetLocalTime(&now);								// Get the local date and time

    sprintf(s, "%04d-%02d-%02d  %02d:%02d:%02d",	// Convert into a string
            now.wYear,
            now.wMonth,
            now.wDay,
            now.wHour,
            now.wMinute,
            now.wSecond);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CHECK AND GET KEYBOARD KEY
*
* Description: This function checks to see if a key has been pressed at the keyboard and returns TRUE if
*              so.  Also, if a key is pressed, the key is read and copied where the argument is pointing
*              to.
*
* Arguments  : c     is a pointer to where the read key will be stored.
*
* Returns    : TRUE  if a key was pressed
*              FALSE otherwise
*********************************************************************************************************
*/

CPU_BOOLEAN PC_GetKey(CPU_INT *c)
{   
	CPU_SR_ALLOC();


	CPU_CRITICAL_ENTER();
	if (PC_CHECK_RECURSIVE_CALLS && lock)			// Check and avoid recursive calls
	{   
		MessageBox(NULL, "Recursive call in PC_GetKey", "uC/OS-III", MB_OK);
		CPU_CRITICAL_EXIT();
		exit(-1);
	} 
	else if (lock)
    {	
		CPU_CRITICAL_EXIT();
		return FALSE;
	} 
	else
	{   
		lock = TRUE;
	}
	CPU_CRITICAL_EXIT();
    
	CPU_CRITICAL_ENTER();
    if (kbhit())									// See if a key has been pressed
    {   
		*c = getch();								// Get key pressed
        lock = FALSE;
		CPU_CRITICAL_EXIT();
        return (TRUE);
    } 
	else
    {   
		*c = 0x00;									// No key pressed
        lock = FALSE;
		CPU_CRITICAL_EXIT();
        return (FALSE);
    }
}


