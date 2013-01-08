#include <stdio.h>		// for fprintf, vsprintf, ...
#include <stdarg.h>		// for va_list, va_start, ...
#include <stdlib.h>		// for abort
#include <time.h>		// for ctime, ...

#ifdef WIN32
#define _WINSOCKAPI_
#include <windows.h>
#endif

#include "AssertDebug.h"	// for __ASSERT
#include "OutputDebug.h"

#define DEBUG_OUT_FILE	"debug_out.txt"
#define DEBUG_OUT_SIZE	256

static FILE* volatile s_pFile = 0;	// debug용 file out이 출력되는 file pointer

// console에 message를 출력한다.
// szMsg : 출력할 message의 format
// ... : argument
// return : number of characters written, not including '\0'
int cout_debug(char *szMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	char s[DEBUG_OUT_SIZE];
	int cnt;
	
	va_start(argptr, szMsg);
#ifdef WIN32
	cnt = _vsnprintf(s, DEBUG_OUT_SIZE-1, szMsg, argptr);
#else // WIN32
	cnt = vsnprintf(s, DEBUG_OUT_SIZE-1, szMsg, argptr);
#endif // WIN32
	va_end(argptr);
	s[DEBUG_OUT_SIZE-2] = '\n';
	s[DEBUG_OUT_SIZE-1] = '\0';

#ifdef WIN32
	OutputDebugStringA(s);
#else // WIN32
	fflush(stderr);
	fflush(stdout);
	fprintf(stderr, s);
	fflush(stderr);
	fflush(stdout);
#endif // WIN32
	return cnt;

#else // _DEBUG
	return 0;
#endif // _DEBUG
}

// file에 message를 출력한다.
// szMsg : 출력할 message의 format
// ... : argument
// return : number of characters written, not including '\0'
int fout_debug(char *szMsg, ...)
{
#ifdef _DEBUG
	// 만약 debug용 파일이 생성되어있지 않다면 생성한다.
	if (!s_pFile)
	{
		// debug용 file을 연다.
		if ((s_pFile = fopen(DEBUG_OUT_FILE, "w")) == NULL) {__ASSERT(FALSE);}

		time_t tProcessBegin;
		time(&tProcessBegin);	
		
		// recursive call이 되었다.(-_-)
		fout_debug("Start debugging.........%s", ctime(&tProcessBegin));
		fout_debug("\n");		
	}
	
	va_list argptr;
	char s[DEBUG_OUT_SIZE];
	int cnt;
	
	va_start(argptr, szMsg);
#ifdef WIN32
        cnt = _vsnprintf(s, DEBUG_OUT_SIZE-1, szMsg, argptr);
#else // WIN32
	cnt = vsnprintf(s, DEBUG_OUT_SIZE-1, szMsg, argptr);
#endif // WIN32
	va_end(argptr);
	s[DEBUG_OUT_SIZE-2] = '\n';
	s[DEBUG_OUT_SIZE-1] = '\0';

	fflush(s_pFile);
	fprintf(s_pFile, s);
	fflush(s_pFile);
	return cnt;

#else // _DEBUG
	return 0;
#endif // _DEBUG
}

// 프로그램이 시작/끝 시간을 기록하고 끝날때 파일을 자동으로 닫게 하는 Trick!
#ifdef _DEBUG
struct AutoDebugFile
{
	AutoDebugFile() // 프로그램이 시작되면 불리게 된다.
	{
	}
	~AutoDebugFile() // 프로그램이 종료되면 불리게 된다.
	{
		if (s_pFile) // 만약 debug용 file이 생성되었다면
		{
			// 프로그램 종료 시간을 얻는다.
			time_t tProcessEnd;
			time(&tProcessEnd);				
			
			// close!
			fout_debug("\n");
			fout_debug("End debugging...........%s", ctime(&tProcessEnd));
			fclose(s_pFile);
		}
	}
} _AutoDebugFile;
#endif // _DEBUG


#define RELEASE_OUT_FILE	"release_out.txt"
static FILE* volatile s_pFile_Release = 0;

int fout_release(char *szMsg, ...)
{
	if (!s_pFile_Release)
	{
		if ((s_pFile_Release = fopen(RELEASE_OUT_FILE, "w")) == NULL)
		{
			return 0;
		}

		time_t tProcessBegin;
		time(&tProcessBegin);

		fout_release("Start debugging [Release].........%s", ctime(&tProcessBegin));
		fout_release("\n");
	}
	
	va_list argptr;
	char s[DEBUG_OUT_SIZE];
	int cnt;
	
	va_start(argptr, szMsg);
#ifdef WIN32
        cnt = _vsnprintf(s, DEBUG_OUT_SIZE-1, szMsg, argptr);
#else // WIN32
	cnt = vsnprintf(s, DEBUG_OUT_SIZE-1, szMsg, argptr);
#endif // WIN32
	va_end(argptr);
	s[DEBUG_OUT_SIZE-2] = '\n';
	s[DEBUG_OUT_SIZE-1] = '\0';

	fflush(s_pFile_Release);
	fprintf(s_pFile_Release, s);
	fflush(s_pFile_Release);
	return cnt;
}

struct AutoReleaseFile
{
	AutoReleaseFile() 
	{
	}
	~AutoReleaseFile()
	{
		if (s_pFile_Release)
		{
			time_t tProcessEnd;
			time(&tProcessEnd);				
			fout_release("\n");
			fout_release("End debugging [Release]...........%s", ctime(&tProcessEnd));
			fclose(s_pFile_Release);
		}
	}
} _AutoReleaseFile;