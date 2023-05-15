#include <Windows.h>
#include <stdio.h>
#define NUMBEROFPORT 8      //열수 있는 포트 수
#define RBUF_SIZE 1024		//수신버퍼크기
#define TBUF_SIZE 1024      //송신버퍼크기

#define READ_SIZE 100		//한번에 읽은 사이즈

#define   ERR_OK               0
#define   ERR_CREATEFILE      -1
#define   ERR_SETUPCOMM       -2
#define   ERR_GETCOMMSTATE    -3
#define   ERR_SETCOMMSTATE    -4
#define   ERR_GETCOMMTIMEOUT  -5
#define   ERR_WRITEFILE       -6
#define   ERR_READFILE        -7
#define   ERR_CLOSEHANDLE     -8

HANDLE hSerialPort[NUMBEROFPORT];

typedef struct  {
    unsigned int  nSize;
    char  szData[READ_SIZE];
} SERIALREADDATA;

unsigned int nTxState=0,nRxState=0;  //status bar의 rx tx 전송상태 클리어 시간 카운트
/*
Parity
Parity scheme to be used. This member can be one of the following values. Value Meaning
===================
EVENPARITY	Even
MARKPARITY	Mark
NOPARITY	No parity
ODDPARITY	Odd
SPACEPARITY	Space
===================

StopBits
Number of stop bits to be used. This member can be one of the following values. Value Meaning
============================
ONESTOPBIT		1 stop bit
ONE5STOPBITS	1.5 stop bits
TWOSTOPBITS		2 stop bits
============================
*/

int OpenSerialPort(int nPort,unsigned long nBaudRate,int nParityBit,int nDataBit,int nStopBit)
{
    wchar_t str[10];
    DCB dcb;
    COMMTIMEOUTS ct;

    wsprintfW(str,L"\\\\.\\COM%d",nPort);
    if((hSerialPort[nPort-1]=CreateFile(str,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))==INVALID_HANDLE_VALUE) return ERR_CREATEFILE;
    if(!SetupComm(hSerialPort[nPort-1],RBUF_SIZE,TBUF_SIZE)) return ERR_SETUPCOMM;
    if(!GetCommState(hSerialPort[nPort-1],&dcb)) return ERR_GETCOMMSTATE;

    dcb.BaudRate = nBaudRate;
    dcb.ByteSize = nDataBit;
    dcb.Parity   = nParityBit;
    dcb.StopBits = nStopBit;
    dcb.fNull    = FALSE;   //0x00 송수신가능


    //흐름제어 : 없음
    dcb.fOutxCtsFlow = false;					// Disable CTS monitoring
    dcb.fOutxDsrFlow = false;					// Disable DSR monitoring
    dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR monitoring
    dcb.fOutX = false;							// Disable XON/XOFF for transmission
    dcb.fInX = false;							// Disable XON/XOFF for receiving
    dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
    /*
     // 흐름제어 : 하드웨어(DTR,RTS,CTS,DSR)
        dcb.fOutxCtsFlow = true;					// Enable CTS monitoring
        dcb.fOutxDsrFlow = true;					// Enable DSR monitoring
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;	// Enable DTR handshaking
        dcb.fOutX = false;							// Disable XON/XOFF for transmission
        dcb.fInX = false;							// Disable XON/XOFF for receiving
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;	// Enable RTS handshaking

     // 흐름제어 : 소프트웨어(Xon,Xoff)
        dcb.fOutxCtsFlow = false;					// Disable CTS (Clear To Send)
        dcb.fOutxDsrFlow = false;					// Disable DSR (Data Set Ready)
        dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR (Data Terminal Ready)
        dcb.fOutX = true;							// Enable XON/XOFF for transmission
        dcb.fInX = true;							// Enable XON/XOFF for receiving
        dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
*/


    if (!SetCommState(hSerialPort[nPort-1],&dcb)) return ERR_SETCOMMSTATE;
    if (!GetCommTimeouts(hSerialPort[nPort-1],&ct)) return ERR_GETCOMMTIMEOUT;

    ct.ReadIntervalTimeout = 100;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.ReadTotalTimeoutConstant = 10;
    ct.WriteTotalTimeoutMultiplier = 0;
    ct.WriteTotalTimeoutConstant = 10;

    if(!SetCommTimeouts(hSerialPort[nPort-1],&ct)) return ERR_GETCOMMTIMEOUT;

    return ERR_OK;
}

int CloseSerialPort(int nPort)
{	if (!CloseHandle(hSerialPort[nPort-1])) return ERR_CLOSEHANDLE;
    return ERR_OK;
}

#define RXTXONTIME 10
/*int WriteSerialPort(int nPort,unsigned char *szData,unsigned int nBytesToWrite)
{	unsigned int nBytesWritten;
    nTxState=RXTXONTIME;   // 상태표시만은 위한 코드
    if (!WriteFile(hSerialPort[nPort-1],szData,nBytesToWrite,(unsigned long *)&nBytesWritten,NULL)) return ERR_WRITEFILE;
    return ERR_OK;
}
*/

int WriteSerialPort(int nPort,unsigned char *szData,unsigned int nBytesToWrite)
{	unsigned int nBytesWritten;
    unsigned int nBytesCompleted=0;
    nTxState=RXTXONTIME;   // 상태표시만은 위한 코드
    while(nBytesCompleted<nBytesToWrite)
    {
        if (!WriteFile(hSerialPort[nPort-1],&szData[nBytesCompleted],nBytesToWrite-nBytesCompleted,(unsigned long *)&nBytesWritten,NULL)) return ERR_WRITEFILE;
        nBytesCompleted+=nBytesWritten;
    }
    return ERR_OK;
}



int ReadSerialPort(int nPort,SERIALREADDATA * SerialReadData)
// nPort = 8, SERIALREADDATA: &srd
{
    if (!ReadFile(hSerialPort[nPort - 1], SerialReadData->szData, READ_SIZE, (unsigned long*)&SerialReadData->nSize, NULL))
    {
        return ERR_READFILE;
    }
    if(SerialReadData->nSize!=0)
    {
        nRxState = RXTXONTIME;  // 상태표시만은 위한 코드
    }
    return ERR_OK;
}

