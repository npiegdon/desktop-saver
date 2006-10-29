// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "create_dialog.h"
#include "resource.h"

// For INTERNAL_ERROR
#include "saver.h"

using namespace std;

BOOL CALLBACK CreateDialogProc(HWND dialog_hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      {
         return TRUE;
      }

   case WM_COMMAND:
      {
         switch (LOWORD(wparam))
         {
         case IDOK:
            {
               HWND edit = GetDlgItem(dialog_hwnd, IDC_NAME_BOX);

               // Grab text from control, allocate memory, and send back
               int length = (int)SendMessage(edit, EM_LINELENGTH, 0, 0);

               char *new_name = new char[length + 1];

               // The EM_GETLINE message requires that you write the length
               // of the string in the first word of the buffer
               *((int*)(new_name)) = length;

               SendMessage(edit, EM_GETLINE, 0, (LPARAM)new_name);

               // It also doesn't null terminate it
               new_name[length] = 0;

               EndDialog(dialog_hwnd, (INT_PTR)new_name);
               return TRUE;
            }

         case IDCANCEL:
            {
               EndDialog(dialog_hwnd, (INT_PTR)ProfileNameDialogCancelled);
               return TRUE;
            }
         }
      }
   }
   return FALSE;
}

wstring AskForNewProfileName(HINSTANCE hinst, HWND hwnd)
{
   INT_PTR ret = DialogBox(hinst, MAKEINTRESOURCE(IDD_NEW_PROFILE), hwnd, CreateDialogProc);

   if (ret <= 0)
   {
      INTERNAL_ERROR(L"Problem creating 'New Profile Name' dialog box.");
      exit(1);
   }

   if (ret == ProfileNameDialogCancelled) { return L""; }

   // Anything else returned ought to be a pointer to the new name
   wstring name = WSTRING((char*)ret);
   delete[] (char*)ret;

   return name;
}
