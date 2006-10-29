// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "file_reader.h"
#include <fstream>

using namespace std;

FileReader::FileReader(const string &filename)
{
   // Try opening the specified file
   file.open(filename.c_str());

   if (!file.good()) file.close();
}

FileReader::~FileReader()
{
   file.close();
}

const string FileReader::ReadLine()
{
   bool keepGoing;
   string line;

   do
   {
      if (!file.good()) return "";

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

         for (int i = 0; i < (int)line.length(); ++i)
         {
            if (!isspace(line[i])) foundNonWhitespace = true;
         }

         // If after searching the whole line, we didn't find any
         // regular characters at all, this is a whitespace line
         if (!foundNonWhitespace) keepGoing = true;
      }

   } while (keepGoing);

   return line;
}
