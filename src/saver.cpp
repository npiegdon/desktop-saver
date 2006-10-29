// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>

#include "saver.h"
#include "file_reader.h"
#include "registry.h"
#include "version.h"

#include <fstream>
#include <sstream>
using namespace std;

DesktopSaver::DesktopSaver(string app_name)
{
   m_app_name = app_name;

   // Grab our polling rate from the registry
   m_rate = read_poll_rate();

   // Set the location of the history file by starting with a default
   m_history_filename = "icon_history.txt";

   // Add the shell folder path if we've got one
   TCHAR sh_path[MAX_PATH];
   HRESULT hr = SHGetFolderPath(0,
      CSIDL_APPDATA | CSIDL_FLAG_CREATE,
      0, SHGFP_TYPE_CURRENT,
      sh_path);

   if (SUCCEEDED(hr))
   {
      // Attempt to create the directory (in case it doesn't already exist
      string path = STRING(sh_path) + STRING("\\DesktopSaver\\");
      SHCreateDirectoryEx(0, path.c_str(), 0);

      m_history_filename = path + m_history_filename;
   }

   // Load our previous icon history file
   deserialize();

   // Read the desktop immediately to either add the very first history
   // slice to our list (on the very first run ever), or to compare to
   // the last known positions of all the icons (on subsequent runs).
   PollDesktopIcons();
}


void DesktopSaver::deserialize()
{
   FileReader fr(m_history_filename);

   // knock out our old history and named profile list
   m_history_list = HistoryList();
   m_named_profile_list = HistoryList();

   // Read in IconHistory objects until one fails to load
   IconHistory h(false);
   while (h.Deserialize(&fr))
   {
      if (h.IsNamedProfile()) m_named_profile_list.push_back(h);
      else m_history_list.push_back(h);
   }
}

void DesktopSaver::serialize() const
{
   ofstream file(m_history_filename.c_str());

   if (!file.good())
   {
      STANDARD_ERROR("Could not save icon position information to the file:" << endl
         << m_history_filename << endl << endl << "Check that you have write access to"
         << " that location and that the file isn't in use.");

      exit(1);
   }

   file << "#" << endl;
   file << "# " << m_app_name << " " << DESKTOPSAVER_VERSION << " icon history file" << endl;
   file << "#" << endl;
   file << endl;

   for (HistoryIter i = m_history_list.begin(); i != m_history_list.end(); ++i)
   { file << i->Serialize() << endl; }

   for (HistoryIter i = m_named_profile_list.begin(); i != m_named_profile_list.end(); ++i)
   { file << i->Serialize() << endl; }

   file.close();
}

void DesktopSaver::NamedProfileAdd(const std::string &name)
{
   IconHistory i = get_desktop(true);
   i.ForceNamedProfileName(name);

   m_named_profile_list.push_back(i);

   // After changes, we should write our results out to disk.
   serialize();
}

void DesktopSaver::NamedProfileOverwrite(const std::string &name)
{
   // Find the profile in question
   MalleableHistoryIter i;
   for (i = m_named_profile_list.begin(); i != m_named_profile_list.end(); ++i)
   {
      if (i->GetName() == name) break;
   }
   if (i == m_named_profile_list.end()) INTERNAL_ERROR("Couldn't find profile '" << name << "' to overwrite.");

   *i = get_desktop(true);
   i->ForceNamedProfileName(name);

   // After changes, we should write our results out to disk.
   serialize();
}

void DesktopSaver::NamedProfileDelete(const std::string &name)
{
   // Find the profile in question
   MalleableHistoryIter i;
   for (i = m_named_profile_list.begin(); i != m_named_profile_list.end(); ++i)
   {
      if (i->GetName() == name) break;
   }
   if (i == m_named_profile_list.end()) INTERNAL_ERROR("Couldn't find profile '" << name << "' to delete.");

   m_named_profile_list.erase(i);

   // After changes, we should write our results out to disk.
   serialize();
}

