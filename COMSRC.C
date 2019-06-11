/*******************************************************************************
*
*       Copyright (c) 1988-91 OS/tools Incorporated -- All rights reserved
*
*       This code is supplied on an As-is basis.  No guarantees, no promises,
*       no licenses.  Use as you see fit!
*
*   Some of this code was written for an OS/2 1.x program and some of the VIO
*   may not be done the way code written for OS/2 2.x would be done.
*
*   You must have the OS/2 Developers Tool Kit to compile this code.
*
*   This code has been successfully compiled with Microsoft's "C" 6.0 compiler,
*   but why bother?
*
*   All current development is done using Borland's OS/2 C++ compiler.
*
*   If you are interested, we have a complete Presentation Manager VIO application
*   we use for in-house testing that is available as source code.  All you need
*   to do is ask for it.
*
*   For questions, comments, or any other communication, contact us on the OS2AVEN
*   forum, Section One, or send mail to CompuServe address 70314,3235.
*
*  All trademarks herin are trademarks, or registered trademarks, of their respective
*  owners.
*
*******************************************************************************/
#define INCL_DEV
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_VIO
#include <os2.h>
#include <stdio.h>
#include <string.h>

/*
**  Atom constants - use your imagination.
*/
#define ATM_LINEWRT      3609
#define ATM_LINERD       3608
#define ATM_DCBWRT       3607
#define ATM_DCBRD        3606
#define ATM_BAUDWRT      3605
#define ATM_BAUDRD       3604
#define ATM_FXOFF        3603
#define ATM_FXON         3602
#define ATM_MDMGET       3601
#define ATM_MDMSET       3600
#define ATM_FLUSH        5034
#define ATM_COMEVENTS    5035
#define ATM_COMEVENT     5036
#define ATM_COMERROR     5037
#define ATM_XMITSTAT     5038
#define ATM_COMSTAT      5039
#define ATM_BRKON        5040
#define ATM_BRKOFF       5041
#define ATM_RCVQLEN      5042
#define ATM_TXQLEN       5043
#define ATM_IMMBYTE      5044
#define ATM_MDMGET_OUT1  5045
#define ATM_MDMSET_OUT1  5046
#define EATM_DOSOPEN     6001
#define EATM_ERRORCD     6002
#define EATM_FUNCTION    6003
#define EATM_IOCTL       6004

/*
** constants used by FlushBuffers function
*/
#define INPUT         1
#define OUTPUT        2

/*
** constants used by SendXonXoff function
*/
#define SEND_XON      0
#define SEND_XOFF     1

/*
**  User Message Constants
*/
#define UM_KILLTHREAD 30000

typedef unsigned char BYTE;
typedef unsigned short WORD;

/*
** used by function 46h
*/
typedef struct
    {
    BYTE byModemSigOn;   // activate signal if bit is a one
    BYTE byModemSigOff;  // de-activate signal if bit is zero
    }MDMSSIG;
/*
**  ON bits take presedent over OFF bits.  Bit one is DTR, bit two is RTS.
**  If you are using COMi and modem signal extensions are enabled, then bit three
**  controls OUT1 and bit six controls LOOP.  Any other bits on, or off, will
**  cause the command to be ignored and an error will be returned.
*/
/*
** used by functions 42h and 62h
*/
typedef struct
    {
    BYTE byDataBits;          // number of data bits
    BYTE byParity;            // 0 = none; 1 = odd, 2 = even, 3 = mark, 4 = space
    BYTE byStopBits;          // 0 = one stop bit, 1 = 1.5 stop bits (5 bit data word only),
                              // 2 = two stop bits (not valid for 5 bit data word)
    BOOL bTransmittingBreak;  // return only, with "get line"
    }LINECHAR;

/*
** used by function 43h
*/
typedef struct
  {
  LONG lBaudRate;
  BYTE byFraction;    // not used by COMi
  }BAUDRT;

/*
** used by function 43h
*/
typedef struct
  {
  BAUDRT stCurrentBaud;
  BAUDRT stHighestBaud;
  BAUDRT stLowestBaud;
  }BAUDST;

