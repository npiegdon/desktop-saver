// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __REGISTRY_H
#define __REGISTRY_H

#include <string>
#include <windows.h>
   
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
   //
   // Check IsGood() after construction to see if key was opened successfully.
   Registry(const RootKey rootKey, const std::string &program, const std::string &company = "");
   ~Registry();

   // The only time this is interesting is right after object creation.  It will
   // be false if the key couldn't be opened successfully.
   const bool IsGood() const { return m_good; }

   // If the key was found and read successfully, function will return true,
   // otherwise 'out' will be filled with defaultValue, and function will
   // return false. (Regardless of return value, 'out' will always be usable)
   const bool Read(const std::string &keyName, std::string *out, const std::string &defaultValue) const;
   const bool Read(const std::string &keyName, bool *out, const bool defaultValue) const;
   const bool Read(const std::string &keyName, long *out, const long defaultValue) const;
   const bool Read(const std::string &keyName, int  *out, const int  defaultValue) const;

   void Write(const std::string &keyName, const std::string &value);
   void Write(const std::string &keyName, const bool value);
   void Write(const std::string &keyName, const long value);
   void Write(const std::string &keyName, const int  value);

   void Delete(const std::string &keyName);

private:
   bool m_good;
   HKEY m_key;
};

#endif
