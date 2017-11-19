// DesktopSaver, (c)2006-2017 Nicholas Piegdon, MIT licensed

#include <windows.h>
#include "tray_icon.h"

TrayIcon::TrayIcon(HWND hwnd, UINT callback_id, HICON icon)
{
    m_icon.cbSize = sizeof(NOTIFYICONDATA);
    m_icon.hWnd = hwnd;
    m_icon.uID = 0;
    m_icon.hIcon = icon;
    m_icon.uCallbackMessage = callback_id;
    m_icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_icon.szTip[0] = 0;

    RestoreIcon();
}

TrayIcon::~TrayIcon() { Shell_NotifyIcon(NIM_DELETE, &m_icon); }
void TrayIcon::RestoreIcon() { Shell_NotifyIcon(NIM_ADD, &m_icon); }

void TrayIcon::SetTooltip(const std::wstring &tooltip)
{
    wcscpy_s(m_icon.szTip, 64, tooltip.c_str());
    Shell_NotifyIcon(NIM_MODIFY, &m_icon);
}
