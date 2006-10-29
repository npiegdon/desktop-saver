// DesktopSaver
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "saver_gui.h"

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE, PSTR, int)
{
   DesktopSaverGui tray(h_instance);
   return tray.Run();
}
