// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "file_reader.h"
#include "icon_history.h"

// For STANDARD_ERROR
#include <windows.h>
#include "saver.h"

#include <sstream>
using namespace std;

#define STRING(v) ((static_cast<std::ostringstream&>(std::ostringstream().flush() << v)).str())

const string IconHistory::named_identifier("named_profile");


IconHistory::IconHistory(bool named_profile)
{
   m_named_profile = named_profile;
   m_name = "Initial History";
}

bool IconHistory::Deserialize(FileReader *fr)
{
   // Reset our icon list;
   m_icons = IconList();
   m_named_profile = false;

   // Read the header
   string new_name = fr->ReadLine();
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
   istringstream icon_count_stream(fr->ReadLine());
   icon_count_stream >> icon_count;

   // Don't check for (icon_count > 0), because
   // that's actually perfectly acceptable.

   // Parse each individual icon
   for (int i = 0; i < icon_count; ++i)
   {
      Icon icon;

      icon.name = fr->ReadLine();
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
         STANDARD_ERROR("There was a problem reading from the history file.  This"
            " should fix itself automatically, but some profiles may have"
            " been lost.");

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

   string addName;
   string delName;
   string movName;

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
   const static string::size_type MaxNameLength = 30;
   const static string ellipsis = "...";
   if (addName.length() > MaxNameLength) addName = addName.substr(0, MaxNameLength) + ellipsis;
   if (delName.length() > MaxNameLength) delName = delName.substr(0, MaxNameLength) + ellipsis;
   if (movName.length() > MaxNameLength) movName = movName.substr(0, MaxNameLength) + ellipsis;

   // Default to more generic messages, but let
   // specific one-icon messages pre-empt
   string extra = "";
   string extra_with_parens = "";
   if (iconsAdd > 0) extra = STRING(iconsAdd << " Added");
   if (iconsDel > 0) extra = STRING(iconsDel << " Deleted");
   if (iconsAdd > 0 && iconsDel > 0) extra = STRING(iconsAdd << " Added, " << iconsDel << " Deleted");
   if (extra.length() > 0) extra_with_parens = " (" + extra + ")";

   if (iconsMov > 0) m_name = STRING(iconsMov << " Moved" << extra_with_parens);
   if (iconsMov == 1) m_name = STRING("'" << movName << "' Moved" << extra_with_parens);

   if (iconsMov == 0) m_name = extra;
   if (iconsMov == 0 && iconsAdd == 1 && iconsDel == 0) m_name = STRING("'" << addName << "' Added");
   if (iconsMov == 0 && iconsAdd == 0 && iconsDel == 1) m_name = STRING("'" << delName << "' Deleted");
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

string IconHistory::Serialize() const
{
   ostringstream os;

   // Write the header
   os << "# =============================================" << endl;
   os << "# IconHistory \"" << m_name << "\"" << endl << endl;

   if (IsNamedProfile()) { os << named_identifier << endl; }

   os << m_name << endl;
   os << (unsigned int)m_icons.size() << endl;
   os << endl;

   // Write each icon
   int counter = 0;
   for (IconIter i = m_icons.begin(); i != m_icons.end(); ++i)
   {
      //os << "# Icon " << ++counter << " of " << (unsigned int)(m_icons.size() + 1) << endl;

      os << i->name << endl;
      os << i->x << endl;
      os << i->y << endl;
      os << endl;
   }

   os << endl;

   return os.str();
}

