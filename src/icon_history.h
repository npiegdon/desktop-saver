// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

#include <string>
#include <set>

// Forward Declarations
class FileReader;
class FileReaderNonUnicode;

// Contains information about one icon that lives on the desktop.
//
// Ideally, we might also include an icon handle in this structure
// to combat the "known issue" below, but handles aren't constant
// over Windows restarts.
//
// KNOWN ISSUE: If the user doesn't have "show file extensions" turned
// on, duplicate named icons are possible.  Over the course of restarting
// explorer, like-named icons may be be ordered differently, swapping
// their final restored positions.
//
// More than likely, only the "first" encountered like-named icon will
// even be recorded (and subsequently restored) properly.  The rest of
// the like-named icons will be ignored.
struct Icon
{
   std::wstring name;
   long x, y;

   bool operator <(const Icon &i) const { return (name < i.name); }
};

// Keeps track of one desktop icon positioning instance
class IconHistory
{
public:
   // Creates a blank history.  Follow up with several add_icon() calls,
   // and finish with a calculate_name() call.
   IconHistory();

   std::wstring GetName() const { return m_name; }
   void CalculateName(const IconHistory &previous_history);
   void SetProfileName(const std::wstring &name) { m_name = name; m_named_profile = !m_name.empty(); }

   bool IsNamedProfile() const { return m_named_profile; }

   void AddIcon(Icon icon);
   bool Identical(const IconHistory &other) const;

   const std::set<Icon> GetIcons() const { return m_icons; }

   // Restore icon history from file.  Returns
   // true on success, false if the FileReader couldn't
   // supply enough input (for the "last in the file" case)
   bool Deserialize(FileReader *fr);

   std::wstring Serialize() const;

private:
   std::set<Icon> m_icons;

   bool m_named_profile;
   std::wstring m_name;

   const static std::wstring named_identifier;
};
