#include "addition.h"

#ifdef __GNUC__

char * gnu_basename(const char *path)
{
	char *base = (char*)strrchr(path, '/');
	return base ? base+1 : (char*)path;
}

char * gnu_basename_no_ext(const char *path)
{
    char *filename, *basename;
    char *extname;
    int name_len;
    filename = path != NULL ? (char*)strrchr (path, '/') + 1 : NULL;
    if(filename == NULL)
        return NULL;

	extname = (char*)strrchr(path, '.');
    if(!extname)
        return filename;
    
    name_len = extname - filename + 1;
    basename = (char*)malloc(name_len);
    strncpy(basename, filename, name_len-1);
    basename[name_len] = '\0';
	
    return basename;
}

char * gnu_extname(const char *path)
{
	return (char*)strrchr(path, '.');
}

char * gnu_dirname (const char *path)
{
    static const char dot[] = "./";
    char *last_slash;
    size_t path_len ;
    char* dir;

    last_slash = path != NULL ? (char*)strrchr (path, '/') : NULL;
	if (last_slash != NULL)
	{
		path_len = last_slash - path + 1;
		dir = (char*)malloc(path_len);
		memcpy(dir, path, path_len);
	}
	else
	{
		return (char*)dot;
	}

  return dir;
}

/*
 * path  : Full path.
 * dir   : Directory path, including trailing slash. Forward slashes ( / ) is only used. 
 *         You can pass NULL for this parameter if you do not need the directory path.
 * fname : Base filename (no extension). 
 *         You can pass NULL for this parameter if you do not need the filename.
 * ext   : Filename extension, including leading period (.). 
 *         You can pass NULL for this parameter if you do not need the filename extension.
 */
int gnu_splitpath(const char * path, char * dir, char * fname, char *ext)
{
	if(dir != NULL)
		sprintf(dir, "%s", gnu_dirname(path));

    if(fname != NULL){
        if(ext)
            sprintf(fname, "%s", gnu_basename_no_ext(path));
        else
            sprintf(fname, "%s", gnu_basename(path));
    }
	
	if(ext != NULL)
		sprintf(ext, "%s", gnu_extname(path));

	return errno;
}

/*
 * The return value is the number of milliseconds that have elapsed 
 * since the system was started.
 */
uint32_t gnu_GetTickCount(void)
{
	clock_t milisec;
	/*
	 * This function returns the number of clock ticks elapsed since
	 * the start of the program. On failure, the function returns a 
	 * value of -1.
	 * On a 32 bit system where CLOCKS_PER_SEC equals 1000000, clock() 
	 * function will return the same value approximately every 72 minutes.
	 */
	milisec = (clock_t)1000 * clock();
	return milisec;
}

void gnu_Sleep(uint32_t milliseconds)
{
	usleep(milliseconds * 1000);
}

int gnu_strcat(char *dst, size_t n, const char *src)
{
	strcat(dst, src);
	return errno;
}

int gnu_strcpy(char *dst, size_t dst_sz, const char *src)
{
	strcpy(dst, src);
	return errno;
}

// Returns a pointer to the altered string. Because the modification is done in place,
// the pointer returned is the same as the pointer passed as the input argument.
// No return value is reserved to indicate an error.
int gnu_strupr(char *str, size_t n)
{
	int i = 0;
	char *_s = (char*)malloc(n);
	memset(_s, '\0', sizeof(_s));
	memcpy(_s, str, n);
	while(str[i] && (i < n)) {
		_s[i] = toupper(_s[i]);
		i++;
	}
	sprintf(str, "%s", _s);
	return errno;
}

char* strupr(char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++)
  {
    *ucs = toupper(*ucs);
  }
  return s;
}

static inline int gnu_fopen_s(FILE** pFile, const char *filename, const char *mode)
{
	*pFile = fopen(filename, mode);
	return errno;
}

int gnu_PathFileExists(const char* path)
{
	FILE *fp;
	if (fp = fopen(path, "r")) {
		fclose(fp);
		return 1;	// File exist
	}
	else return 0; // File not exist
}

///////////////////////////////
// Microsoft to GNU function
///////////////////////////////
uint32_t GetTickCount(void)
{
	return gnu_GetTickCount();
}

/*
 * Suspends the execution of the current thread until the time-out 
 * interval elapses, in miliseconds
 */
void Sleep(uint32_t milliseconds)
{
	gnu_Sleep(milliseconds);
}

errno_t strcat_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	return gnu_strcat(strDestination, numberOfElements, strSource);
}

errno_t strcpy_s(char *dest, rsize_t dest_size, const char *src)
{
	return gnu_strcpy(dest, dest_size, src);
}

errno_t _strupr_s(char *str, size_t numberOfElements) 
{
	return gnu_strupr(str, numberOfElements);
}

int sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...)
{
	va_list aptr;
	int ret;

	va_start(aptr, format);
	ret = vsprintf(buffer, format, aptr);
	va_end(aptr);

	return ret;
}

int fopen_s (FILE** pFile, const char *filename, const char *mode)
{ 
    return gnu_fopen_s(pFile, filename, mode);
};


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
) {
	return gnu_splitpath(path, dir, fname, ext);
}

errno_t _splitpath(
   const char * path,
   char * drive,
   char * dir,
   char * fname,
   char * ext
) {
	return gnu_splitpath(path, dir, fname, ext);
}

DWORD GetPrivateProfileString(
  char* lpAppName,
  LPCTSTR lpKeyName,
  LPCTSTR lpDefault,
  LPTSTR  lpReturnedString,
  DWORD   nSize,
  LPCTSTR lpFileName
) {
	printf("[%s] Microsoft function, not implemted\n", __func__);
	return 0;
}

DWORD GetModuleFileName(
  HINSTANCE hModule,
  LPSTR   lpFilename,
  DWORD   nSize
) {
	printf("[%s] Microsoft function, not implemted\n", __func__);
	return 0;
}

/*
 * Determines whether a path to a file system object 
 * such as a file or folder is valid.
 * Return TRUE if the file exists; otherwise, FALSE.
 */
BOOL PathFileExists(
  const char* pszPath
) {
	return gnu_PathFileExists(pszPath) ? TRUE : FALSE;
}

#endif