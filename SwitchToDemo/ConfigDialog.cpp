#include "stdafx.h"
#include "SwitchToDemo.h"
#include "Config.h"
#include "ConfigDialog.h"

struct VKEYWITHNAME
{
	WPARAM vKey;
	LPCTSTR pszName;
};
typedef const VKEYWITHNAME *LPCVKEYWITHNAME;

static void OnInitDialog(HWND hWndDlg);
static void OnOK(HWND hWndDlg);
static void GetVKeys(LPCVKEYWITHNAME *ppVKeys, int *pnCount);

INT_PTR CALLBACK ConfigDialogProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			OnInitDialog(hWndDlg);
			return TRUE;
		}

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			OnOK(hWndDlg);
			EndDialog(hWndDlg, IDOK);
			return TRUE;

		case IDCANCEL:
			EndDialog(hWndDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static void OnInitDialog(HWND hDlg)
{
	BYTE vkey = LOBYTE((WORD)g_Config.Hotkey);
	BYTE mod = HIBYTE((WORD)g_Config.Hotkey);
	SendDlgItemMessage(hDlg, IDC_MODWIN, BM_SETCHECK, mod & MOD_WIN ? 1 : 0, 0);
	SendDlgItemMessage(hDlg, IDC_MODSHIFT, BM_SETCHECK, mod & MOD_SHIFT ? 1 : 0, 0);
	SendDlgItemMessage(hDlg, IDC_MODCTRL, BM_SETCHECK, mod & MOD_CONTROL ? 1 : 0, 0);
	SendDlgItemMessage(hDlg, IDC_MODALT, BM_SETCHECK, mod & MOD_ALT ? 1 : 0, 0);
	SendDlgItemMessage(hDlg, IDC_CHECK1, BM_SETCHECK, g_Config.AutoChangeMultiMonitorMode ? 1 : 0, 0);

	LPCVKEYWITHNAME pVKeys = NULL;
	int nKeys = 0;
	GetVKeys(&pVKeys, &nKeys);
	HWND hDDL = GetDlgItem(hDlg, IDC_KEY);
	for (int i=0; i < nKeys; i++)
	{
		const VKEYWITHNAME& vk = pVKeys[i];
		LRESULT index = SendMessage(hDDL, CB_ADDSTRING, 0, (LPARAM)vk.pszName);
		if (vk.vKey == vkey)
			SendMessage(hDDL, CB_SETCURSEL, index, 0);
		SendMessage(hDDL, CB_SETITEMDATA, index, vk.vKey);
	}
}

static void OnOK(HWND hDlg)
{
	BYTE mod = 0;
	mod |= SendDlgItemMessage(hDlg, IDC_MODWIN, BM_GETCHECK, 0, 0) != 0 ? MOD_WIN : 0;
	mod |= SendDlgItemMessage(hDlg, IDC_MODSHIFT, BM_GETCHECK, 0, 0) != 0 ? MOD_SHIFT : 0;
	mod |= SendDlgItemMessage(hDlg, IDC_MODCTRL, BM_GETCHECK, 0, 0) != 0 ? MOD_CONTROL : 0;
	mod |= SendDlgItemMessage(hDlg, IDC_MODALT, BM_GETCHECK, 0, 0) != 0 ? MOD_ALT : 0;

	HWND hDDL = GetDlgItem(hDlg, IDC_KEY);
	WPARAM index = (WPARAM)SendMessage(hDDL, CB_GETCURSEL, 0, 0);
	BYTE vkey = (BYTE)SendMessage(hDDL, CB_GETITEMDATA, index, 0);

	g_Config.Hotkey = MAKEWORD(vkey, mod);

	TCHAR szKey[20];
	SendMessage(hDDL, CB_GETLBTEXT, index, (LPARAM)szKey);
	g_Config.szHotkeyDisplayName[0] = TEXT('\0');
	lstrcat(g_Config.szHotkeyDisplayName, mod & MOD_WIN ? TEXT("Win + ") : TEXT(""));
	lstrcat(g_Config.szHotkeyDisplayName, mod & MOD_SHIFT ? TEXT("Shift + ") : TEXT(""));
	lstrcat(g_Config.szHotkeyDisplayName, mod & MOD_CONTROL ? TEXT("Ctrl + ") : TEXT(""));
	lstrcat(g_Config.szHotkeyDisplayName, mod & MOD_ALT ? TEXT("Alt + ") : TEXT(""));
	lstrcat(g_Config.szHotkeyDisplayName, szKey);

	g_Config.AutoChangeMultiMonitorMode = SendDlgItemMessage(hDlg, IDC_CHECK1, BM_GETCHECK, 0, 0) != 0;
}

static void GetVKeys(LPCVKEYWITHNAME *ppVKeys, int *pnCount)
{
	static const VKEYWITHNAME vkeys[] =
	{
		{'0', TEXT("0")},
		{'1', TEXT("1")},
		{'2', TEXT("2")},
		{'3', TEXT("3")},
		{'4', TEXT("4")},
		{'5', TEXT("5")},
		{'6', TEXT("6")},
		{'7', TEXT("7")},
		{'8', TEXT("8")},
		{'9', TEXT("9")},

		{'A', TEXT("A")},
		{'B', TEXT("B")},
		{'C', TEXT("C")},
		{'D', TEXT("D")},
		{'E', TEXT("E")},
		{'F', TEXT("F")},
		{'G', TEXT("G")},
		{'H', TEXT("H")},
		{'I', TEXT("I")},
		{'J', TEXT("J")},
		{'K', TEXT("K")},
		{'L', TEXT("L")},
		{'M', TEXT("M")},
		{'N', TEXT("N")},
		{'O', TEXT("O")},
		{'P', TEXT("P")},
		{'Q', TEXT("Q")},
		{'R', TEXT("R")},
		{'S', TEXT("S")},
		{'T', TEXT("T")},
		{'U', TEXT("U")},
		{'V', TEXT("V")},
		{'W', TEXT("W")},
		{'X', TEXT("X")},
		{'Y', TEXT("Y")},
		{'Z', TEXT("Z")},

		{VK_F1, TEXT("F1")},
		{VK_F2, TEXT("F2")},
		{VK_F3, TEXT("F3")},
		{VK_F4, TEXT("F4")},
		{VK_F5, TEXT("F5")},
		{VK_F6, TEXT("F6")},
		{VK_F7, TEXT("F7")},
		{VK_F8, TEXT("F8")},
		{VK_F9, TEXT("F9")},
		{VK_F10, TEXT("F10")},
		{VK_F11, TEXT("F11")},
		{VK_F12, TEXT("F12")},
		{VK_F13, TEXT("F13")},
		{VK_F14, TEXT("F14")},
		{VK_F15, TEXT("F15")},
		{VK_F16, TEXT("F16")},

		{VK_SPACE, TEXT("Space")},
		{VK_TAB, TEXT("Tab")},
		{VK_ESCAPE, TEXT("Esc")},
		{VK_RETURN, TEXT("Enter")},
		{VK_BACK, TEXT("Backspace")},
		{VK_DELETE, TEXT("Delete")},
		{VK_INSERT, TEXT("Insert")},
		
		{VK_LEFT, TEXT("Left")},
		{VK_RIGHT, TEXT("Right")},
		{VK_UP, TEXT("Up")},
		{VK_DOWN, TEXT("Down")},
		{VK_PRIOR, TEXT("Page Up")},
		{VK_NEXT, TEXT("Page Down")},
		{VK_HOME, TEXT("Home")},
		{VK_END, TEXT("END")},
		
		{VK_PRINT, TEXT("PrintScreen")},
		{VK_SCROLL, TEXT("ScrollLock")},
		{VK_NUMLOCK, TEXT("NumLock")},
		{VK_CAPITAL, TEXT("CapsLock")},
	};
	*ppVKeys = vkeys;
	*pnCount = sizeof(vkeys)/sizeof(vkeys[0]);
}
