// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __SAVER_H
#define __SAVER_H

#include <string>
#include <vector>
#include "icon_history.h"

// A handy ostringstream macro
#ifndef STRING
#include <sstream>
#define STRING(v) ((static_cast<std::ostringstream&>(std::ostringstream().flush() << v)).str())
#endif

#define INTERNAL_ERROR(err) MessageBox(0, STRING("DesktopSaver Error in file '" << __FILE__ << "', line " << __LINE__ << ":\n" << err).c_str(), "DesktopSaver Error!", MB_ICONERROR)
#define STANDARD_ERROR(err) MessageBox(0, STRING(err).c_str(), "DesktopSaver Error!", MB_ICONERROR | MB_APPLMODAL)
#define ASK_QUESTION(str)  (MessageBox(0, STRING(str).c_str(), "DesktopSaver", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == IDYES)

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
   DesktopSaver(std::string app_name);

   static const size_t MaxProfileCount = 10;
   static const size_t MaxIconHistoryCount = 25;

   void PollDesktopIcons();
   void RestoreHistory(IconHistory history);

   void NamedProfileAdd(const std::string &name);
   void NamedProfileOverwrite(const std::string &name);
   void NamedProfileDelete(const std::string &name);

   bool GetRunOnStartup() const;
   void SetRunOnStartup(bool run);

   PollRate GetPollRate() const { return m_rate; }
   void SetPollRate(PollRate r);

   unsigned int GetPollRateMilliseconds() const;

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

   std::string m_app_name;

   std::string m_history_filename;
   HistoryList m_history_list;
   HistoryList m_named_profile_list;
};

#endif
