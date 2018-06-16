#define INCL_WIN
#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "2q.h"

 /*
  * 2q.c,  By Peter Fitzsimmons.  No rights reserved.
  * 
  * This sample program demonstrates how easy it is to use more than one
  * message queue in a multithreaded program.
  */

static HWND hwndFrame, hwndClient, hwndMLE;
static HMQ hmqWork;			/* worker thread's queue handle */

static MRESULT EXPENTRY MyWinProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
    FILE *fin;

    switch (msg) {
	case WM_CREATE:
	    /* create a MLE in the client window: */

	    hwndMLE = WinCreateWindow(hwnd,
		WC_MLE,
		NULL,
		MLS_BORDER | WS_VISIBLE | MLS_VSCROLL | MLS_HSCROLL,
		0, 0, 0, 0,
		hwnd,
		HWND_TOP,
		ID_MLE,
		NULL,
		NULL);
	    WinSetFocus(HWND_DESKTOP, hwndMLE);
	    break;
	case WM_ERASEBACKGROUND:
	    return ((MRESULT) TRUE);
	case WM_SIZE:
	    /* when frame is resized,  make sure MLE gets adjusted too */
	    WinSetWindowPos(hwndMLE, 0,
		5, 5,
		SHORT1FROMMP(mp2) - 10,
		SHORT2FROMMP(mp2) - 10,
		SWP_SIZE | SWP_MOVE);
	    break;
	case WM_COMMAND:
	    switch (SHORT1FROMMP(mp1)) {
		case IDM_OPEN:

		    /*
		     * Open a text file,  and tell the other thread to load
		     * it by sending a user-defined message.
		     */
		    fin = fopen("2q.c", "rb");
		    if (fin && hmqWork) {
                        WinPostQueueMsg(hmqWork, WMU_LOADFILE, (MPARAM) fin, 0);
		    }
		    else if (!fin)
			WinMessageBox(HWND_DESKTOP, hwnd,
			    "Error: I was expecting 2Q.C to be in the current directory.",
			    "Sample App", 0, MB_OK | MB_ICONEXCLAMATION);
		    break;
		case IDM_CLEAR:

		    /*
		     * Send the MLE a message telling to to delete a bunch
		     * of characters.  Note that this message can be sent
		     * while the other thread is loading the file into the
		     * MLE,  which demonstrates two threads sending
		     * messages to the same control at the same time.
		     */
		    WinSendMsg(hwndMLE, MLM_DELETE, 0, (MPARAM) 0x10000);
		    break;
	    }
	    break;
	case WMU_DONEFILE:		/* this user message sent by other
					 * thread */
	    if (MBID_YES == WinMessageBox(HWND_DESKTOP, hwnd,
		    "Finished.  Load another file?",
		    "Sample App", 0, MB_YESNO | MB_ICONQUESTION)) {
		WinPostMsg(hwnd, WM_COMMAND, MPFROMSHORT(IDM_OPEN), 0);
	    }
	    break;
	case WM_DESTROY:

	    /*
	     * The WM_QUIT message will cause the other thread to fall out
	     * of it's queue loop,  and terminate.  This is not really
	     * necessary with THIS program,	since it is about to exit()
	     * anyway.
	     */
	    WinPostQueueMsg(hmqWork, WM_QUIT, 0, 0);
	default:
	    return (WinDefWindowProc(hwnd, msg, mp1, mp2));
    }
    return (FALSE);
}

static void far pascal _loadds WorkThread(void)
{
    HAB hab;
    QMSG qmsg;
    HWND menu;
    int c;
    FILE *f;
    char buf[2];

    hab = WinInitialize(0);
    hmqWork = WinCreateMsgQueue(hab, 0);  /*DEFAULT_QUEUE_SIZE);*/

    DosSetPrty(PRTYS_THREAD, PRTYC_IDLETIME, 0, 0);

    /*- get menu handle of main window */
    menu = WinWindowFromID(hwndFrame, FID_MENU);

    while (WinGetMsg(hab, &qmsg, NULL, 0, 0)) {
	switch (qmsg.msg) {
	    case WMU_LOADFILE:
		f = (FILE *) (ULONG) qmsg.mp1;
		/* disable "open" menu */
		winEnableMenuItem(menu, IDM_OPEN, FALSE);
		buf[1] = 0;		/* terminate string */

		/*
		 * This is an incredibly slow way of loading a file into
		 * an MLE.  But that is the whole point of this program.
		 * You can take as much time as you wish in a
		 * non-window-creating-thread.
		 */
		while (EOF != (c = getc(f))) {
		    if (c == '\n')
			continue;
		    buf[0] = (char) c;
		    if (!WinSendMsg(hwndMLE, MLM_INSERT, buf, 0))
			break;
		}
		fclose(f);
		winEnableMenuItem(menu, IDM_OPEN, TRUE);

		/* tell master thread we are done */
		WinSendMsg(hwndClient, WMU_DONEFILE, 0, 0);
		break;
		/* case etc:... */
	}
    }
    WinDestroyMsgQueue(hmqWork);
    WinTerminate(hab);
    DosExit(EXIT_THREAD, 0);
}

int cdecl main(void)
{
    ULONG flstyle;
    HMQ hmq;
    QMSG qmsg;
    HAB hab;
    char *szClass = "2qclass";

    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);	/* DEFAULT_QUEUE_SIZE); */


    if (!WinRegisterClass(hab, szClass, MyWinProc, CS_SIZEREDRAW, 0))
	return (1);

    flstyle = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER | FCF_MINMAX |
	FCF_TASKLIST | FCF_SHELLPOSITION | FCF_MENU;

    hwndMLE = 0;
    hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
        0L,                             /* frame-window style            */
	&flstyle,			/* window style                  */
	szClass,			/* class name                    */
	NULL,				/* window title                  */
	0L,				/* default client style          */
	0,				/* resource in executable file   */
	ID_MAIN,			/* resource id                   */
	&hwndClient);			/* receives client window handle */

    if (!hwndFrame)
	return (1);
    else {
	static BYTE stack[4096];
	TID tid;

	DosCreateThread((PFNTHREAD) WorkThread, &tid, stack + sizeof(stack));
    }

    WinShowWindow(hwndFrame, TRUE);

    while (WinGetMsg(hab, &qmsg, NULL, 0, 0))
	WinDispatchMsg(hab, &qmsg);

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
}
