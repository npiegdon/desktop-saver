// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "file_reader.h"

#include "fstream_util.h"
#include <fstream>

#include <set>

using namespace std;

FileReader::FileReader(const wstring &filename)
{

#pragma warning(push)
#pragma warning(disable : 4996)
   IMBUE_NULL_CODECVT(file);
#pragma warning(pop)

   // Try opening the specified file
   file.open(filename.c_str(), ios::in | ios::binary);

   if (!file.good()) file.close();
}

FileReader::~FileReader()
{
   file.close();
}

const wstring FileReader::ReadLine()
{
   bool keepGoing;
   wstring line;

   do
   {
      if (!file.good()) return L"";

      getline(file, line);

      keepGoing = false;

      // Strip comments out of the line
      for (size_t i = 0; i < line.length(); ++i)
      {
         if (line[i] == comment_char)
         {
            line = line.substr(0, i);
            break;
         }
      }

      // If the now-comment-stripped line is empty, just
      // keep grabbing input from the file, and ignore this line
      if (line.length() == 0) keepGoing = true;

      // If this doesn't appear to be empty or a comment line, a little
      // more rigor is required to determine if this is a whitespace line
      // (containing an accidental space or something)
      if (line.length() > 0)
      {
         bool foundNonWhitespace = false;

         std::set<wchar_t> whitespace;
         whitespace.insert(L' ');
         whitespace.insert(L'\n');
         whitespace.insert(L'\r');
         whitespace.insert(L'\t');
         
         for (int i = 0; i < (int)line.length(); ++i)
         {
            wchar_t character = line[i];
            if (whitespace.find(character) == whitespace.end()) foundNonWhitespace = true;
         }

         // If after searching the whole line, we didn't find any
         // regular characters at all, this is a whitespace line
         if (!foundNonWhitespace) keepGoing = true;
      }

   } while (keepGoing);

   while (line.length() > 1 && line[line.length() - 1] == 13)
   {
      line = line.substr(0, line.length() - 1);
   }

   return line;
}
