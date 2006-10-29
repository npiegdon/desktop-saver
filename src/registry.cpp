// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "registry.h"

using std::string;

Registry::Registry(const RootKey rootKey, const string &program, const string &company)
{
   m_good = true;
   m_key = NULL;

   const string run_buf = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
   string buffer = "Software\\";

   if (rootKey != CU_Run && rootKey != LM_Run)
   {
      if (program.length() > 0)
      {
         // Handle passing in only one string to write to the company root
         if (company.length() > 0) buffer += company + "\\" + program;
         else buffer += program;
      }
      else m_good = false;
   }

   if (m_good)
   {
      long result = 0;
      DWORD disposition;

      // Open the requested key
      switch(rootKey)
      {
      case CurrentUser:
         result = RegCreateKeyEx(HKEY_CURRENT_USER, buffer.c_str(), 0, "REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &m_key, &disposition);
         if (result != ERROR_SUCCESS) m_good = false;
         break;

      case LocalMachine:
         result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, buffer.c_str(), 0, "REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &m_key, &disposition);
         if (result != ERROR_SUCCESS) m_good = false;
         break;

      case CU_Run:
         result = RegCreateKeyEx(HKEY_CURRENT_USER, run_buf.c_str(), 0, "REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &m_key, &disposition);
         if (result != ERROR_SUCCESS) m_good = false;
         break;

      case LM_Run:
         result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, run_buf.c_str(), 0, "REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &m_key, &disposition);
         if (result != ERROR_SUCCESS) m_good = false;
         break;
      }
   }

   // The user can now check via IsGood() whether the key was opened successfully
}

Registry::~Registry()
{
   RegCloseKey(m_key);
}

void Registry::Delete(const string &keyName)
{
   if (!m_good) return;
   RegDeleteValue(m_key, keyName.c_str());
}


void Registry::Write(const string &keyName, const string &value)
{
   if (!m_good) return;
   RegSetValueEx(m_key, keyName.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (DWORD)(value.length() + 1));
}

void Registry::Write(const string &keyName, const bool value)
{
   if (!m_good) return;

   int val = (value)?1:0;
   RegSetValueEx(m_key, keyName.c_str(), 0, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
}

void Registry::Write(const string &keyName, const long value)
{
   if (!m_good) return;
   RegSetValueEx(m_key, keyName.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
}

void Registry::Write(const string &keyName, const int value)
{
   if (!m_good) return;
   RegSetValueEx(m_key, keyName.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
}

const bool Registry::Read(const string &keyName, string *out, const string &defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!m_good) return false;

   // Read the value once to get the size of the string
   long result = 0;
   DWORD size = 0;
   result = RegQueryValueEx(m_key, keyName.c_str(), 0, NULL, NULL, &size);

   // Read the value again to get the actual string
   if (result == ERROR_SUCCESS)
   {
      char *data = new char[size + 1];
      if (!data) return false;

      result = RegQueryValueEx(m_key, keyName.c_str(), 0, NULL, (LPBYTE)data, &size);

      if (result == ERROR_SUCCESS) *out = string(data);

      if (data) delete[] data;
      data = 0;
   }

   // 'out' would have only been set on success, otherwise the
   // default still exists in 'out', so we're all set
   return (result == ERROR_SUCCESS);
}

const bool Registry::Read(const string &keyName, bool *out, const bool defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!m_good) return false;

   DWORD data = 0;
   DWORD dataSize = sizeof(DWORD);
   
   const long result = RegQueryValueEx(m_key, keyName.c_str(), 0, NULL, (LPBYTE)&data, &dataSize);
   if (result == ERROR_SUCCESS) *out = !(data == 0);

   return (result == ERROR_SUCCESS);
}

const bool Registry::Read(const string &keyName, long *out, const long defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!m_good) return false;

   DWORD data = 0;
   DWORD dataSize = sizeof(DWORD);

   const long result = RegQueryValueEx(m_key, keyName.c_str(), 0, NULL, (LPBYTE)&data, &dataSize);
   if (result == ERROR_SUCCESS) *out = data;

   return (result == ERROR_SUCCESS);
}

const bool Registry::Read(const string &keyName, int *out, const int defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!m_good) return false;

   DWORD data = 0;
   DWORD dataSize = sizeof(DWORD);

   const long result = RegQueryValueEx(m_key, keyName.c_str(), 0, NULL, (LPBYTE)&data, &dataSize);
   if (result == ERROR_SUCCESS) *out = (signed)data;

   return (result == ERROR_SUCCESS);
}
