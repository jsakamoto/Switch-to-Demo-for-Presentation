#include "stdafx.h"

struct Config
{
	LPARAM Hotkey;
	TCHAR szHotkeyDisplayName[100];
	BOOL AutoChangeMultiMonitorMode;
};

extern Config g_Config;