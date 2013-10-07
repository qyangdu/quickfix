// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#include "../config_windows.h"

#define _WINSOCKAPI_
#include <windows.h>
#undef ABSOLUTE
#undef TRUE
#undef FALSE
#undef DELETE
#undef ERROR
#using <mscorlib.dll>

#ifdef HAVE_BOOST
#define BOOST_USE_WINDOWS_H
#endif


