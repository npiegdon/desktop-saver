// DesktopSaver, (c)2006-2017 Nicholas Piegdon, MIT licensed
#pragma once

#include <string>
#include <Windows.h>

// Registry simplifies reading/writing the Windows registry
// (It currently does not support enumerating or deleting)
class Registry
{
public:
   enum RootKey { CurrentUser, LocalMachine, CU_Run, LM_Run };

   // Company is optional.  Read/Write will use [rootKey]/Software/[program] if
   // not supplied, or [rootKey]/Software/[company]/[program] if it is supplied.
   //
   // In the event of CU_Run or LM_Run, both 'program' and 'company' are ignored.
   Registry(const RootKey rootKey, const std::wstring &program, const std::wstring &company = L"");
   ~Registry();

   // If the key was found and read successfully, function will return true,
   // otherwise 'out' will be filled with defaultValue, and function will
   // return false. (Regardless of return value, 'out' will always be usable)
   bool Read(const std::wstring &keyName, std::wstring *out, const std::wstring &defaultValue) const;
   bool Read(const std::wstring &keyName, bool *out, bool defaultValue) const;
   bool Read(const std::wstring &keyName, long *out, long defaultValue) const;
   bool Read(const std::wstring &keyName, int  *out, int  defaultValue) const;

   // Convenience functions when you don't care whether things went successfully
   std::wstring Registry::Read(const std::wstring &keyName, const std::wstring &defaultValue) const { std::wstring result; Read(keyName, &result, defaultValue); return result; }
   bool Registry::Read(const std::wstring &keyName, bool defaultValue) const { bool result; Read(keyName, &result, defaultValue); return result; }
   long Registry::Read(const std::wstring &keyName, long defaultValue) const { long result; Read(keyName, &result, defaultValue); return result; }
   int Registry::Read(const std::wstring &keyName, int defaultValue) const { int result; Read(keyName, &result, defaultValue); return result; }

   void Write(const std::wstring &keyName, const std::wstring &value);
   void Write(const std::wstring &keyName, bool value);
   void Write(const std::wstring &keyName, long value);
   void Write(const std::wstring &keyName, int  value);

   void Delete(const std::wstring &keyName);

private:
   bool good;
   HKEY key;
};
