// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include <algorithm>

#include <windows.h>
#include "resource.h"

#include "saver_gui.h"
#include "saver.h"
#include "version.h"
#include "tray_icon.h"
#include "create_dialog.h"
using namespace std;

static const int WM_TRAYMESSAGE =          WM_USER + 1;

// Main Menu
static const int WM_Tray_History_Clear =   WM_USER + 2;
static const int WM_Tray_Exit =            WM_USER + 3;
static const int WM_Tray_Profile_Create =  WM_USER + 4;

// Options
static const int WM_Tray_On_Startup =      WM_USER + 5;
static const int WM_Tray_Disable_History = WM_USER + 6;
static const int WM_Tray_Poll_Endpoints =  WM_USER + 7;
static const int WM_Tray_Poll_Interval1 =  WM_USER + 8;
static const int WM_Tray_Poll_Interval2 =  WM_USER + 9;
static const int WM_Tray_Poll_Interval3 =  WM_USER + 10;
static const int WM_Tray_Poll_Interval4 =  WM_USER + 11;

// Lookups
// NOTE: Order is very significant here
static const int WM_Lookup_Begin =         WM_USER + 12;
static const int WM_Tray_History =         WM_Lookup_Begin;
static const int WM_Tray_Named_Profile =   WM_Tray_History + DesktopSaver::MaxIconHistoryCount;
static const int WM_Tray_Profile_Update =  WM_Tray_Named_Profile + DesktopSaver::MaxProfileCount;
static const int WM_Tray_Profile_Delete =  WM_Tray_Profile_Update + DesktopSaver::MaxProfileCount;

static const LRESULT RET_DEF_PROC = -35;

// Sort of like a Singleton
map<HWND, DesktopSaverGui*> DesktopSaverGui::gui_lookup;

DesktopSaverGui::DesktopSaverGui(HINSTANCE hinst)
{
   m_app_name = L"DesktopSaver";
   m_class_name = L"desktop_saver";
   m_hinstance = hinst;

   // Win32 Stuff
   WNDCLASS wndclass;
   wndclass.style = 0;
   wndclass.lpfnWndProc = DesktopSaverGui::proc;
   wndclass.cbClsExtra = 0;
   wndclass.cbWndExtra = 0;
   wndclass.hInstance = hinst;
   wndclass.hIcon = LoadIcon(hinst, L"IDI_APP_ICON");
   wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
   wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
   wndclass.lpszMenuName = NULL;
   wndclass.lpszClassName = m_class_name.c_str();

   if (!RegisterClass(&wndclass))
   {
      INTERNAL_ERROR(L"Couldn't register the " << m_class_name << L" window!");
      exit(1);
   }

   gui_lookup[0] = this;
   m_hwnd = CreateWindow(m_class_name.c_str(), WSTRING(m_app_name << L" " << DESKTOPSAVER_VERSION).c_str(),
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 350, 200, NULL, NULL, hinst, this);

   if (!m_hwnd)
   {
      INTERNAL_ERROR(L"Couldn't create main program window!");
      exit(1);
   }

   // Leave the (default, completely empty, useless) window hidden
   ShowWindow(m_hwnd, SW_HIDE);
   UpdateWindow(m_hwnd);

   // Create the system tray icon
   m_tray_icon = new TrayIcon(m_hwnd, WM_TRAYMESSAGE, LoadIcon(hinst, L"IDI_APP_ICON"));
   m_tray_icon->SetTooltip(WSTRING(m_app_name << L" " << DESKTOPSAVER_VERSION));

   m_saver = new DesktopSaver(m_app_name);

   // Create our desktop icon polling timer
   m_timer_id = 1;
   update_timer();

}

DesktopSaverGui::~DesktopSaverGui()
{
   if (m_tray_icon) delete m_tray_icon;
   if (m_saver) delete m_saver;
}

int DesktopSaverGui::Run()
{
   MSG message;
   while (GetMessage(&message, NULL, 0, 0))
   {
      TranslateMessage(&message);
      DispatchMessage(&message);
   }
   return int(message.wParam);
}


