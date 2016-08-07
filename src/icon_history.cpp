// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed

#include "file_reader.h"
#include "icon_history.h"
#include "string_util.h"

#include <windows.h>
#include "saver.h"

#include <sstream>
using namespace std;

const wstring IconHistory::named_identifier(L"named_profile");

IconHistory::IconHistory() : m_named_profile(false), m_name(L"Initial History") { }

bool IconHistory::Deserialize(FileReader *fr)
{
   m_icons.clear();
   m_named_profile = false;

   // Read the header
   wstring new_name = fr->ReadLine();
   if (new_name.length() <= 0) return false;

   // If this is a named profile, the first
   // string will be a special identifier.  The
   // next line is always the profile's name.
   if (new_name == named_identifier)
   {
      m_named_profile = true;

      new_name = fr->ReadLine();
      if (new_name.length() <= 0) return false;
   }
   m_name = new_name;

   // Parse the icon count using istringstreams
   int icon_count = 0;
   wistringstream icon_count_stream(fr->ReadLine());
   icon_count_stream >> icon_count;

   // Don't check for (icon_count > 0), because
   // that's actually perfectly acceptable.

   // Parse each individual icon
   for (int i = 0; i < icon_count; ++i)
   {
      Icon icon;

      icon.name = fr->ReadLine();
      wistringstream x_stream(fr->ReadLine());
      wistringstream y_stream(fr->ReadLine());

      x_stream >> icon.x;
      y_stream >> icon.y;

      if (!x_stream.bad() && !y_stream.bad() && icon.name.length() > 0)
      {
         AddIcon(icon);
      }
      else
      {
         STANDARD_ERROR(L"There was a problem reading from the history file.  This should fix itself automatically, but some profiles may have been lost.");
         return false;
      }
   }

   return true;
}

void IconHistory::AddIcon(Icon icon)
{
   // This will fail on duplicates (see "KNOWN ISSUE" for
   // struct Icon in icon_history.h), but we ignore it.
   m_icons.insert(icon);
}

void IconHistory::CalculateName(const IconHistory &previous_history)
{
   int iconsAdd = 0;
   int iconsDel = 0;
   int iconsMov = 0;

   wstring addName;
   wstring delName;
   wstring movName;

   // Check first in one direction for moved and added icons
   for (const auto &i : m_icons)
   {
      bool found = false;
      for (const auto &j : previous_history.m_icons)
      {
         if (i.name != j.name) continue;

         found = true;
         if (i.x != j.x || i.y != j.y) { iconsMov++; movName = i.name; }
         break;
      }

      if (found) continue;
      iconsAdd++;
      addName = i.name;
   }

   // Now check the other direction for deleted icons
   for (const auto &i : previous_history.m_icons)
   {
      bool found = false;
      for (const auto &j : m_icons)
      {
         if (i.name != j.name) continue;
         found = true;
         break;
      }
      if (found) continue;

      iconsDel++;
      delName = i.name;
   }

   // Trim down super-long filenames for display purposes
   const static wstring::size_type MaxNameLength = 30;
   const static wstring ellipsis = L"...";
   if (addName.length() > MaxNameLength) addName = addName.substr(0, MaxNameLength) + ellipsis;
   if (delName.length() > MaxNameLength) delName = delName.substr(0, MaxNameLength) + ellipsis;
   if (movName.length() > MaxNameLength) movName = movName.substr(0, MaxNameLength) + ellipsis;

   // Default to more generic messages, but let
   // specific one-icon messages pre-empt
   wstring extra = L"";
   wstring extra_with_parens = L"";
   if (iconsAdd > 0) extra = WSTRING(iconsAdd << L" Added");
   if (iconsDel > 0) extra = WSTRING(iconsDel << L" Deleted");
   if (iconsAdd > 0 && iconsDel > 0) extra = WSTRING(iconsAdd << L" Added, " << iconsDel << L" Deleted");
   if (extra.length() > 0) extra_with_parens = L" (" + extra + L")";

   if (iconsMov > 0) m_name = WSTRING(iconsMov << L" Moved" << extra_with_parens);
   if (iconsMov == 1) m_name = WSTRING(L"'" << movName << L"' Moved" << extra_with_parens);

   if (iconsMov == 0) m_name = extra;
   if (iconsMov == 0 && iconsAdd == 1 && iconsDel == 0) m_name = WSTRING(L"'" << addName << L"' Added");
   if (iconsMov == 0 && iconsAdd == 0 && iconsDel == 1) m_name = WSTRING(L"'" << delName << L"' Deleted");
}

bool IconHistory::Identical(const IconHistory &other) const
{
   // Match the icon set in both directions.  This will
   // cover additions, deletions, and moved icons.

   for (const auto &i : m_icons)
   {
      bool found = false;
      for (const auto &j : other.m_icons)
      {
         if (i.name != j.name || i.x != j.x || i.y != j.y) continue;
         found = true;
         break;
      }
      if (!found) return false;
   }

   for (const auto &i : other.m_icons)
   {
      bool found = false;
      for (const auto &j : m_icons)
      {
         if (i.name != j.name || i.x != j.x || i.y != j.y) continue;
         found = true;
         break;
      }
      if (!found) return false;
   }

   return true;
}

wstring IconHistory::Serialize() const
{
   wostringstream os;
   os << L": =============================================" << endl;
   os << L": IconHistory \"" << m_name << L"\"" << endl << endl;

   if (IsNamedProfile()) { os << named_identifier << endl; }

   os << m_name << endl;
   os << (unsigned int)m_icons.size() << endl;
   os << endl;

   // Write each icon
   int counter = 0;
   for (const auto &i : m_icons)
   {
      os << i.name << endl;
      os << i.x << endl;
      os << i.y << endl;
      os << endl;
   }

   os << endl;
   return os.str();
}

