// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

#include <string>

class ErrorTracker
{
public:

   // Sets up a top-level exception handler that will write out a
   // mini-dump file on unhandled exception to the provided location
   //
   // The usual file extension is .dmp
   static void Initialize(const std::wstring &dumpFilename, bool exit1AfterDumping, bool prefixPathWithDesktop);

   // Create an exceptionless mini-dump immediately at the current
   // location.  Call Initialize() first.
   static void PerformDump();
};
