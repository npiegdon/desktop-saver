// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __SAVER_GUI_H
#define __SAVER_GUI_H

#include <windows.h>
#include <string>
#include <map>

class DesktopSaver;
class TrayIcon;

class DesktopSaverGui
{
public:
   DesktopSaverGui(HINSTANCE h_instance);
   ~DesktopSaverGui();

   int Run();

private:
   DesktopSaver *m_saver;

   static std::map<HWND, DesktopSaverGui*> DesktopSaverGui::gui_lookup;
   static LRESULT CALLBACK proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

   LRESULT message_destroy();
   LRESULT message_create(HWND hwnd);
   LRESULT message_timer(WPARAM timer_id);
   LRESULT message_tray(WPARAM w, LPARAM l);
   LRESULT message_menu(WPARAM choice);
   LRESULT message_default(UINT message, WPARAM wparam, LPARAM lparam);

   HMENU build_dynamic_menu();

   void update_timer();

   HWND m_hwnd;
   HINSTANCE m_hinstance;
   std::string m_app_name;
   std::string m_class_name;

   UINT m_taskbar_restart_message;
   UINT_PTR m_timer_id;

   TrayIcon *m_tray_icon;
};

#endif
