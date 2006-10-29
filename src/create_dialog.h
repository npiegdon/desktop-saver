// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __CREATE_DIALOG_H
#define __CREATE_DIALOG_H

#include <string>
#include <windows.h>

const static INT_PTR ProfileNameDialogCancelled = 1;

std::wstring AskForNewProfileName(HINSTANCE hinst, HWND hwnd);

#endif
