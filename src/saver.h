// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __SAVER_H
#define __SAVER_H

#include <string>
#include <vector>
#include "icon_history.h"

// A handy ostringstream macro
#ifndef WSTRING
#include <sstream>
#define WSTRING(v) ((static_cast<std::wostringstream&>(std::wostringstream().flush() << v)).str())
#endif

#define INTERNAL_ERROR(err) MessageBox(0, WSTRING(L"DesktopSaver Error in file '" << __FILE__ << L"', line " << __LINE__ << L":\n" << err).c_str(), L"DesktopSaver Error!", MB_ICONERROR)
#define STANDARD_ERROR(err) MessageBox(0, WSTRING(err).c_str(), L"DesktopSaver Error!", MB_ICONERROR | MB_APPLMODAL)
#define ASK_QUESTION(str)  (MessageBox(0, WSTRING(str).c_str(), L"DesktopSaver", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == IDYES)

typedef std::vector<IconHistory> HistoryList;
typedef HistoryList::iterator MalleableHistoryIter;

typedef HistoryList::const_iterator HistoryIter;
typedef HistoryList::const_reverse_iterator HistoryRevIter;

enum PollRate
{
   DisableHistory, PollEndpoints,
   Interval1, Interval2, Interval3, Interval4
};
const static PollRate DefaultPollRate = Interval2;


class DesktopSaver
{
public:
   DesktopSaver(std::wstring app_name);

   static const size_t MaxProfileCount = 10;
   static const size_t MaxIconHistoryCount = 25;

   void PollDesktopIcons();
   void RestoreHistory(IconHistory history);

   void NamedProfileAdd(const std::wstring &name);
   void NamedProfileOverwrite(const std::wstring &name);
   void NamedProfileDelete(const std::wstring &name);
   void NamedProfileAutostart(const std::wstring &name);

   bool GetRunOnStartup() const;
   void SetRunOnStartup(bool run);

   PollRate GetPollRate() const { return m_rate; }
   void SetPollRate(PollRate r);

   unsigned int GetPollRateMilliseconds() const;

   std::wstring GetAutostartProfileName() const;

   const HistoryList &History() const { return m_history_list; }
   const HistoryList &NamedProfiles() const { return m_named_profile_list; }
   void ClearHistory();

private:
   // Save our history slices to file, to be read back next time
   void serialize() const;
   void deserialize();

   IconHistory get_desktop(bool named_profile);

   PollRate read_poll_rate() const;
   void write_poll_rate();

   // We cache the poll rate inside the object instead of
   // reading from the registry each time because this
   // is required during polls -- and polls should be as
   // lightweight as possible
   PollRate m_rate;

   std::wstring m_app_name;

   std::wstring m_history_filename_ANSI;
   std::wstring m_history_filename_UNICODE;

   HistoryList m_history_list;
   HistoryList m_named_profile_list;
};

#endif
