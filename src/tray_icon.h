// DesktopSaver, (c)2006-2017 Nicholas Piegdon, MIT licensed
#pragma once

#include <windows.h>
#include <string>

class TrayIcon
{
public:
   TrayIcon(HWND hwnd, UINT callback_id, HICON icon);
   ~TrayIcon();

   // This is called internally on creation, and also (by user code) when the
   // taskbar is re-created (after an explorer.exe crash).  Find out when to
   // call this in the MSDN article "The Taskbar", under the heading "Taskbar
   // Creation Notification".
   void RestoreIcon();

   void SetTooltip(const std::wstring &tooltip);

private:
   NOTIFYICONDATA m_icon;
};