/*
** DCB used by functions 53h and 73h
*/
typedef struct
    {
    WORD wWrtTimeout;  // 1/10 second increments - zero = wait 1/10 second
    WORD wReadTimeout; //  ditto
    BYTE byFlags1;
    BYTE byFlags2;
    BYTE byFlags3;
    BYTE byErrorReplacementChar;
    BYTE byBreakReplacementChar;
    BYTE byXonChar;
    BYTE byXoffChar;
   }DCB;
/*
**  Constants for DCB flags
**
**  See the "Physical Device Driver" technical reference under "Category One,
**  Asynchronous Serial Device Drivers" for more detail.
*/
#define F1_ENABLE_DTR                   '\x01'
#define F1_ENABLE_DTR_INPUT_HS          '\x02'
#define F1_DTR_MASK                     '\x03'
#define F1_ENABLE_DSR_INPUT_SENSE       '\x40'
#define F1_ENABLE_DCD_OUTPUT_HS         '\x20'
#define F1_ENABLE_DSR_OUTPUT_HS         '\x10'
#define F1_ENABLE_CTS_OUTPUT_HS         '\x08'

#define F2_ENABLE_RTS                   '\x40'
#define F2_ENABLE_RTS_INPUT_HS          '\x80'
#define F2_ENABLE_RTS_TOG_ON_XMIT       '\xc0'
#define F2_RTS_MASK                     '\xc0'

#define F2_ENABLE_BREAK_REPL            '\x10'
#define F2_ENABLE_NULL_STRIP            '\x08'
#define F2_ENABLE_ERROR_REPL            '\x04'

#define F2_ENABLE_RCV_XON_XOFF_FLOW     '\x02'
#define F2_ENABLE_XMIT_XON_XOFF_FLOW    '\x01'
#define F2_ENABLE_FULL_DUPLEX           '\x20'

#define F3_USE_TX_FIFO                  '\x80'
#define F3_4_CHARACTER_FIFO             '\x20'
#define F3_8_CHARACTER_FIFO             '\x40'
#define F3_14_CHARACTER_FIFO            '\x60'
#define F3_FIFO_DISABLE                 '\x08'
#define F3_FIFO_ENABLE                  '\x10'
#define F3_FIFO_APO                     '\x18'
#define F3_FIFO_MASK                    '\xf8'
#define F3_RTO_WAIT_NORM                '\x02' // Read Time-Out
#define F3_RTO_WAIT_NONE                '\x06'
#define F3_RTO_WAIT_SOMETHING           '\x04'
#define F3_RTO_MASK                     '\x06'
#define F3_INFINITE_WTO                 '\x01' // Write Time-Out

/*
**  Function prototypes
*/
void SendXonXoff(HFILE hCom,WORD wSignal);
void FlushComBuffer(HFILE hCom,ULONG ulDirection);
APIRET ForceXoff(HFILE hCom);
APIRET ForceXon(HFILE hCom);
APIRET GetBaudRate(HFILE hCom,LONG *plBaud);
APIRET SetBaudRate(HFILE hCom,LONG lBaud);
APIRET GetLineCharacteristics(HFILE hCom,LINECHAR *pstLineChar);
APIRET SetLineCharacteristics(HFILE hCom,LINECHAR *pstLineChar);
APIRET SetModemSignals(HFILE hCom,BYTE byOnByte,BYTE byOffByte,WORD *pwCOMerror);
APIRET GetModemOutputSignals(HFILE hCom,BYTE *pbySignals);
BOOL ToggleOUT1(HFILE hCom,BOOL bToggle,BOOL *pActive);
APIRET GetModemInputSignals(HFILE hCom,BYTE *pbySignals);
APIRET GetTransmitQueueLen(HFILE hCom,WORD *pwQueueLen,WORD *pwByteCount);
APIRET GetReceiveQueueLen(HFILE hCom,WORD *pwQueueLen,WORD *pwByteCount);
APIRET BreakOff(HFILE hCom,WORD *pwCOMerror);
APIRET BreakOn(HFILE hCom,WORD *pwCOMerror);
APIRET GetCOMerror(HFILE hCom,WORD *pwCOMerror);
APIRET GetCOMevent(HFILE hCom,WORD *pwCOMevent);
APIRET GetCOMstatus(HFILE hCom,BYTE *pbyCOMstatus);
APIRET GetXmitStatus(HFILE hCom,BYTE *pbyCOMstatus);
APIRET SendDCB(HFILE hCom,DCB *pstComDCB);
APIRET GetDCB(HFILE hCom,DCB *pstComDCB);