LRESULT CALLBACK DesktopSaverGui::proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
   if (gui_lookup[hwnd] == 0)
   {
      gui_lookup[hwnd] = gui_lookup[0];
      gui_lookup[0] = 0;
   }

   DesktopSaverGui *gui = gui_lookup[hwnd];
   if (!gui)
   {
      INTERNAL_ERROR(L"ERROR: Win32 message received before global Icon Saver instance created!");
      exit(1);
   }

   switch (message)
   {
   case WM_ENDSESSION:  // fall-through
   case WM_DESTROY:     { return gui->message_destroy(); }
   case WM_CREATE:      { return gui->message_create(hwnd); }
   case WM_TRAYMESSAGE: { return gui->message_tray(wparam, lparam); }
   case WM_COMMAND:     { return gui->message_menu(wparam); }
   case WM_TIMER:       { return gui->message_timer(wparam); return 0; }
   default:             {
      LRESULT ret = gui->message_default(message, wparam, lparam);

      if (ret == RET_DEF_PROC) break;
      else return ret;
                        }
   }
   return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT DesktopSaverGui::message_timer(WPARAM timer_id)
{
   // This should never happen, but isn't necessarily a critical error
   if (timer_id != m_timer_id) INTERNAL_ERROR(L"An unknown (external) timer event was received!");

   m_saver->PollDesktopIcons();

   return 0;
}

LRESULT DesktopSaverGui::message_default(UINT message, WPARAM wparam, LPARAM lparam)
{
   if (message == m_taskbar_restart_message)
   {
      // After an explorer crash, you have to re-add your icons to the tray
      m_tray_icon->RestoreIcon();

      // Because explorer probably just restarted, it might be a good
      // idea to poll immediately and see what havok was caused.
      m_saver->PollDesktopIcons();

      return 0;
   }

   // Tell our WndProc to use DefWindowProc for all other unhandled messages
   return RET_DEF_PROC;
}

LRESULT DesktopSaverGui::message_create(HWND hwnd)
{
   // WARNING: Do not use m_hwnd in this message, it's not valid at this
   //          point!  Instead, use the passed-in hwnd from WndProc

   // Register that we want to know when explorer.exe recovers from a crash
   m_taskbar_restart_message = RegisterWindowMessage(L"TaskbarCreated");

   return 0;
}

LRESULT DesktopSaverGui::message_destroy()
{
   // Stop the automatic polling
   KillTimer(m_hwnd, m_timer_id);

   // Poll one last time just before we shut down
   m_saver->PollDesktopIcons();

   // Signal that we're quitting
   PostQuitMessage(0);

   return 0;
}

LRESULT DesktopSaverGui::message_tray(WPARAM w, LPARAM l)
{
   switch (l)
   {
   case WM_LBUTTONUP:
   case WM_RBUTTONUP:
   case WM_CONTEXTMENU:
      // Only let certain messages pass through (all of which
      // simply pop-up the menu)
      break;

   default: return DefWindowProc(m_hwnd, WM_TRAYMESSAGE, w, l);
   }

   // Poll just before we create the menu so that it
   // looks like we get an instant response
   m_saver->PollDesktopIcons();

   // Dynamically build our history menu
   HMENU menu = build_dynamic_menu();

   // We need the cursor position to know where to pop up the menu
   POINT point;
   GetCursorPos(&point);

   // Some of this jazz is required because of a Microsoft KB article having 
   // to do with popup menus not disappearing if you click elsewhere.
   SetForegroundWindow(m_hwnd);
   TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, point.x, point.y, 0, m_hwnd, 0);
   PostMessage(m_hwnd, WM_NULL, 0, 0);

   // TrackPopupMenu only returns after it's been taken care
   // of, so we can clean up our resources immediately afterwards
   //
   // DestroyMenu is recursive, so it handles the submenus
   DestroyMenu(menu);

   return 0;
}

