// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __FILE_READER_H
#define __FILE_READER_H

#include <string>
#include <fstream>

// A simple file manipulation class to read the plain-text with pound ('#')
// comment line format.  (Whitespace allowed, with one data item per line)
class FileReader
{
public:
   FileReader(const std::string &filename);
   ~FileReader();

   // Will skip whitespace and comment lines
   // On eof, will continuously return empty strings
   const std::string ReadLine();

private:
   // Explicitly deny copying and assignment
   FileReader(const FileReader&);
   FileReader& operator=(FileReader&);

   const static char comment_char = '#';

   std::ifstream file;
};

#endif