extern HEV hevKillThreadSem;
extern BOOL bStopThread = TRUE;
extern BOOL bInputLF = FALSE;
extern BOOL bRemoteEcho = FALSE;

extern HPS hpsVio;

extern HAB habAnchorBlock;
extern char szPortName[];
extern HWND hwndFrame;
extern HWND hwndClient;
extern HFILE hCom;
extern char abyInString[100];

/*****************************************************************************
** Keyboard input
**
**  This is a code fargment that demonstrates how to "grab" a keystroke.  This
**  fragment would be placed in the main window procedure (hwndClient) message (msg)
**  case statement.  The WM_CHAR message is sent to the client window procedure
**  of the process that has keyboard focus, usually the foreground process, whenever
**  the user presses a key.

    case WM_CHAR:
      if (fProcessType == TERMINAL)
        {
        if (SHORT1FROMMP(mp1) & KC_CHAR)
          {
          chKey = SHORT1FROMMP(mp2);
          if ((rc = DosWrite(hCom,(PVOID)&chKey,1,&ulCount)) != NO_ERROR)
            {
            sprintf(szMessage,"Error Writing to Port - Error = %u",wError);
            ErrorNotify(szMessage);
            }
          if (bLocalEcho == TRUE)
            VioWrtTTY(&chKey,1,hpsVio);
          if (chKey == '\x0d')
            {
            chKey = '\x0a';
            if (bLocalEcho == TRUE)
              VioWrtTTY(&chKey,1,hpsVio);
            if (bOutputLF)
              {
              if ((rc = DosWrite(hCom,(PVOID)&chKey,1,&ulCount)) != NO_ERROR)
                {
                sprintf(szMessage,"Error Writing to Port - Error = %u",rc);
                ErrorNotify(szMessage);
                }
              }
            }
          }
        }
      else
        return(WinDefWindowProc(hwnd,msg,mp1,mp2));
      break;
*******************************************************************************/

/********************************************************************************
**
**  OpenPort
**
**********************************************************************************/
BOOL OpenPort(HWND hwnd,HFILE *phCom,char szPortName[])
  {
  ULONG ulAction;
  APIRET rc;
  CHAR szMsgString[100];
  CHAR szCaption[80];

  if ((rc = DosOpen(szPortName,phCom,&ulAction,0L,0,0x0001,0x21c2,0L)) != NO_ERROR)
    {
    WinQueryWindowText(hwndFrame,sizeof(szCaption),szCaption);
    WinLoadString(habAnchorBlock,(HMODULE)0,EATM_DOSOPEN,sizeof(szMsgString),szMsgString);
    sprintf(&szMsgString[strlen(szMsgString)],"%s\nError Code = %04X",szPortName,rc);
    WinMessageBox(HWND_DESKTOP,
                  hwnd,
             (PSZ)szMsgString,
             (PSZ)szCaption,
                  0,
                 (MB_MOVEABLE | MB_OK | MB_CUAWARNING));
    return(FALSE);
    }
  return(TRUE);
  }

void ErrorNotify(char szMessage[])
  {
  WinMessageBox( HWND_DESKTOP,
                 hwndFrame,
                 (PSZ)szMessage,
                 0,
                 0,
                 MB_MOVEABLE | MB_OK | MB_CUAWARNING );
  }

/***************************************************************************************************
**  DosDevIOCtl functions for serial devices
**
**
****************************************************************************************************/

