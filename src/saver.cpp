// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>

#include "saver.h"
#include "file_reader.h"
#include "registry.h"
#include "version.h"

#include <algorithm>
#include <fstream>
#include <sstream>
using namespace std;

DesktopSaver::DesktopSaver(const wstring &app_name)
{
   m_app_name = app_name;

   // Grab our polling rate from the registry
   m_rate = read_poll_rate();

   // Set the location of the history file by starting with a default
   m_history_filename_UNICODE = L"icon_history_2.txt";

   // Add the shell folder path if we've got one
   TCHAR sh_path[MAX_PATH];
   HRESULT hr = SHGetFolderPath(0,
      CSIDL_APPDATA | CSIDL_FLAG_CREATE,
      0, SHGFP_TYPE_CURRENT,
      sh_path);

   if (SUCCEEDED(hr))
   {
      // Attempt to create the directory (in case it doesn't already exist
      wstring path = wstring(sh_path) + L"\\DesktopSaver\\";
      SHCreateDirectoryEx(0, path.c_str(), 0);

      m_history_filename_ANSI = path + m_history_filename_ANSI;
      m_history_filename_UNICODE = path + m_history_filename_UNICODE;
   }

   // Load our previous icon history file
   deserialize();

   // Read the desktop immediately to either add the very first history
   // slice to our list (on the very first run ever), or to compare to
   // the last known positions of all the icons (on subsequent runs).
   PollDesktopIcons();

   const std::wstring autostart = GetAutostartProfileName();
   if (autostart.length() > 0)
   {
      for (HistoryIter h = m_named_profile_list.begin(); h != m_named_profile_list.end(); ++h)
      {
         if (h->GetName() != autostart) continue;

         RestoreHistory(*h);
         break;
      }   
   }

}

void DesktopSaver::deserialize()
{
   // knock out our old history and named profile list
   m_history_list = HistoryList();
   m_named_profile_list = HistoryList();

    FileReader fr(m_history_filename_UNICODE);

    // Read in IconHistory objects until one fails to load
    IconHistory h;
    while (h.Deserialize(&fr))
    {
        if (h.IsNamedProfile()) m_named_profile_list.push_back(h);
        else m_history_list.push_back(h);
    }

}

void DesktopSaver::serialize() const
{
   // NOTE: The multi-byte output requires this
   const static wstring end = L"\r\n";

   wostringstream file;

   file << L":" << end;
   file << L": " << m_app_name << L" " << DESKTOPSAVER_VERSION << L" icon history file" << end;
   file << L":" << end;
   file << end;

   for (HistoryIter i = m_history_list.begin(); i != m_history_list.end(); ++i)
   { file << i->Serialize() << end; }

   for (HistoryIter i = m_named_profile_list.begin(); i != m_named_profile_list.end(); ++i)
   { file << i->Serialize() << end; }

   FILE *f = 0;
   errno_t err = _wfopen_s(&f, m_history_filename_UNICODE.c_str(), L"wb");

   if (err != 0 || f == 0)
   {
      STANDARD_ERROR(L"Could not save icon position information to the file:" << endl
         << m_history_filename_UNICODE << endl << endl
         << L"Check that you have write access to that location and that the file isn't in use.");

      exit(1);
   }

   fputws(file.str().c_str(), f);
   fclose(f);
}

void DesktopSaver::NamedProfileAdd(const std::wstring &name)
{
   IconHistory i = ReadDesktop();
   i.SetProfileName(name);

   m_named_profile_list.push_back(i);

   // After changes, we should write our results out to disk.
   serialize();
}

void DesktopSaver::NamedProfileOverwrite(const std::wstring &name)
{
   // Find the profile in question
   MalleableHistoryIter i;
   for (i = m_named_profile_list.begin(); i != m_named_profile_list.end(); ++i)
   {
      if (i->GetName() == name) break;
   }
   if (i == m_named_profile_list.end()) INTERNAL_ERROR(L"Couldn't find profile '" << name << L"' to overwrite.");

   *i = ReadDesktop();
   i->SetProfileName(name);

   // After changes, we should write our results out to disk.
   serialize();
}

