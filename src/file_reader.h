// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <sstream>

// A simple file manipulation class to read the plain-text with colon (':')
// comment line format.  (Whitespace allowed, with one data item per line)
class FileReader
{
public:
   FileReader(const std::wstring &filename);

   // Will skip whitespace and comment lines
   // On eof, will continuously return empty strings
   const std::wstring ReadLine();

private:
   // Explicitly deny copying and assignment
   FileReader(const FileReader&);

   const static wchar_t comment_char = L':';

   std::unique_ptr<std::wistringstream> stream;
};
