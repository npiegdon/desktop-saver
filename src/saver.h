// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

#include <string>
#include <vector>
#include "icon_history.h"
#include "string_util.h"

#define INTERNAL_ERROR(err) MessageBox(0, WSTRING(L"DesktopSaver Error in file '" << __FILE__ << L"', line " << __LINE__ << L":\n" << err).c_str(), L"DesktopSaver Error!", MB_ICONERROR)
#define STANDARD_ERROR(err) MessageBox(0, WSTRING(err).c_str(), L"DesktopSaver Error!", MB_ICONERROR | MB_APPLMODAL)
#define ASK_QUESTION(str)  (MessageBox(0, WSTRING(str).c_str(), L"DesktopSaver", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == IDYES)

typedef std::vector<IconHistory> HistoryList;
typedef HistoryList::iterator MalleableHistoryIter;

typedef HistoryList::const_iterator HistoryIter;
typedef HistoryList::const_reverse_iterator HistoryRevIter;

enum PollRate { DisableHistory, PollEndpoints, Interval1, Interval2, Interval3, Interval4, PollRate_Max };

class DesktopSaver
{
public:
   DesktopSaver();

   static const size_t MaxProfileCount = 10;
   static const size_t MaxIconHistoryCount = 25;

   void PollDesktopIcons();
   void RestoreHistory(const IconHistory history);

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

   const HistoryList &History() const { return m_history; }
   const HistoryList &NamedProfiles() const { return m_namedProfiles; }
   void ClearHistory();

private:
   // Save our history slices to file, to be read back next time
   void serialize() const;
   void deserialize();

   static void RestoreHistoryOnce(const IconHistory &history);
   static IconHistory ReadDesktop();

   PollRate read_poll_rate() const;
   void write_poll_rate();

   // We cache the poll rate inside the object instead of
   // reading from the registry each time because this
   // is required during polls -- and polls should be as
   // lightweight as possible
   PollRate m_rate;

   std::wstring m_historyPath;
   HistoryList m_history, m_namedProfiles;
};