HMENU DesktopSaverGui::build_dynamic_menu()
{
   HMENU options = CreatePopupMenu();

   // Find out whether the run-at-startup options should be checked
   long registry_checked = (m_saver->GetRunOnStartup() ? MF_CHECKED : 0);
   AppendMenu(options, MF_STRING | registry_checked, WM_Tray_On_Startup, L"&Run at Startup");

   AppendMenu(options, MF_SEPARATOR, 0, 0);

   PollRate p = m_saver->GetPollRate();

   // Decide which of these gets the checkmark
   AppendMenu(options, MF_STRING | (p==DisableHistory?MF_CHECKED:0), WM_Tray_Disable_History, L"&Disable History");
   AppendMenu(options, MF_STRING | (p==PollEndpoints?MF_CHECKED:0),  WM_Tray_Poll_Endpoints, L"Poll at Startup and Shutdown only");
   AppendMenu(options, MF_STRING | (p==Interval1?MF_CHECKED:0),      WM_Tray_Poll_Interval1, L"Poll every 5 minutes");
   AppendMenu(options, MF_STRING | (p==Interval2?MF_CHECKED:0),      WM_Tray_Poll_Interval2, L"Poll every 20 minutes");
   AppendMenu(options, MF_STRING | (p==Interval3?MF_CHECKED:0),      WM_Tray_Poll_Interval3, L"Poll every 60 minutes");
   AppendMenu(options, MF_STRING | (p==Interval4?MF_CHECKED:0),      WM_Tray_Poll_Interval4, L"Poll every 360 minutes");

   const HistoryList &named_profiles = m_saver->NamedProfiles();

   // Build up each "Update Profile" menu item
   HMENU profile_update = CreatePopupMenu();
   int update_choice = 0;
   for (HistoryRevIter i = named_profiles.rbegin(); i != named_profiles.rend(); ++i)
   {
      AppendMenu(profile_update, MF_STRING, WM_Tray_Profile_Update + update_choice++, i->GetName().c_str());
   }

   // Build up each "Delete Profile" menu item
   HMENU profile_delete = CreatePopupMenu();
   int delete_choice = 0;
   for (HistoryRevIter i = named_profiles.rbegin(); i != named_profiles.rend(); ++i)
   {
      AppendMenu(profile_delete, MF_STRING, WM_Tray_Profile_Delete + delete_choice++, i->GetName().c_str());
   }

   HMENU menu = CreatePopupMenu();

   const HistoryList &history = m_saver->History();

   // This shouldn't happen (but is non-critical)
   if (history.size() > DesktopSaver::MaxIconHistoryCount) INTERNAL_ERROR(L"History List too long!");

   // If History is disabled, don't show the history list at all
   if (m_saver->GetPollRate() != DisableHistory)
   {
      // Build up each history menu item
      int history_choice = 0;
      for (HistoryRevIter i = history.rbegin(); i != history.rend(); ++i)
      {
         AppendMenu(menu, MF_STRING, WM_Tray_History + history_choice++, i->GetName().c_str());
      }

      // Decide whether to gray-out the "Clear History" option, if we don't
      // have any history slices to clear
      long additional_clear_flags = MF_GRAYED;
      if (history.size() > 0) additional_clear_flags = 0;
      AppendMenu(menu, MF_STRING | additional_clear_flags, WM_Tray_History_Clear, L"&Clear History");

      AppendMenu(menu, MF_SEPARATOR, 0, 0);
   }

   // Add each named profile to this list
   int named_choice = 0;
   for (HistoryRevIter i = named_profiles.rbegin(); i != named_profiles.rend(); ++i)
   {
      AppendMenu(menu, MF_STRING, WM_Tray_Named_Profile + named_choice++, i->GetName().c_str());
   }


   // If we've reached our maximum number of user-created
   // named profiles, disable the "Create" command
   bool too_many_profiles = false;
   if (m_saver->NamedProfiles().size() >= DesktopSaver::MaxProfileCount)
   {
      too_many_profiles = true;
   }

   bool have_profiles = (named_profiles.size() > 0);
   AppendMenu(menu, MF_STRING | (too_many_profiles?MF_GRAYED:0), WM_Tray_Profile_Create, L"&Create New Named Profile...");
   AppendMenu(menu, MF_STRING | MF_POPUP | (have_profiles?0:MF_GRAYED), (UINT_PTR)profile_update, L"&Overwrite Named Profile");
   AppendMenu(menu, MF_STRING | MF_POPUP | (have_profiles?0:MF_GRAYED), (UINT_PTR)profile_delete, L"&Delete Named Profile");

   AppendMenu(menu, MF_SEPARATOR, 0, 0);

   AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)options, L"&Options");

   // Let them quit the program if they want  :)
   AppendMenu(menu, MF_STRING, WM_Tray_Exit, L"E&xit");

   return menu;
}