IconHistory DesktopSaver::get_desktop(bool named_profile)
{
   // Much of the following is based on a codeguru.com article written
   // by Jeroen-bart Engelen (who in turn used a lot of advice from others).

   IconHistory snapshot(named_profile);

   // Find the desktop listview window
   HWND program_manager = FindWindow(NULL, ("Program Manager"));                     if (program_manager == NULL) return snapshot;
   HWND desktop = FindWindowEx(program_manager, NULL, ("SHELLDLL_DefView"), NULL);   if (desktop == NULL) return snapshot;
   HWND listview = FindWindowEx(desktop, NULL, ("SysListView32"), NULL);             if (listview == NULL) return snapshot;

   // Find out how many icons we have
   unsigned long icon_count = static_cast<unsigned long>(SendMessage(listview, LVM_GETITEMCOUNT, 0, 0));
   if (icon_count == 0) return snapshot;

   // Get a handle to the root (explorer.exe) process for the next step
   DWORD explorer_id;
   GetWindowThreadProcessId(listview, &explorer_id);

   HANDLE explorer = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, explorer_id);
   if (explorer == NULL) return snapshot;

   // Allocate some shared memory for message passing
   void *remote_pos  =                    VirtualAllocEx(explorer, NULL, sizeof(POINT),             MEM_COMMIT, PAGE_READWRITE);
   void *remote_item =                    VirtualAllocEx(explorer, NULL, sizeof(LVITEM),            MEM_COMMIT, PAGE_READWRITE);
   char *remote_text = static_cast<char*>(VirtualAllocEx(explorer, NULL, sizeof(char)*(MAX_PATH+1), MEM_COMMIT, PAGE_READWRITE));

   // Grab each one of the icons from the desktop
   for (unsigned long i = 0; i < icon_count; ++i)
   {
      Icon icon;
      LRESULT ret;

      // Get the icon's position
      ret = SendMessage(listview, LVM_GETITEMPOSITION, i, reinterpret_cast<LPARAM>(remote_pos));
      if (ret != TRUE) continue;

      POINT local_pos;
      ReadProcessMemory(explorer, remote_pos, &local_pos, sizeof(POINT), NULL);
      icon.x = local_pos.x;
      icon.y = local_pos.y;

      // Get ready to grab the icon label
      LVITEM local_item;
      local_item.iSubItem = 0;
      local_item.cchTextMax = MAX_PATH;
      local_item.mask = LVIF_TEXT;
      local_item.pszText = (LPTSTR)remote_text;

      WriteProcessMemory(explorer, remote_item, &local_item, sizeof(LVITEM), NULL);

      // Grab the icon label
      ret = SendMessage(listview, LVM_GETITEMTEXT, i, (LPARAM)remote_item);
      if (ret < 0) continue;

      // We can skip the step of reading back the 'item', because the 
      // only thing we care about is the newly-changed 'text' buffer.
      char local_text[MAX_PATH+1];
      ReadProcessMemory(explorer, remote_text, &local_text, sizeof(local_text), NULL);

      icon.name = local_text;

      // Now that we've put together an entire icon record, dump it
      // into our brand new history slice
      snapshot.AddIcon(icon);
   }

   // Clean up our cross-process resources
   VirtualFreeEx(explorer, remote_pos,  0, MEM_RELEASE);
   VirtualFreeEx(explorer, remote_item, 0, MEM_RELEASE);
   VirtualFreeEx(explorer, remote_text, 0, MEM_RELEASE);
   CloseHandle(explorer);

   return snapshot;
}

void DesktopSaver::PollDesktopIcons()
{
   if (GetPollRate() == DisableHistory)
   {
      m_history_list.clear();
      return;
   }

   IconHistory history = get_desktop(false);

   if (m_history_list.size() > 0)
   {
      // Skip adding this slice to our history list if nothing has changed
      if (history.Identical(m_history_list.back())) return;

      // If we have any previous history slices, we can generate
      // a sort of diff'ed name for the slice, (otherwise it will
      // just use the default history name "Initial History")
      history.CalculateName(m_history_list.back());
   }

   // Add it to our history list
   m_history_list.push_back(history);

   // Limit the history list length
   while (m_history_list.size() > MaxIconHistoryCount)
   {
      m_history_list.erase(m_history_list.begin());
   }

   // Now that we're all finished with a polling cycle, write our
   // new results out to disk.
   serialize();
}

