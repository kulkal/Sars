#ifndef __ASSERT_DEBUG_H__
#define __ASSERT_DEBUG_H__

#include <assert.h>	// for assert
#define __ASSERT(expr)		assert(expr)

long _CrtGetCurAlloc();
void _CrtOutputDataSize(size_t nDataSize);
void _CrtOutputAlloc(long lFromRequest);

#endif // __ASSERT_DEBUG_H__
