#pragma once

#include <assert.h>

#ifdef _DEBUG
	#define ASSERT( EXPR, MSG) assert( MSG && EXPR );
#else
	#define ASSERT( EXPR, MSG);
#endif

#ifdef _DEBUG
#define ERROR( MSG) assert( MSG && 0 );
#else
#define ERROR( MSG);
#endif
