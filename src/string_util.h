// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

// Handy string macros

#ifndef STRING
#include <sstream>
#define STRING(v) ((static_cast<std::ostringstream&>(std::ostringstream().flush() << v)).str())
#endif

#ifndef WSTRING
#include <sstream>
#define WSTRING(v) ((static_cast<std::wostringstream&>(std::wostringstream().flush() << v)).str())
#endif
