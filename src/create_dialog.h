// DesktopSaver, (c)2006-2016 Nicholas Piegdon, MIT licensed
#pragma once

#include <string>
#include <windows.h>

const static INT_PTR ProfileNameDialogCancelled = 1;

std::wstring AskForNewProfileName(HINSTANCE hinst, HWND hwnd);
