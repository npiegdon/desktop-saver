// DesktopSaver, (c)2006-2017 Nicholas Piegdon, MIT licensed

#include "create_dialog.h"
#include "resource.h"
#include <memory>

static std::wstring result;

INT_PTR CALLBACK CreateDialogProc(HWND dialog_hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_INITDIALOG: return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
        {
            const HWND edit = GetDlgItem(dialog_hwnd, IDC_NAME_BOX);
            const int length = (int)SendMessage(edit, EM_LINELENGTH, 0, 0);

            if (length == 0) { EndDialog(dialog_hwnd, 0); return TRUE; }
            auto bytes = std::make_unique<wchar_t[]>(length);

            // EM_GETLINE looks for the length of the string in the first word of the buffer
            *reinterpret_cast<WORD*>(bytes.get()) = length;
            SendMessage(edit, EM_GETLINE, 0, (LPARAM)bytes.get());

            result = std::wstring(bytes.get(), length);
            EndDialog(dialog_hwnd, 0);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(dialog_hwnd, 0);
            return TRUE;
        }
    }
    return FALSE;
}

std::wstring AskForNewProfileName(HINSTANCE hinst, HWND hwnd)
{
    result.clear();
    DialogBox(hinst, MAKEINTRESOURCE(IDD_NEW_PROFILE), hwnd, CreateDialogProc);
    return result;
}
