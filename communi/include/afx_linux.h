#ifndef	__AFX_LINUX_H__
#define	__AFX_LINUX_H__

#ifdef __GNUC__
#endif 

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

typedef int OVERLAPPED;
typedef int CSemaphore;
typedef int CSingleLock;

#define TRUE (BOOL)true
#define FALSE (BOOL)false

#define _64_BIT_

#define VOID void
#define CONST const

typedef int BOOL;
typedef unsigned char BYTE;
typedef BYTE BOOLEAN;
typedef char CCHAR;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned char UCHAR;
typedef wchar_t WCHAR;
// typedef unsigned short WORD;
typedef uint16_t WORD;
// typedef unsigned long DWORD;
typedef uint32_t DWORD;
typedef uint32_t DWORD32;
typedef uint64_t DWORD64;
typedef uint64_t DWORDLONG;
typedef uint64_t QWORD;
typedef short SHORT;
typedef unsigned short USHORT;
typedef int INT;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
typedef uint8_t UINT8;
typedef unsigned int UINT;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef long LONG;
typedef int32_t LONG32;
typedef int64_t LONG64;
typedef int64_t LONGLONG; 
typedef unsigned long ULONG;
typedef uint32_t ULONG32;
typedef uint64_t ULONG64;
typedef uint64_t ULONGLONG;
typedef float FLOAT;
typedef int HFILE;
#if defined(_64_BIT_)
 typedef uint64_t ULONG_PTR;
#else
 typedef unsigned long ULONG_PTR;
#endif
// #ifdef _64_BIT_
//  typedef int HALF_PTR;
// #else
//  typedef short HALF_PTR;
// #endif
#if defined(_64_BIT_) 
 typedef int64_t INT_PTR; 
#else 
 typedef int INT_PTR;
#endif
#if defined(_64_BIT_)
 typedef int64_t LONG_PTR; 
#else
 typedef long LONG_PTR;
#endif
typedef LONG_PTR LPARAM;
typedef BOOL *LPBOOL;
typedef BYTE *LPBYTE;
typedef CONST void *LPCVOID;
typedef DWORD *LPDWORD;
typedef void *PVOID;
typedef void *LPVOID;
typedef PVOID HANDLE;
typedef HANDLE *PHANDLE;
typedef HANDLE *LPHANDLE;
typedef HANDLE SERVICE_STATUS_HANDLE;
typedef int *LPINT;
typedef long *LPLONG;
typedef CHAR *LPSTR;
typedef WORD *LPWORD;
typedef BOOL *PBOOL;
typedef BOOLEAN *PBOOLEAN;
typedef BYTE *PBYTE;
typedef CHAR *PCHAR;
typedef DWORD *PDWORD;
typedef ULONG_PTR DWORD_PTR;
typedef DWORD_PTR *PDWORD_PTR;
typedef DWORD32 *PDWORD32;
typedef DWORD64 *PDWORD64;
typedef FLOAT *PFLOAT;
// #ifdef _64_BIT_
//  typedef HALF_PTR *PHALF_PTR;
// #else
//  typedef HALF_PTR *PHALF_PTR;
// #endif
typedef int *PINT;
typedef INT_PTR *PINT_PTR;
typedef INT8 *PINT8;
typedef INT16 *PINT16;
typedef INT32 *PINT32;
typedef INT64 *PINT64;
typedef LONG *PLONG;
typedef LONGLONG *PLONGLONG;
typedef LONG_PTR *PLONG_PTR;
typedef LONG32 *PLONG32;
typedef LONG64 *PLONG64;
typedef USHORT *PUSHORT;
typedef WORD *PWORD;
typedef ULONG_PTR SIZE_T;
typedef unsigned char UCHAR;
// #ifdef _64_BIT_
//  typedef unsigned int UHALF_PTR;
// #else
//  typedef unsigned short UHALF_PTR;
// #endif

typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
} GUID;

#define INVALID_HANDLE_VALUE (HANDLE)(-1)

//
// winioctl.h
//
#ifndef FILE_READ_DATA
#define FILE_READ_DATA            0x0001    /* file & pipe */
#endif

#ifndef FILE_WRITE_DATA
#define FILE_WRITE_DATA           0x0002    /* file & pipe */
#endif

#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS                FILE_READ_DATA  /* file & pipe */
#define FILE_WRITE_ACCESS               FILE_WRITE_DATA /* file & pipe */


// Maximum Path Length Limitation: In the Windows API (with some exceptions
// discussed in the following paragraphs), the maximum length for a path is
// MAX_PATH, which is defined as 260 characters.
#define MAX_PATH 260

#define INFINITE (-1)
#endif	//__AFX_LINUX_H__