VOID IOctlErrorMessageBox(ULONG idErrMsg,WORD wFunction,WORD wError)
  {
  CHAR szErrMsg[80];
  CHAR szMsgString[200];
  LONG lLength;

  lLength = WinLoadString(habAnchorBlock,NULLHANDLE,idErrMsg,sizeof(szMsgString),szMsgString);
  WinLoadString(habAnchorBlock,NULLHANDLE,EATM_FUNCTION,sizeof(szErrMsg),szErrMsg);
  lLength += sprintf(&szMsgString[lLength],"\n\n%s 0x%02X",szErrMsg,wFunction);
  WinLoadString(habAnchorBlock,NULLHANDLE,EATM_ERRORCD,sizeof(szErrMsg),szErrMsg);
  sprintf(&szMsgString[lLength],"\n%s 0x%04X",szErrMsg,wError);
  lLength = WinLoadString(habAnchorBlock,NULLHANDLE,EATM_IOCTL,sizeof(szErrMsg),szErrMsg);
  sprintf(&szErrMsg[lLength]," %s",szPortName);
  WinMessageBox(HWND_DESKTOP,
                hwndFrame,
           (PSZ)szMsgString,
           (PSZ)szErrMsg,
                0,
               (MB_MOVEABLE | MB_OK | MB_CUAWARNING));
  }

APIRET GetDCB(HFILE hCom,DCB *pstComDCB)
  {
  ULONG ulDataLen = sizeof(DCB);
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x73,0L,0L,&ulParmLen,(PVOID)pstComDCB,sizeof(DCB),&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_DCBRD,0x73,rc);
  return(rc);
  }

APIRET SendDCB(HFILE hCom,DCB *pstComDCB)
  {
  ULONG ulDataLen = 0L;
  ULONG ulParmLen = sizeof(DCB);
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x53,(PVOID)pstComDCB,sizeof(DCB),&ulParmLen,0L,0L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_DCBWRT,0x53,rc);
  return(rc);
  }

APIRET SendByteImmediate(HFILE hCom,BYTE bySendByte)
  {
  ULONG ulDataLen = 0L;
  ULONG ulParmLen = 1L;
  USHORT rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x44,(PVOID)&bySendByte,1L,&ulParmLen,0L,0L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_IMMBYTE,0x44,rc);
  return(rc);
  }

APIRET GetXmitStatus(HFILE hCom,BYTE *pbyCOMstatus)
  {
  ULONG ulDataLen = 1l;
  ULONG ulParmLen = 0l;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x65,0L,0L,&ulParmLen,(PVOID)pbyCOMstatus,1L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_XMITSTAT,0x65,rc);
  return(rc);
  }

APIRET GetCOMstatus(HFILE hCom,BYTE *pbyCOMstatus)
  {
  ULONG ulDataLen = 1L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x64,0L,0L,&ulParmLen,pbyCOMstatus,1L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_COMSTAT,0x64,rc);
  return(rc);
  }

APIRET GetCOMevent(HFILE hCom,WORD *pwCOMevent)
  {
  ULONG ulDataLen = 2L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x72,0L,0L,&ulParmLen,pwCOMevent,2L,&ulDataLen)) != NO_ERROR)
  	IOctlErrorMessageBox(ATM_COMEVENT,0x72,rc);
  return(rc);
  }

APIRET GetCOMerror(HFILE hCom,WORD *pwCOMerror)
  {
  ULONG ulDataLen = 2L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x6d,0L,0L,&ulParmLen,pwCOMerror,2L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_COMERROR,0x6d,rc);
  return(rc);
  }

APIRET BreakOn(HFILE hCom,WORD *pwCOMerror)
  {
  ULONG ulDataLen = 2L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x4b,0L,0L,&ulParmLen,pwCOMerror,2L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_BRKON,0x4b,rc);
  return(rc);
  }

APIRET BreakOff(HFILE hCom,WORD *pwCOMerror)
  {
  ULONG ulDataLen = 2L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x45,0L,0L,&ulParmLen,pwCOMerror,2L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_BRKOFF,0x45,rc);
  return(rc);
  }