void DesktopSaver::NamedProfileDelete(const std::wstring &name)
{
   // Find the profile in question
   MalleableHistoryIter i;
   for (i = m_named_profile_list.begin(); i != m_named_profile_list.end(); ++i)
   {
      if (i->GetName() == name) break;
   }
   if (i == m_named_profile_list.end()) INTERNAL_ERROR(L"Couldn't find profile '" << name << "' to delete.");

   m_named_profile_list.erase(i);

   // After changes, we should write our results out to disk.
   serialize();
}

void DesktopSaver::NamedProfileAutostart(const std::wstring &name)
{
   Registry r(Registry::CurrentUser, m_app_name, L"");

   // Setting it to the same value is how you turn off auto-start
   if (GetAutostartProfileName() == name) r.Delete(L"profile_autostart");
   else r.Write(L"profile_autostart", name);
}

static BOOL CALLBACK WorkerWithShellDefView(HWND child, LPARAM lparam)
{
   wchar_t name[64];
   GetClassName(child, name, 64);
   if (wcscmp(name, L"WorkerW") != 0) return TRUE;

   HWND defView = FindWindowEx(child, NULL, L"SHELLDLL_DefView", NULL);
   if (defView == NULL) return TRUE;

   *reinterpret_cast<HWND*>(lparam) = defView;
   return FALSE;
}

IconHistory DesktopSaver::ReadDesktop()
{
   IconHistory snapshot;

   HWND desktop = GetShellWindow();
   if (desktop == NULL) return snapshot;

   HWND desktopInner = FindWindowEx(desktop, NULL, L"SHELLDLL_DefView", NULL);

   // From http://stackoverflow.com/a/9352551
   // If a live wallpaper is used, the desktop is found under a WorkerW (with a SHELLDLL_DefView child) instead of Progman
   if (desktopInner == NULL) EnumWindows(WorkerWithShellDefView, reinterpret_cast<LPARAM>(&desktopInner));
   if (desktopInner == NULL) return snapshot;

   HWND listview = FindWindowEx(desktopInner, NULL, L"SysListView32", NULL);
   if (listview == NULL) return snapshot;

   // Find out how many icons we have
   auto icon_count = ListView_GetItemCount(listview);
   if (icon_count == 0) return snapshot;

   DWORD explorer_id;
   GetWindowThreadProcessId(listview, &explorer_id);

   HANDLE explorer = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, explorer_id);
   if (explorer == NULL) return snapshot;

   // Allocate some shared memory for message passing
   void    *remote_pos  =                       VirtualAllocEx(explorer, NULL, sizeof(POINT),                MEM_COMMIT, PAGE_READWRITE);
   void    *remote_item =                       VirtualAllocEx(explorer, NULL, sizeof(LVITEM),               MEM_COMMIT, PAGE_READWRITE);
   wchar_t *remote_text = static_cast<wchar_t*>(VirtualAllocEx(explorer, NULL, sizeof(wchar_t)*(MAX_PATH+1), MEM_COMMIT, PAGE_READWRITE));

   for (int i = 0; i < icon_count; ++i)
   {
      // Get the icon's position
      if (ListView_GetItemPosition(listview, i, remote_pos) != TRUE) continue;

      POINT local_pos;
      ReadProcessMemory(explorer, remote_pos, &local_pos, sizeof(POINT), NULL);

      // Get ready to grab the icon label
      LVITEM local_item;
      local_item.iSubItem = 0;
      local_item.cchTextMax = MAX_PATH;
      local_item.mask = LVIF_TEXT;
      local_item.pszText = (LPTSTR)remote_text;

      WriteProcessMemory(explorer, remote_item, &local_item, sizeof(LVITEM), NULL);

      // Grab the icon label
      if (SendMessage(listview, LVM_GETITEMTEXT, i, (LPARAM)remote_item) < 0) continue;

      // We can skip the step of reading back the 'item', because the 
      // only thing we care about is the newly-changed 'text' buffer.
      wchar_t local_text[MAX_PATH+1];
      ReadProcessMemory(explorer, remote_text, &local_text, sizeof(local_text), NULL);

      // Now that we've put together an entire icon record, dump it
      // into our brand new history slice
      snapshot.AddIcon(Icon{ local_text, local_pos.x, local_pos.y });
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
   auto &h = m_history_list;
   if (GetPollRate() == DisableHistory) { h.clear(); return; }

   IconHistory history = ReadDesktop();
   if (h.size() > 0)
   {
      if (history.Identical(h.back())) return;

      // If we have any previous history slices, we can generate a sort of diff'ed name for
      // the slice, (otherwise it will just use the default history name "Initial History")
      history.CalculateName(h.back());

      // If this looks like anything we've seen before, no reason to clutter the list with a bunch of back-and-forth
      h.erase(remove_if(h.begin(), h.end(), [&history](const IconHistory &me) { return me.Identical(history); }), h.end());
   }

   h.push_back(history);
   while (h.size() > MaxIconHistoryCount) h.erase(h.begin());

   serialize();
}

