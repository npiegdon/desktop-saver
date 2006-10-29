// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "file_reader.h"
#include "icon_history.h"
#include "string_util.h"

// For STANDARD_ERROR
#include <windows.h>
#include "saver.h"

#include <sstream>
using namespace std;

const wstring IconHistory::named_identifier(L"named_profile");


IconHistory::IconHistory(bool named_profile)
{
   m_named_profile = named_profile;
   m_name = L"Initial History";
}

bool IconHistory::Deserialize(FileReader *fr)
{
   // Reset our icon list;
   m_icons = IconList();
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
         STANDARD_ERROR(L"There was a problem reading from the history file.  This"
            L" should fix itself automatically, but some profiles may have"
            L" been lost.");

         return false;
      }
   }

   return true;
}

bool IconHistory::DeserializeNonUnicode(FileReaderNonUnicode *fr)
{
   // Reset our icon list;
   m_icons = IconList();
   m_named_profile = false;

   // Read the header
   string ANSI_new_name = fr->ReadLine();
   if (ANSI_new_name.length() <= 0) return false;

   Widen<wchar_t> w;
   wstring new_name = w(ANSI_new_name);

   // If this is a named profile, the first
   // string will be a special identifier.  The
   // next line is always the profile's name.
   if (new_name == named_identifier)
   {
      m_named_profile = true;

      ANSI_new_name = fr->ReadLine();
      if (ANSI_new_name.length() <= 0) return false;
   }

   m_name = w(ANSI_new_name);

   // Parse the icon count using istringstreams
   int icon_count = 0;
   istringstream icon_count_stream(fr->ReadLine());
   icon_count_stream >> icon_count;

   // Don't check for (icon_count > 0), because
   // that's actually perfectly acceptable.

   // Parse each individual icon
   for (int i = 0; i < icon_count; ++i)
   {
      Icon icon;

      icon.name = w(fr->ReadLine());
      istringstream x_stream(fr->ReadLine());
      istringstream y_stream(fr->ReadLine());

      x_stream >> icon.x;
      y_stream >> icon.y;

      if (!x_stream.bad() && !y_stream.bad() && icon.name.length() > 0)
      {
         AddIcon(icon);
      }
      else
      {
         STANDARD_ERROR(L"There was a problem reading from the history file.  This"
            L" should fix itself automatically, but some profiles may have"
            L" been lost.");

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
   for (IconIter i = m_icons.begin(); i != m_icons.end(); ++i)
   {
      bool found = false;
      for (IconIter j = previous_history.m_icons.begin(); j != previous_history.m_icons.end(); ++j)
      {
         if (i->name == j->name)
         {
            found = true;
            if (i->x != j->x || i->y != j->y)
            {
               iconsMov++;
               movName = i->name;
            }
            // else two identical icons

            // If we've already found it, we can skip the rest of the list
            break;
         }
         // else two wholly unrelated icons
      }

      if (!found)
      {
         iconsAdd++;
         addName = i->name;
      }
      // else we found the icon in question (whether moved or not), no change
   }

   // Now check the other direction for deleted icons
   for (IconIter i = previous_history.m_icons.begin(); i != previous_history.m_icons.end(); ++i)
   {
      bool found = false;
      for (IconIter j = m_icons.begin(); j != m_icons.end(); ++j)
      {
         if (i->name == j->name)
         {
            found = true;

            // If we've already found it, we can skip the rest of the list
            break;
         }
         // else two wholly unrelated icons
      }

      if (!found)
      {
         iconsDel++;
         delName = i->name;
      }
      // else we found the icon in question, no change
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
   if (iconsMov == 1) m_name = WSTRING("'" << movName << L"' Moved" << extra_with_parens);

   if (iconsMov == 0) m_name = extra;
   if (iconsMov == 0 && iconsAdd == 1 && iconsDel == 0) m_name = WSTRING(L"'" << addName << L"' Added");
   if (iconsMov == 0 && iconsAdd == 0 && iconsDel == 1) m_name = WSTRING(L"'" << delName << L"' Deleted");
}

bool IconHistory::Identical(const IconHistory &other) const
{
   // Match the icon set in both directions.  This will
   // cover additions, deletions, and moved icons.

   for (IconIter i = m_icons.begin(); i != m_icons.end(); ++i)
   {
      bool found = false;
      for (IconIter j = other.m_icons.begin(); j != other.m_icons.end(); ++j)
      {
         if (i->name == j->name && i->x == j->x && i->y == j->y)
         {
            found = true;
            break;
         }
      }
      if (!found) return false;
   }

   for (IconIter i = other.m_icons.begin(); i != other.m_icons.end(); ++i)
   {
      bool found = false;
      for (IconIter j = m_icons.begin(); j != m_icons.end(); ++j)
      {
         if (i->name == j->name && i->x == j->x && i->y == j->y)
         {
            found = true;
            break;
         }
      }

      if (!found) return false;
   }

   return true;
}

wstring IconHistory::Serialize() const
{
   wostringstream os;

   // NOTE: The multi-byte output requires this
   const static wstring end = L"\r\n";

   // Write the header
   os << L": =============================================" << end;
   os << L": IconHistory \"" << m_name << L"\"" << end << end;

   if (IsNamedProfile()) { os << named_identifier << end; }

   os << m_name << end;
   os << (unsigned int)m_icons.size() << end;
   os << end;

   // Write each icon
   int counter = 0;
   for (IconIter i = m_icons.begin(); i != m_icons.end(); ++i)
   {
      os << i->name << end;
      os << i->x << end;
      os << i->y << end;
      os << end;
   }

   os << end;

   return os.str();
}