void DesktopSaver::RestoreHistory(IconHistory history)
{
   // Much of the following is based on a codeguru.com article written
   // by Jeroen-bart Engelen (who in turn used a lot of advice from others).

   // Find the desktop listview window
   HWND program_manager = FindWindow(NULL, ("Program Manager"));                     if (program_manager == NULL) return;
   HWND desktop = FindWindowEx(program_manager, NULL, ("SHELLDLL_DefView"), NULL);   if (desktop == NULL) return;
   HWND listview = FindWindowEx(desktop, NULL, ("SysListView32"), NULL);             if (listview == NULL) return;

   // Find out how many icons we have
   unsigned long icon_count = static_cast<unsigned long>(SendMessage(listview, LVM_GETITEMCOUNT, 0, 0));
   if (icon_count == 0) return;

   // Get a handle to the root (explorer.exe) process for the next step
   DWORD explorer_id;
   GetWindowThreadProcessId(listview, &explorer_id);

   HANDLE explorer = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, explorer_id);
   if (explorer == NULL) return;

   // Allocate some shared memory for message passing
   void *remote_item =                    VirtualAllocEx(explorer, NULL, sizeof(LVITEM),            MEM_COMMIT, PAGE_READWRITE);
   char *remote_text = static_cast<char*>(VirtualAllocEx(explorer, NULL, sizeof(char)*(MAX_PATH+1), MEM_COMMIT, PAGE_READWRITE));

   for (unsigned long i = 0; i < icon_count; ++i)
   {
      // Get ready to grab the icon label
      LVITEM local_item;
      local_item.iSubItem = 0;
      local_item.cchTextMax = MAX_PATH;
      local_item.mask = LVIF_TEXT;
      local_item.pszText = (LPTSTR)remote_text;

      WriteProcessMemory(explorer, remote_item, &local_item, sizeof(LVITEM), NULL);

      // Grab the icon label
      LRESULT ret = SendMessage(listview, LVM_GETITEMTEXT, i, (LPARAM)remote_item);
      if (ret < 0) continue;

      // We can skip the step of reading back the 'item', because the 
      // only thing we care about is the newly-changed 'text' buffer.
      char icon_name_buf[MAX_PATH+1];
      ReadProcessMemory(explorer, remote_text, &icon_name_buf, sizeof(icon_name_buf), NULL);
      const string icon_name(icon_name_buf);

      // Run through the saved list of icons
      const IconList icons = history.GetIcons();
      for (IconIter j = icons.begin(); j != icons.end(); ++j)
      {
         // If the saved and current names match, move the icon
         if (j->name == icon_name) SendMessage(listview, LVM_SETITEMPOSITION, i, MAKELPARAM(j->x, j->y));
      }
   }

   // Clean up our cross-process resources
   VirtualFreeEx(explorer, remote_item, 0, MEM_RELEASE);
   VirtualFreeEx(explorer, remote_text, 0, MEM_RELEASE);
   CloseHandle(explorer);

   // Force a poll just afterwards to log the new history
   PollDesktopIcons();
}

void DesktopSaver::ClearHistory()
{
   m_history_list.clear();

   // As an added security measure, we should write
   // the history file out immediately to erase any
   // remaining "evidence"
   serialize();

   // Force a poll just afterwards to log the new history
   PollDesktopIcons();
}

bool DesktopSaver::GetRunOnStartup() const
{
   Registry r(Registry::CU_Run, "", "");

   string result;

   const string no_result = "__no_result_found";
   r.Read(m_app_name, &result, no_result);

   return (result != no_result);
}

void DesktopSaver::SetRunOnStartup(bool run)
{
   Registry r(Registry::CU_Run, "", "");

   if (!run)
   {
      r.Delete(m_app_name);
      return;
   }
   else
   {
      // Strip whitespace off the ends of the command-line string
      // HKCU/.../Run won't run a command that has a trailing space
      // after it.
      string command(GetCommandLine());
      while (command.length() > 0 && isspace(command[0]))                  command = command.substr(1, command.length()-1);
      while (command.length() > 0 && isspace(command[command.length()-1])) command = command.substr(0, command.length()-1);

      r.Write (m_app_name, command);
   }
}

PollRate DesktopSaver::read_poll_rate() const
{
   Registry r(Registry::CurrentUser, m_app_name, "");

   int poll_rate;

   r.Read("poll_rate", &poll_rate, (int)DefaultPollRate);

   // Force to our enumeration values ONLY
   switch (poll_rate)
   {
   case DisableHistory:
   case PollEndpoints:
   case Interval1:
   case Interval2:
   case Interval3:
   case Interval4:
      return (PollRate)poll_rate;

   default:
      return DefaultPollRate;
   }

}

void DesktopSaver::SetPollRate(PollRate r)
{
   m_rate = r;
   write_poll_rate();
}  

void DesktopSaver::write_poll_rate()
{
   Registry r(Registry::CurrentUser, m_app_name, "");
   r.Write("poll_rate", (int)m_rate);
}

unsigned int DesktopSaver::GetPollRateMilliseconds() const
{
   unsigned int timer_delay;

   // Set the number of minutes based on the option chosen
   PollRate p = GetPollRate();
   switch (p)
   {
   case DisableHistory: timer_delay = 0;   break;
   case PollEndpoints:  timer_delay = 0;   break;
   case Interval1:      timer_delay = 5;   break;
   case Interval2:      timer_delay = 20;  break;
   case Interval3:      timer_delay = 60;  break;
   case Interval4:      timer_delay = 360; break;

   default:             timer_delay = 0;   break;
   }

   timer_delay *= 60;   // (seconds per minute)
   timer_delay *= 1000; // (milliseconds per second)

   return timer_delay;
}