void DesktopSaver::RestoreHistory(const IconHistory history)
{
   IconHistory previous = ReadDesktop();

   // Sometimes shimmying icons around bumps others into places they shouldn't be.  This
   // happens when the new location is already occupied.  This is a little naive, but we
   // just try to do it for a while until we reach a stable state.
   for (int i = 0; i < 3; ++i)
   {
      RestoreHistoryOnce(history);

      const IconHistory current = ReadDesktop();
      if (current.Identical(previous)) return;

      previous = current;
   }

   // Force a poll just afterwards to log the new history
   PollDesktopIcons();
}

void DesktopSaver::RestoreHistoryOnce(const IconHistory &history)
{
   // Find the desktop listview window
   HWND program_manager = FindWindow(NULL, (L"Program Manager"));                     if (program_manager == NULL) return;
   HWND desktop = FindWindowEx(program_manager, NULL, (L"SHELLDLL_DefView"), NULL);   if (desktop == NULL) return;
   HWND listview = FindWindowEx(desktop, NULL, (L"SysListView32"), NULL);             if (listview == NULL) return;

   // Find out how many icons we have
   auto icon_count = ListView_GetItemCount(listview);
   if (icon_count == 0) return;

   // Get a handle to the root (explorer.exe) process for the next step
   DWORD explorer_id;
   GetWindowThreadProcessId(listview, &explorer_id);

   HANDLE explorer = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, explorer_id);
   if (explorer == NULL) return;

   // Allocate some shared memory for message passing
   void    *remote_item =                       VirtualAllocEx(explorer, NULL, sizeof(LVITEM),               MEM_COMMIT, PAGE_READWRITE);
   wchar_t *remote_text = static_cast<wchar_t*>(VirtualAllocEx(explorer, NULL, sizeof(wchar_t)*(MAX_PATH+1), MEM_COMMIT, PAGE_READWRITE));

   for (int i = 0; i < icon_count; ++i)
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
      wchar_t icon_name_buf[MAX_PATH+1];
      ReadProcessMemory(explorer, remote_text, &icon_name_buf, sizeof(icon_name_buf), NULL);
      const wstring icon_name(icon_name_buf);

      // Run through the saved list of icons
      const set<Icon> icons = history.GetIcons();
      for (const auto &j : icons)
      {
         // If the saved and current names match, move the icon
         if (j.name == icon_name) ListView_SetItemPosition(listview, i, j.x, j.y);
      }
   }

   // Clean up our cross-process resources
   VirtualFreeEx(explorer, remote_item, 0, MEM_RELEASE);
   VirtualFreeEx(explorer, remote_text, 0, MEM_RELEASE);
   CloseHandle(explorer);
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
   Registry r(Registry::CU_Run, L"", L"");

   wstring result;

   const wstring no_result = L"__no_result_found";
   r.Read(m_app_name, &result, no_result);

   return (result != no_result);
}

void DesktopSaver::SetRunOnStartup(bool run)
{
   Registry r(Registry::CU_Run, L"", L"");

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
      wstring command(GetCommandLine());
      while (command.length() > 0 && isspace(command[0]))                  command = command.substr(1, command.length()-1);
      while (command.length() > 0 && isspace(command[command.length()-1])) command = command.substr(0, command.length()-1);

      r.Write (m_app_name, command);
   }
}

PollRate DesktopSaver::read_poll_rate() const
{
   Registry r(Registry::CurrentUser, m_app_name, L"");

   int poll_rate;

   r.Read(L"poll_rate", &poll_rate, (int)DefaultPollRate);

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
   Registry r(Registry::CurrentUser, m_app_name, L"");
   r.Write(L"poll_rate", (int)m_rate);
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

std::wstring DesktopSaver::GetAutostartProfileName() const
{
   Registry r(Registry::CurrentUser, m_app_name, L"");

   std::wstring name;
   r.Read(L"profile_autostart", &name, L"");

   return name;
}
