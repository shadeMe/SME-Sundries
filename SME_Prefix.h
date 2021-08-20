#ifndef __SME_SUNDRIES_PREFIX_H__
#define __SME_SUNDRIES_PREFIX_H__

#ifndef __cplusplus_cli
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <algorithm>
#include <fstream>
#include <windows.h>
#include <commctrl.h>
#include <time.h>

typedef unsigned char		UInt8;		//!< An unsigned 8-bit integer value
typedef unsigned short		UInt16;		//!< An unsigned 16-bit integer value
typedef unsigned long		UInt32;		//!< An unsigned 32-bit integer value
typedef unsigned long long	UInt64;		//!< An unsigned 64-bit integer value
typedef signed char			SInt8;		//!< A signed 8-bit integer value
typedef signed short		SInt16;		//!< A signed 16-bit integer value
typedef signed long			SInt32;		//!< A signed 32-bit integer value
typedef signed long long	SInt64;		//!< A signed 64-bit integer value
typedef float				Float32;	//!< A 32-bit floating point value
typedef double				Float64;	//!< A 64-bit floating point value

#define FORMAT_STR(Buffer, ...)		_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, ##__VA_ARGS__)

#undef SME_ASSERT
extern "C"
{
	_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);
}
#define SME_ASSERT(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
#endif

#define SAFEDELETE(X)				if (X)	{ delete X; X = NULL; }
#define SAFEDELETE_ARRAY(X)			if (X)	{ delete [] X; X = NULL; }
#define SAFERELEASE_D3D(X)			if (X)	{ X->Release(); X = NULL; }

#pragma warning(push)
#pragma warning(disable: 4005)
#define __STR2__(x)						#x
#define __STR1__(x)						__STR2__(x)
#define __LOC__							__FILE__(__LINE__)
#define COMPILER_MESSAGE(X)				__LOC__ : message: X
#define COMPILER_WARNING(X)				__LOC__ : warning: X
#define COMPILER_ERROR(X)				__LOC__ : error: X
#define TODO(X)							__pragma( message ( __STR1__( COMPILER_WARNING(X) ) ) )
#define HACK(X)							__pragma( message ( __STR1__( COMPILER_WARNING(X) ) ) )
#define PANIC(X)						__pragma( message ( __STR1__( COMPILER_ERROR(X) ) ) )
#pragma warning(pop)
#endif