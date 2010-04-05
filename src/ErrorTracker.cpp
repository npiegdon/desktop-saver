
#pragma comment(lib, "DbgHelp.lib")

#include "string_util.h"
#include "ErrorTracker.h"

#include <Windows.h>
#include <DbgHelp.h>
#include <shlobj.h>

using namespace std;

static wstring g_dumpFilename;
static bool g_shutdownAfterDumping;
static bool g_writeToDesktop;

BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) 
{
   if (!pInput) return FALSE; 
   if (!pOutput) return FALSE; 

   switch( pInput->CallbackType ) 
   {
   case IncludeThreadCallback:
   case IncludeModuleCallback:
   case ThreadCallback:
   case ThreadExCallback:
      return TRUE;

   case ModuleCallback: 
      if (!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory)) 
      {
         // Exclude in-memory, read-only modules
         pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
      }
      return TRUE;

   default:
      return FALSE;
   }
}

LONG WINAPI UnhandledExceptionHandler(struct _EXCEPTION_POINTERS *pep)
{
   wchar_t sh_path[MAX_PATH];
   HRESULT hr = SHGetFolderPath(0, CSIDL_DESKTOP, 0, SHGFP_TYPE_CURRENT, sh_path);
   if (g_writeToDesktop && FAILED(hr))
   {
      // "Unable to create mini-dump file. HRESULT: " << hr
      return 0;
   }

   const std::wstring path = g_writeToDesktop ? WSTRING(sh_path << L"\\" << g_dumpFilename) : g_dumpFilename;

   HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
   if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
   {
      // "Unable to create mini-dump file. Error: " << GetLastError()
      return 0;
   }

   MINIDUMP_EXCEPTION_INFORMATION mdei; 
   mdei.ThreadId = GetCurrentThreadId(); 
   mdei.ExceptionPointers = pep; 
   mdei.ClientPointers = FALSE; 

   MINIDUMP_CALLBACK_INFORMATION mci; 
   mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
   mci.CallbackParam = 0; 

   MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory); 

   BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci); 
   if (!result)
   {
      // "Couldn't write mini-dump. Error: " << GetLastError()
      return 0;
   }

   CloseHandle(hFile);

   if (pep && g_shutdownAfterDumping) exit(1);
   return 0;
}

void ErrorTracker::PerformDump()
{
   UnhandledExceptionHandler(0);
}

void ErrorTracker::Initialize(const wstring &dumpName, bool exit1AfterDumping, bool prefixPathWithDesktop)
{
   SetUnhandledExceptionFilter(&UnhandledExceptionHandler);
   g_dumpFilename = dumpName;
   g_shutdownAfterDumping = exit1AfterDumping;
   g_writeToDesktop = prefixPathWithDesktop;
}