APIRET GetReceiveQueueLen(HFILE hCom,WORD *pwQueueLen,WORD *pwByteCount)
  {
  ULONG ulDataLen = 4L;
  ULONG ulParmLen = 0L;
  APIRET rc;
  struct
    {
    WORD wByteCount;
    WORD wQueueLen;
    }stWordPair;

  if ((rc = DosDevIOCtl(hCom,0x01,0x68,0L,0L,&ulParmLen,&stWordPair,4L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_RCVQLEN,0x68,rc);
  *pwQueueLen = stWordPair.wQueueLen;
  *pwByteCount = stWordPair.wByteCount;
  return(rc);
  }

APIRET GetTransmitQueueLen(HFILE hCom,WORD *pwQueueLen,WORD *pwByteCount)
  {
  ULONG ulDataLen = 4L;
  ULONG ulParmLen = 0L;
  APIRET rc;
  struct
    {
    WORD wByteCount;
    WORD wQueueLen;
    }stWordPair;

  if ((rc = DosDevIOCtl(hCom,0x01,0x69,0L,0L,&ulParmLen,&stWordPair,4L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_TXQLEN,0x69,rc);
  *pwQueueLen = stWordPair.wQueueLen;
  *pwByteCount = stWordPair.wByteCount;
  return(rc);
  }

APIRET GetModemInputSignals(HFILE hCom,BYTE *pbySignals)
  {
  ULONG ulDataLen = 1L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x67,0L,0L,&ulParmLen,pbySignals,1L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_MDMSET,0x67,rc);
  return(rc);
  }
/*
**  Toggle the OUT1 modem signal; returns resultant OUT1 state in the "*pActivate"
**  variable, or FALSE if the device driver is not capable of performing this function.
**
**  This function will only work with the COMi device driver, when the "Extended
**  Modem Signals" extension has been enabled.
*/
BOOL ToggleOUT1(HFILE hCom,BOOL bToggle,BOOL *pActive)
  {
  ULONG ulDataLen = 1L;
  ULONG ulParmLen = 0L;
  MDMSSIG stModemSetSignals;
  BYTE byState;
  WORD wCOMerror;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x66,0L,0L,&ulParmLen,&byState,1L,&ulDataLen)) != NO_ERROR)
    {
    IOctlErrorMessageBox(ATM_MDMGET_OUT1,0x69,rc);
    return(FALSE);
    }
  byState &= '\x04';
  stModemSetSignals.byModemSigOff = ~byState;

  byState ^= '\x04';
  if ((byState & '\x04') != 0)
    *pActive = TRUE;
  else
    *pActive = FALSE;
  stModemSetSignals.byModemSigOn = byState;

  ulDataLen = 2L;
  ulParmLen = 2L;
  if ((rc = DosDevIOCtl(hCom,0x01,0x46,&stModemSetSignals,2L,&ulParmLen,&wCOMerror,2L,&ulDataLen)) != NO_ERROR)
    {
    IOctlErrorMessageBox(ATM_MDMSET_OUT1,0x69,rc);
    return(FALSE);
    }
  if (!bToggle)
    {
    stModemSetSignals.byModemSigOff = ~byState;

    if ((byState ^= '\x04') != 0)
      *pActive = TRUE;
    else
      *pActive = FALSE;
    stModemSetSignals.byModemSigOn = byState;
    if ((rc = DosDevIOCtl(hCom,0x01,0x46,&stModemSetSignals,2L,&ulParmLen,&wCOMerror,2L,&ulDataLen)) != NO_ERROR)
      {
      IOctlErrorMessageBox(ATM_MDMSET_OUT1,0x69,rc);
      return(FALSE);
      }
    }
  return(TRUE);
  }

APIRET GetModemOutputSignals(HFILE hCom,BYTE *pbySignals)
  {
  ULONG ulDataLen = 1L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x66,0L,0L,&ulParmLen,pbySignals,1L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_MDMGET,0x66,rc);
  return(rc);
  }

