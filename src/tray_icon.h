// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __TRAY_ICON_H
#define __TRAY_ICON_H

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

   void SetTooltip(const std::string &tooltip);

private:

   NOTIFYICONDATA build_icon_data();

   int m_id;
   HWND m_hwnd;
   UINT m_callback_id;
   HICON m_icon;
   std::string m_tooltip;

   static int m_icon_id_counter;
};

#endif
