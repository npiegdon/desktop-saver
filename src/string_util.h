// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __STRING_UTIL_H
#define __STRING_UTIL_H

// Handy string macros

#ifndef STRING
#include <sstream>
#define STRING(v) ((static_cast<std::ostringstream&>(std::ostringstream().flush() << v)).str())
#endif

#ifndef WSTRING
#include <sstream>
#define WSTRING(v) ((static_cast<std::wostringstream&>(std::wostringstream().flush() << v)).str())
#endif

#endif