APIRET SetModemSignals(HFILE hCom,BYTE byOnByte,BYTE byOffByte,WORD *pwCOMerror)
  {
  ULONG ulDataLen = 2L;
  ULONG ulParmLen = 2L;
  APIRET rc;
  MDMSSIG stModemSetSignals;

  stModemSetSignals.byModemSigOn = byOnByte;
  stModemSetSignals.byModemSigOff = byOffByte;

  if ((rc = DosDevIOCtl(hCom,0x01,0x46,&stModemSetSignals,2L,&ulParmLen,pwCOMerror,2L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_MDMSET,0x46,rc);
  return(rc);
  }

APIRET SetLineCharacteristics(HFILE hCom,LINECHAR *pstLineChar)
  {
  ULONG ulDataLen = 0L;
  ULONG ulParmLen = sizeof(LINECHAR);
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x42,pstLineChar,sizeof(LINECHAR),&ulParmLen,0L,0L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_LINERD,0x42,rc);
  return(rc);
  }

APIRET GetLineCharacteristics(HFILE hCom,LINECHAR *pstLineChar)
  {
  ULONG ulDataLen = sizeof(LINECHAR);
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x62,0L,0L,&ulParmLen,pstLineChar,sizeof(LINECHAR),&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_LINERD,0x62,rc);
  return(rc);
  }

