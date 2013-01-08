#include <memory.h>
#include <crtdbg.h>
#include <stdio.h>
#include <windows.h>
#include "AssertDebug.h"

// Âü°í)
// http://www.codeguru.com/Cpp/W-P/win32/tutorials/article.php/c9535/
// CodeGuru : Inside CRT : Debug Heap Management
#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
	struct _CrtMemBlockHeader * pBlockHeaderNext;
	struct _CrtMemBlockHeader * pBlockHeaderPrev;
	char *                      szFileName;
	int                         nLine;
	size_t                      nDataSize;
	int                         nBlockUse;
	long                        lRequest;
	unsigned char               gap[nNoMansLandSize];
	/* followed by:
	*  unsigned char           data[nDataSize];
	*  unsigned char           anotherGap[nNoMansLandSize];
	*/
} _CrtMemBlockHeader;

long _CrtGetCurAlloc()
{
	_CrtMemState state;
	memset(&state, 0, sizeof(_CrtMemState));
	_CrtMemCheckpoint(&state);
	if (state.pBlockHeader) return state.pBlockHeader->lRequest;
	return -1;
}

void _CrtOutputDataSize(size_t nDataSize)
{
	char szDebug[256];

	_CrtMemState state;
	memset(&state, 0, sizeof(_CrtMemState));
	_CrtMemCheckpoint(&state);
	
	// Prev
	_CrtMemBlockHeader* pCurHeader = state.pBlockHeader;
	for (;;)
	{
		if (pCurHeader == NULL) break;

		if (nDataSize == pCurHeader->nDataSize)
		{
			sprintf(szDebug, "LEAK-DEBUG %s,%d,%d\n", pCurHeader->szFileName, pCurHeader->nLine, pCurHeader->lRequest);
			OutputDebugStringA(szDebug);
		}
		pCurHeader = pCurHeader->pBlockHeaderPrev;
	}
	// Next
	pCurHeader = state.pBlockHeader;
	for (;;)
	{
		if (pCurHeader == NULL) break;

		if (nDataSize == pCurHeader->nDataSize)
		{
			sprintf(szDebug, "LEAK-DEBUG %s,%d,%d\n", pCurHeader->szFileName, pCurHeader->nLine, pCurHeader->lRequest);
			OutputDebugStringA(szDebug);
		}
		pCurHeader = pCurHeader->pBlockHeaderNext;
	}
}

void _CrtOutputAlloc(long lFromRequest)
{
	char szDebug[256];

	_CrtMemState state;
	memset(&state, 0, sizeof(_CrtMemState));
	_CrtMemCheckpoint(&state);

	_CrtMemBlockHeader* pCurHeader = state.pBlockHeader;
	if (pCurHeader == NULL) return;
	long lToRequest = pCurHeader->lRequest;

	// prev
	for (;;)
	{
		if (pCurHeader == NULL) break;

		if (lFromRequest <= pCurHeader->lRequest && pCurHeader->lRequest <= lToRequest)
		{
			sprintf(szDebug, "LEAK-DEBUG %s,%d,%d\n", pCurHeader->szFileName, pCurHeader->nLine, pCurHeader->lRequest);
			OutputDebugStringA(szDebug);
		}
		pCurHeader = pCurHeader->pBlockHeaderPrev;
	}
	// next
	pCurHeader = state.pBlockHeader;
	for (;;)
	{
		if (pCurHeader == NULL) break;

		if (lFromRequest <= pCurHeader->lRequest && pCurHeader->lRequest <= lToRequest)
		{
			sprintf(szDebug, "LEAK-DEBUG %s,%d,%d\n", pCurHeader->szFileName, pCurHeader->nLine, pCurHeader->lRequest);
			OutputDebugStringA(szDebug);
		}
		pCurHeader = pCurHeader->pBlockHeaderNext;
	}
}