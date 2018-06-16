

#define TRUE 1
#define FALSE 0

#define ID_MAIN              1
#define IDM_OPEN  (ID_MAIN + 1)
#define IDM_CLEAR (ID_MAIN + 2)

#define ID_MLE 100


#define WMU_LOADFILE  (WM_USER + 0)
#define WMU_DONEFILE  (WM_USER + 1)

#define winEnableMenuItem(hwndMenu, id, Enable) \
    WinSendMsg(hwndMenu, MM_SETITEMATTR, \
            MPFROM2SHORT(id, TRUE), \
            MPFROM2SHORT(MIA_DISABLED, Enable ? 0 : MIA_DISABLED))


#define MAX_FNAME_LEN	  260
typedef struct _DLF {   /* dlf */
        USHORT   rgbAction;
        USHORT   rgbFlags;
        PHFILE   phFile;
        PSZ      pszExt;
        PSZ      pszAppName;
        PSZ      pszTitle;
        PSZ      pszInstructions;
        CHAR szFileName[MAX_FNAME_LEN];
        CHAR szOpenFile[MAX_FNAME_LEN];
} DLF, FAR *PDLF;

int EXPENTRY WtiDlgFile(HWND hwndOwner, PDLF pdlf);