APIRET SetBaudRate(HFILE hCom,LONG lBaud)
  {
  ULONG ulDataLen = 0l;
  ULONG ulParmLen = sizeof(BAUDRT);
  APIRET rc;
  WORD wFunction = 0x0041;
  BAUDRT stBaudRate;

  stBaudRate.lBaudRate = lBaud;
  stBaudRate.byFraction = 0;
  if (stBaudRate.lBaudRate > 0xffff)
  wFunction = 0x0043;
  if ((rc = DosDevIOCtl(hCom,0x01,wFunction,&stBaudRate,sizeof(BAUDRT),&ulParmLen,0L,0L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_BAUDWRT,wFunction,rc);
  return(rc);
  }

APIRET GetBaudRate(HFILE hCom,LONG *plBaud)
  {
  ULONG ulDataLen = sizeof(BAUDRT);
  ULONG ulParmLen = 0L;
  APIRET rc;
  BAUDST stBaudRate;

  *plBaud = 0;
  if ((rc = DosDevIOCtl(hCom,0x01,0x63,0L,0L,&ulParmLen,&stBaudRate,sizeof(BAUDRT),&ulDataLen)) != NO_ERROR)
    rc = DosDevIOCtl(hCom,0x01,0x61,0L,0L,&ulParmLen,&stBaudRate,sizeof(BAUDRT),&ulDataLen);
  if (rc != NO_ERROR)
    IOctlErrorMessageBox(ATM_BAUDRD,0x63,rc);
  *plBaud = stBaudRate.stCurrentBaud.lBaudRate;
  return(rc);
  }

APIRET ForceXon(HFILE hCom)
  {
  ULONG ulDataLen = 0L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x48,0L,0L,&ulParmLen,0L,0L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_FXON,0x48,rc);
  return(rc);
  }

APIRET ForceXoff(HFILE hCom)
  {
  ULONG ulDataLen = 0L;
  ULONG ulParmLen = 0L;
  APIRET rc;

  if ((rc = DosDevIOCtl(hCom,0x01,0x47,0L,0L,&ulParmLen,0L,0L,&ulDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_FXOFF,0x47,rc);
  return(rc);
  }

void FlushComBuffer(HFILE hCom,ULONG ulDirection)
  {
  ULONG cbDataLen = 0;
  ULONG cbParmLen = 1;
  WORD Error;
  BYTE byParam = 0;

  if ((Error = DosDevIOCtl(hCom,0x0b,ulDirection,&byParam,1L,&cbParmLen,0L,0L,&cbDataLen)) != NO_ERROR)
    IOctlErrorMessageBox(ATM_FLUSH,0x0b,Error);
  }

void SendXonXoff(HFILE hCom,WORD wSignal)
  {
  DCB stComDCB;
  BYTE byTemp;

  GetDCB(hCom,&stComDCB);
  if (wSignal == SEND_XON)
    byTemp = stComDCB.byXonChar;
  else
    byTemp = stComDCB.byXoffChar;

  SendByteImmediate(hCom,byTemp);
  }

/********************************************************************************
**  Terminal Tread
**
**  This code is launched via the DosCreateThread API function.
**
**  It continues to loop until the "bStopThread" variable is TRUE.  As this thread
**  ends it sets a semphore so that the main process will know it is finisned.  This
**  is used to "synchronize" the thread exit to the main thread.  See function
**  "KillThread" for more information.
**
**  Note that all variables defined externally are accessable by both this thread
**  and the main thread.
**
**  A call to DosOpen must have been called to open the port and initialize the
**  "hCom" variable before calling DosCreateThread.
**
**********************************************************************************/

void APIENTRY TerminalThread(HFILE hCom)
  {
  APIRET rc;
  ULONG ulCount;
  BYTE byTemp;

  while (!bStopThread)
    {
    if (!bInputLF)
      {
      if ((rc = DosRead(hCom,(PVOID)abyInString,80,&ulCount)) != NO_ERROR)
        {
        sprintf(abyInString,"Error Reading Port - Error = %u",rc);
        ErrorNotify(abyInString);
        break;
        }
      else
        if (ulCount != 0)
          {
          if (bRemoteEcho == TRUE)
            if ((rc = DosWrite(hCom,(PVOID)abyInString,ulCount,&ulCount)) != NO_ERROR)
              {
              sprintf(abyInString,"Error Writing Echo to Port - Error = %u",rc);
              ErrorNotify(abyInString);
              break;
              }
          abyInString[ulCount] = 0;
          VioWrtTTY(abyInString,strlen(abyInString),hpsVio);
          }
      }
    else
      {
      if ((rc = DosRead(hCom,(PVOID)&byTemp,1,&ulCount)) != NO_ERROR)
        {
        sprintf(abyInString,"Error Reading Port - Error = %u",rc);
        ErrorNotify(abyInString);
        break;
        }
      else
        if (ulCount != 0)
          {
          if (bRemoteEcho == TRUE)
            if ((rc = DosWrite(hCom,(PVOID)&byTemp,1,&ulCount)) != NO_ERROR)
              {
              sprintf(abyInString,"Error Writing Echo to Port - Error = %u",rc);
              ErrorNotify(abyInString);
              break;
              }
          VioWrtTTY(&byTemp,1,hpsVio);
          if (byTemp == '\x0d')
            VioWrtTTY("\x0a",1,hpsVio);
          }
      }
    }
  WinSendMsg(hwndClient,UM_KILLTHREAD,0L,0L);
  DosPostEventSem(hevKillThreadSem);
  }

/*
**  CreateThread
**
**  Creates COM port thread
**
**  Since serial devices must sometimes wait for data to be received (and sometimes
**  transmitted), it makes sense to read from, and write to, a serial device from
**  some thread other than the thread that is executing the main window thread.
**
**  If you do a DosRead to the serial device driver (COMx) the device driver will
**  not return to the calling thread until either all of the characters requested
**  are received or until the read time-out period has passed.  If you DosRead from
**  the main window thread, no other window thread can run until the device driver
**  has returned control to its calling thread and that thread has returned control
**  the the window queue.
*/
VOID CreateThread(TID *ptidThread)
  {
  bStopThread = FALSE;
  DosCreateThread(ptidThread,(PFNTHREAD)TerminalThread,0L,0L,4096);
  }

/*
**  Killing the COM thread is only made complicated by the desire to have the main
**  window thread not return control to the user until the COM thread has been killed
**  and is no longer blocked in the device driver.
**
**  This could cause a problem if the read (or write) time-out is relatively long
**  and the thread is blocked in the device driver when "KillThread" is called.  It
**  would be a good idea to either flush the buffers (see "FlushComBuffer" function),
**  or cause the read time-out to be shortened (see "SetDCB" function) as part of the
**  "KillThread" processing.
*/
VOID KillThread(void)
  {
  APIRET rc;
  ULONG ulPostCount;

  DosResetEventSem(hevKillThreadSem,&ulPostCount);
  bStopThread = TRUE;
  /*
  ** Flush buffers or lower time-out values, if necessary.
  **
  ** GetDCB
  ** set time-outs to zero
  ** SetDCB
  */
  DosWaitEventSem(hevKillThreadSem,10000);  // 10000 (ten seconds) is arbitrary
                                            // and should probably be -1, once
                                            // code is debugged
  /*
  ** reset time-outs to required values
  ** SetDCB
  */
  }

