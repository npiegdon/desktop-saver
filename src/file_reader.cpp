// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed

#include "file_reader.h"

#include <fstream>
#include <set>

using namespace std;

FileReader::FileReader(const wstring &filename) : stream(0)
{
   // Try opening the specified file
   FILE *f = 0;
   errno_t err = _wfopen_s(&f, filename.c_str(), L"rb");
   if (err != 0 || f == 0) return;

   const static int BufferSize = 256;
   wchar_t buffer[BufferSize];
   memset(buffer, 0, BufferSize*sizeof(wchar_t));

   wstring whole_file;
   while (fread(buffer, sizeof(wchar_t), BufferSize - 1, f) > 0)
   {
      whole_file += wstring(buffer);
      memset(buffer, 0, BufferSize*sizeof(wchar_t));
   }
   
   stream = new wistringstream(whole_file.c_str());
   fclose(f);
}

FileReader::~FileReader()
{
   delete stream;
}

const wstring FileReader::ReadLine()
{
   if (!stream) return L"";

   bool keepGoing;
   wstring line;

   do
   {
      if (!stream->good()) return L"";

      getline(*stream, line);

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

