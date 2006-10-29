// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "tray_icon.h"

int TrayIcon::m_icon_id_counter = 0;

TrayIcon::TrayIcon(HWND hwnd, UINT callback_id, HICON icon)
: m_hwnd(hwnd), m_callback_id(callback_id), m_icon(icon)
{
   m_tooltip = "";
   m_id = m_icon_id_counter++;

   // Now that we've set up the initial state of the object, we can
   // use our typical creation function to make the initial icon
   RestoreIcon();
}

TrayIcon::~TrayIcon()
{
   NOTIFYICONDATA nid = build_icon_data();
   bool ret = (Shell_NotifyIcon(NIM_DELETE, &nid) == TRUE);

   if (!ret) __debugbreak();
}

void TrayIcon::RestoreIcon()
{
   NOTIFYICONDATA nid = build_icon_data();
   bool ret = (Shell_NotifyIcon(NIM_ADD, &nid) == TRUE);

   if (!ret) __debugbreak();
}

void TrayIcon::SetTooltip(const std::string &tooltip)
{
   // Just set the tooltip and "modify" the icon
   m_tooltip = tooltip;

   NOTIFYICONDATA nid = build_icon_data();
   bool ret = (Shell_NotifyIcon(NIM_MODIFY, &nid) == TRUE);

   if (!ret) __debugbreak();
}

NOTIFYICONDATA TrayIcon::build_icon_data()
{
   NOTIFYICONDATA nid;
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = m_hwnd;
   nid.uID = m_id;
   nid.hIcon = m_icon;
   nid.uCallbackMessage = m_callback_id;
   nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

   // Copy the tooltip string into the structure
   strncpy_s(nid.szTip, m_tooltip.c_str(), 64);

   return nid;
}
