// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed

#include <Windows.h>

#include "ErrorTracker.h"
#include "version.h"

#include "saver_gui.h"
#include "saver.h"

using namespace std;



wstring GetCommandLineArguments()
{
   wstring result;

   // CommandLineToArgvW is only available in Windows XP or later.  So,
   // rather than maintain separate binaries for Win2K, I do a runtime
   // library load and check to see if the function I need is available.
   HINSTANCE shell32 = LoadLibrary(L"shell32");
   if (!shell32) return result;

   typedef LPWSTR* (WINAPI *COMMANDLINETOARGVW_SIGNATURE)(LPCWSTR, int*);

   COMMANDLINETOARGVW_SIGNATURE function = 0;
   function = (COMMANDLINETOARGVW_SIGNATURE) GetProcAddress(shell32, "CommandLineToArgvW");

   if (function)
   {
      // Grab the file argument from the command line if there is one.
      wstring raw_command_line = GetCommandLine();

      int argument_count;
      LPWSTR *arguments = (function)(raw_command_line.c_str(), &argument_count);

      for (int i = 1; i < argument_count; ++i)
      {
         result += arguments[i];
         result += L" ";
      }
   }
   FreeLibrary(shell32);

   // Trim off the trailing space
   if (result.length() > 1) result = result.substr(0, result.length() - 1);

   return result;
}


int AutoLoadProfile(wstring profileName)
{
   DesktopSaver saver(L"DesktopSaver");

   const HistoryList &profiles = saver.NamedProfiles();
   for (HistoryIter h = profiles.begin(); h != profiles.end(); ++h)
   {
      if (h->GetName() == profileName)
      {
         saver.RestoreHistory(*h);
         return 0;
      }
   }

   return 1;
}

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE, PSTR cmdLine, int)
{
   ErrorTracker::Initialize(WSTRING(L"DesktopSaver-" << DESKTOPSAVER_VERSION << L"-crash-report.dmp"), true, true);

   if (cmdLine != 0 && cmdLine[0] != 0)
   {
      wstring profileName = GetCommandLineArguments();
      if (profileName.length() > 0) return AutoLoadProfile(profileName);
   }

   DesktopSaverGui tray(h_instance);
   return tray.Run();
}