LRESULT DesktopSaverGui::message_menu(WPARAM choice)
{
   switch (choice)
   {
   case WM_Tray_History_Clear:
      {
         if (ASK_QUESTION(L"Are you sure you want to erase your icon position history?\n"
            L"(Your named profiles will remain intact)."))
         {
            m_saver->ClearHistory();
         }

         break;
      }

   case WM_Tray_Exit:
      { 
         SendMessage(m_hwnd, WM_DESTROY, 0, 0);
         break;
      }

   case WM_Tray_Profile_Create:
      {
         // This option is only available if there aren't
         // too many named profiles already.

         wstring name = AskForNewProfileName(m_hinstance, m_hwnd);

         // If the user presses cancel in the dialog (or just
         // leaves the box blank), it comes back empty.
         if (name == L"") break;

         // Check that this (case insensitive) name doesn't already exist.
         bool duplicate = false;
         wstring duplicate_profile_name;

         const HistoryList history = m_saver->NamedProfiles();
         for (size_t i = 0; i < history.size(); ++i)
         {
            wstring name_lower = name;
            std::transform(name.begin(), name.end(), name_lower.begin(), tolower);

            wstring other = history[i].GetName();

            wstring other_lower = other;
            std::transform(other.begin(), other.end(), other_lower.begin(), tolower);

            if (name_lower == other_lower)
            {
               duplicate = true;
               duplicate_profile_name = other;
               break;
            }
         }

         if (duplicate)
         {
            if (ASK_QUESTION(L"A profile with the name '" << duplicate_profile_name << L"' already exists.  Overwrite?"))
            { m_saver->NamedProfileOverwrite(duplicate_profile_name); }
         }
         else { m_saver->NamedProfileAdd(name); }

         break;
      }

   case WM_Tray_On_Startup:
      {
         m_saver->SetRunOnStartup(!m_saver->GetRunOnStartup());
         break;
      }

   case WM_Tray_Disable_History:
      {
         PollRate current = m_saver->GetPollRate();

         if (current != DisableHistory
            && ASK_QUESTION(L"Disabling your history will erase all history snapshots.  Continue?\n"
            L"(Your named profiles will remain intact)."))
         {
            // We must set the poll rate before clearing the history
            // otherwise a poll will occur *just* after the clear
            // and it won't be stopped by the disable bit.
            m_saver->SetPollRate(DisableHistory);
            update_timer();

            m_saver->ClearHistory();
         }
         break;
      }

   case WM_Tray_Poll_Endpoints: m_saver->SetPollRate(PollEndpoints); update_timer(); break;
   case WM_Tray_Poll_Interval1: m_saver->SetPollRate(Interval1);     update_timer(); break;
   case WM_Tray_Poll_Interval2: m_saver->SetPollRate(Interval2);     update_timer(); break;
   case WM_Tray_Poll_Interval3: m_saver->SetPollRate(Interval3);     update_timer(); break;
   case WM_Tray_Poll_Interval4: m_saver->SetPollRate(Interval4);     update_timer(); break;

   default:
      {
         if (choice < WM_Lookup_Begin) break;
         if (choice >= WM_Tray_Profile_Delete + DesktopSaver::MaxProfileCount) break;

         bool handled = false;

         const HistoryList &history = m_saver->History();
         const HistoryList &named_profiles = m_saver->NamedProfiles();

         // History selection
         if (choice >= WM_Tray_History + 0 && choice < WM_Tray_History + DesktopSaver::MaxIconHistoryCount)
         {
            int menu_choice = ((UINT)choice - WM_Tray_History);
            int history_choice = int(history.size() - menu_choice - 1);

            m_saver->RestoreHistory(history[history_choice]);
            handled = true;
         }

         if (choice >= WM_Tray_Named_Profile + 0 && choice < WM_Tray_Named_Profile + DesktopSaver::MaxProfileCount)
         {
            int menu_choice = ((UINT)choice - WM_Tray_Named_Profile);
            int profile_choice = int(named_profiles.size() - menu_choice - 1);

            m_saver->RestoreHistory(named_profiles[profile_choice]);
            handled = true;
         }

         if (choice >= WM_Tray_Profile_Update + 0 && choice < WM_Tray_Profile_Update + DesktopSaver::MaxProfileCount)
         {
            int menu_choice = ((UINT)choice - WM_Tray_Profile_Update);
            int profile_choice = int(named_profiles.size() - menu_choice - 1);

            wstring name = named_profiles[profile_choice].GetName();

            if (ASK_QUESTION(L"Overwrite '" << name << L"' profile with current desktop snapshot?"))
            {
               m_saver->NamedProfileOverwrite(name);
            }

            handled = true;
         }

         if (choice >= WM_Tray_Profile_Delete + 0 && choice < WM_Tray_Profile_Delete + DesktopSaver::MaxProfileCount)
         {
            int menu_choice = ((UINT)choice - WM_Tray_Profile_Delete);
            int profile_choice = int(named_profiles.size() - menu_choice - 1);

            wstring name = named_profiles[profile_choice].GetName();

            if (ASK_QUESTION(L"Are you sure you want to delete the '" << name << L"' profile?"))
            {
               m_saver->NamedProfileDelete(name);
            }

            handled = true;
         }

         if (!handled) STANDARD_ERROR(L"Unexpected 'choice' in popup menu");
      }

   } // switch

   return 0;
}

void DesktopSaverGui::update_timer()
{
   KillTimer(m_hwnd, m_timer_id);

   UINT timer_delay = m_saver->GetPollRateMilliseconds();
   if (timer_delay == 0) return;

   if (!SetTimer(m_hwnd, m_timer_id, timer_delay, (TIMERPROC)0))
   {
      INTERNAL_ERROR(L"Couldn't set polling timer!");
      exit(1);
   }
}
