#ifndef	__ADDITION_H__
#define	__ADDITION_H__

#ifdef __GNUC__
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include "ffw_linux.h"

char * gnu_basename(const char *path);
char * gnu_basename_no_ext(const char *path);
char * gnu_extname(const char *path);
char * gnu_dirname (const char *path);
int gnu_splitpath(const char * path, char * dir, char * fname, char *ext);

uint32_t gnu_GetTickCount(void);
void gnu_Sleep(uint32_t milliseconds);

int gnu_strcat(char *dst, size_t len, const char *src);
int gnu_strcpy(char *dst, size_t n, const char *src);
int gnu_strupr(char *str, size_t len);

// int gnu_fopen_s(FILE** pFile, const char *filename, const char *mode);


// Microsoft functions
uint32_t GetTickCount(void);
void Sleep(uint32_t milliseconds);
errno_t strcat_s(char *strDestination, size_t numberOfElements, const char *strSource);
errno_t strcpy_s(char *dest, rsize_t dest_size, const char *src);

#define Strcmp( s1, s2 )  strcmp(s1, s2)
errno_t _strupr_s(
   char *str,
   size_t numberOfElements
);

// errno_t _strupr(char *str);
// #define _strupr(s) gnu_strupr( (s), strlen( (s) ) );
char* strupr(char *s);
#define _strupr(s) strupr( (s) );

int sprintf_s(
   char *buffer,
   size_t sizeOfBuffer,
   const char *format,
   ...
);

int fopen_s (FILE** pFile, const char *filename, const char *mode);

errno_t _splitpath_s(
   const char * path,
   char * drive,
   size_t driveNumberOfElements,
   char * dir,
   size_t dirNumberOfElements,
   char * fname,
   size_t nameNumberOfElements,
   char * ext,
   size_t extNumberOfElements
);

errno_t _splitpath(
   const char * path,
   char * drive,
   char * dir,
   char * fname,
   char * ext
);

DWORD GetPrivateProfileString(
  char* lpAppName,
  LPCTSTR lpKeyName,
  LPCTSTR lpDefault,
  LPTSTR  lpReturnedString,
  DWORD   nSize,
  LPCTSTR lpFileName
);

DWORD GetModuleFileName(
  HINSTANCE hModule,
  LPSTR   lpFilename,
  DWORD   nSize
);

BOOL PathFileExists(
  const char* pszPath
);

// Defined in ffwe20.cpp
extern HINSTANCE GetHandle(void);
extern void SetHandle(HINSTANCE hInstance);
#endif  // __GNUC__
#endif	//__ADDITION_H__