#include "stdafx.h"
#include <math.h>
#include "SwitchToDemo.h"
#include "ConfigDialog.h"
#include "Config.h"
#include "SlideShowWnds.h"
#include "Utility.h"

#define MAX_LOADSTRING 100
#define WM_TRAYICONMESSAGE	(WM_USER + 1)
#define WM_PINGPREVINSTANCE	(WM_USER + 2)

// Global variables.
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
UINT WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
HWND hMainWnd;
TCHAR szConfigPath[MAX_PATH];
LPCTSTR pszConfigSection = TEXT("SwitchToDemo");
RECT rcLastSlideShowWndPos;
RECT rcLastPresenterViewWndPos;

// Function prototypes.
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void SetupTaskTrayIcon(HWND hWnd);
void OnHotKey(HWND);
void OnTaskTrayMessage(HWND hWnd, LPARAM lParam);
HWND FindApplicationWindow();
SLIDESHOWWNDS FindSlideShowWindow();
BOOL CALLBACK FindPowerPointSlideShowWindiwProc(HWND hwnd, LPARAM lParam);
void FadeMainWnd(BOOL bFadeIn);
void Configuration(HWND hwnd);
void RestoreVisibility(HWND hWnd);
#define MULTIMONITORMODE_CLONE 0
#define MULTIMONITORMODE_EXTEND 1
void SwitchMultiMonitorMode(int mode);

#define ACTION_TO_SHOW 1
#define ACTION_TO_HIDE 2


int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SWITCHTODEMO, szWindowClass, MAX_LOADSTRING);
	ZeroMemory(&rcLastSlideShowWndPos, sizeof(rcLastSlideShowWndPos));
	ZeroMemory(&rcLastPresenterViewWndPos, sizeof(rcLastPresenterViewWndPos));

	HWND hPrevInstanceHwnd = FindWindow(szWindowClass, NULL);
	if (IsWindow(hPrevInstanceHwnd))
	{
		PostMessage(hPrevInstanceHwnd, WM_PINGPREVINSTANCE, 0, 0);
		return 200;
	}
	MyRegisterClass(hInstance);

	// Initialize applicationinstance.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SWITCHTODEMO));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_SWITCHTODEMO);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SWITCHTODEMO));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	//InitCommonControls();

	GetModuleFileName(NULL, szConfigPath, MAX_PATH);
	lstrcat(szConfigPath, TEXT(".ini"));

	// Load configuration.
	g_Config.Hotkey = GetPrivateProfileInt(pszConfigSection, TEXT("Hotkey"), MAKEWORD('H', MOD_WIN | MOD_SHIFT), szConfigPath);
	GetPrivateProfileString(pszConfigSection, TEXT("HotkeyDisplayName"), TEXT("Win + Shift + H"), g_Config.szHotkeyDisplayName, sizeof(g_Config.szHotkeyDisplayName) / sizeof(TCHAR), szConfigPath);

	// Create main window.
	hInst = hInstance;
	hMainWnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		szWindowClass, szTitle, WS_POPUP,
		0, 0, 640, 480, NULL, NULL, hInstance, NULL);
	if (!hMainWnd) return FALSE;

	// Register Hot-key "Windows + Shift + H".
	BYTE mod = HIBYTE(g_Config.Hotkey);
	BYTE vkey = LOBYTE(g_Config.Hotkey);
	RegisterHotKey(hMainWnd, 0, mod, vkey);

	// Setup task tray icon.
	SetupTaskTrayIcon(hMainWnd);

	ShowWindow(hMainWnd, SW_HIDE);

	return TRUE;
}

void SetupTaskTrayIcon(HWND hWnd)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;

	Shell_NotifyIcon(NIM_DELETE, &nid);

	nid.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SWITCHTODEMO), IMAGE_ICON, 16, 16, LR_SHARED);
	nid.uCallbackMessage = WM_TRAYICONMESSAGE;
	lstrcpy(nid.szInfoTitle, szTitle);
	wsprintf(nid.szTip, TEXT("%s (Hotkey is %s)"), szTitle, g_Config.szHotkeyDisplayName);
	wsprintf(nid.szInfo, TEXT("Enter hot-key \"%s\" to Show or Hide Slide-show."), g_Config.szHotkeyDisplayName);
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_INFO;
	Shell_NotifyIcon(NIM_ADD, &nid);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_CONFIGURATION:
			Configuration(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_TRAYICONMESSAGE:
		OnTaskTrayMessage(hWnd, lParam);
		break;
	case WM_PINGPREVINSTANCE:
		SetupTaskTrayIcon(hWnd);
		break;
	case WM_DESTROY:
	{
		SLIDESHOWWNDS slideHwnds = FindSlideShowWindow();
		RestoreVisibility(slideHwnds.hSlideShowWnd);
		RestoreVisibility(slideHwnds.hPresenterViewWnd);

		NOTIFYICONDATA nid;
		ZeroMemory(&nid, sizeof(nid));
		nid.cbSize = sizeof(nid);
		nid.hWnd = hWnd;
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	PostQuitMessage(0);
	break;

	case WM_HOTKEY:
		OnHotKey(hWnd);
		break;

	default:
		if (message == WM_TASKBARCREATED)
		{
			SetupTaskTrayIcon(hWnd);
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void RestoreVisibility(HWND hWnd) {
	if (IsWindow(hWnd) && IsWindowVisible(hWnd) == FALSE)
	{
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
	}
}

void OnTaskTrayMessage(HWND hWnd, LPARAM lParam)
{
	if (lParam == WM_RBUTTONDOWN)
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		SetForegroundWindow(hWnd);
		HMENU hMenuRoot = LoadMenu(hInst, MAKEINTRESOURCE(IDC_SWITCHTODEMO));
		HMENU hMenuPopup = GetSubMenu(hMenuRoot, 0);
		TrackPopupMenu(hMenuPopup, TPM_LEFTALIGN, cursorPos.x, cursorPos.y, 0, hWnd, NULL);
		DestroyMenu(hMenuRoot);
	}
}

SLIDESHOWWNDS FindSlideShowWindow()
{
	// Microsoft PowerPoint (2007 Viewer, 2010-2013) 
	SLIDESHOWWNDS slideHwnds;
	ZeroMemory(&slideHwnds, sizeof(slideHwnds));
	EnumWindows(FindPowerPointSlideShowWindiwProc, (LPARAM)&slideHwnds);

	if (IsWindow(slideHwnds.hSlideShowWnd) == FALSE) {
		// Kingsoft Presentation (2010) 
		slideHwnds.hSlideShowWnd = FindWindow(TEXT("TSlideShowDlg.UnicodeClass"), NULL);
		slideHwnds.hPresenterViewWnd = NULL;
	}
	return slideHwnds;
}

BOOL CALLBACK FindPowerPointSlideShowWindiwProc(HWND hwnd, LPARAM lParam)
{
	SLIDESHOWWNDS* pSlideShowWnds = (SLIDESHOWWNDS*)lParam;

	// Find by Window Class Name...
	TCHAR szClassName[50];
	GetClassName(hwnd, szClassName, sizeof(szClassName) / sizeof(TCHAR));
	if (lstrcmp(szClassName, TEXT("screenClass")) != 0) return TRUE;

	// and detect Presenter View.
	TCHAR szCaption[400];
	GetWindowText(hwnd, szCaption, sizeof(szCaption) / sizeof(TCHAR));
	LPCTSTR pNGWord = TEXT("Presenter View");
	int begin = lstrlen(szCaption) - lstrlen(pNGWord);
	begin = max(0, begin);
	if (lstrcmp(&szCaption[begin], pNGWord) == 0) {
		pSlideShowWnds->hPresenterViewWnd = hwnd;
	}
	else {
		pSlideShowWnds->hSlideShowWnd = hwnd;
	}

	return IsWindow(pSlideShowWnds->hSlideShowWnd) && IsWindow(pSlideShowWnds->hPresenterViewWnd) ? FALSE : TRUE;
}

HWND FindApplicationWindow()
{
	// Microsoft PowerPoint (2010-2013) 
	HWND hAppWnd = FindWindow(TEXT("PPTFrameClass"), NULL);
	if (IsWindow(hAppWnd) == FALSE)
		// Kingsoft Presentation (2010) 
		hAppWnd = FindWindow(TEXT("TfmMain.UnicodeClass"), NULL);
	return hAppWnd;
}

void OnHotKey(HWND hWnd)
{
	// Find slideshow window, and if not found then nothing to do.
	SLIDESHOWWNDS slideWnds = FindSlideShowWindow();
	HWND hSlideShowWnd = slideWnds.hSlideShowWnd;
	HWND hPresenterViewWnd = slideWnds.hPresenterViewWnd;
	if (!IsWindow(hSlideShowWnd)) return;

	// Minimize application window.
	HWND hAppWnd = FindApplicationWindow();
	if (IsWindow(hAppWnd) && IsWindow(hSlideShowWnd))
	{
		ShowWindow(hAppWnd, SW_SHOWMINNOACTIVE);
	}

	// Hide or Show slide-show window.
	BOOL visible = IsWindowVisible(hSlideShowWnd);
	int action = visible ? ACTION_TO_HIDE : ACTION_TO_SHOW;
	BOOL enablePresenterView = IsWindow(hPresenterViewWnd);

	if (action == ACTION_TO_HIDE) {
		GetWindowRect(hSlideShowWnd, &rcLastSlideShowWndPos);
		if (enablePresenterView) GetWindowRect(hPresenterViewWnd, &rcLastPresenterViewWndPos);

		SetWindowPos(hMainWnd, rcLastSlideShowWndPos);
		FadeMainWnd(TRUE);

		HideWindow(slideWnds);

		SetForegroundWindow(GetDesktopWindow());

		FadeMainWnd(FALSE);

		if (enablePresenterView) SwitchMultiMonitorMode(MULTIMONITORMODE_CLONE);
	}
	else // action == ACTION_TO_SHOW
	{
		if (enablePresenterView) SwitchMultiMonitorMode(MULTIMONITORMODE_EXTEND);

		SetWindowPos(hMainWnd, rcLastSlideShowWndPos);
		FadeMainWnd(TRUE);

		SetWindowPos(hSlideShowWnd, rcLastSlideShowWndPos);
		SetWindowPos(hPresenterViewWnd, rcLastPresenterViewWndPos);

		ShowWindow(slideWnds);

		FadeMainWnd(FALSE);

		SetForegroundWindow(hSlideShowWnd);
		if (enablePresenterView) SetForegroundWindow(hPresenterViewWnd);
	}
}

void FadeMainWnd(BOOL bFadeIn)
{
	int base = bFadeIn ? 0 : 255;
	if (bFadeIn)
	{
		SetLayeredWindowAttributes(hMainWnd, 0, 0, LWA_ALPHA);
		ShowWindow(hMainWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hMainWnd);
		BringWindowToTop(hMainWnd);
	}
	for (int i = 0; i < 256; i += 16)
	{
		int alpha = abs(base - i);
		SetLayeredWindowAttributes(hMainWnd, 0, alpha, LWA_ALPHA);
		UpdateWindow(hMainWnd);
		Sleep(30);
	}
	if (!bFadeIn)
	{
		ShowWindow(hMainWnd, SW_HIDE);
	}
}

void Configuration(HWND hwnd)
{
	Config prevConfig = g_Config;
	UnregisterHotKey(hwnd, 0);

	INT result = DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIG), GetDesktopWindow(), ConfigDialogProc);
	if (result == IDOK)
	{
		// Save configuration.
		TCHAR szHotkeyInt[100];
		wsprintf(szHotkeyInt, TEXT("%d"), g_Config.Hotkey);
		WritePrivateProfileString(pszConfigSection, TEXT("Hotkey"), szHotkeyInt, szConfigPath);
		WritePrivateProfileString(pszConfigSection, TEXT("HotkeyDisplayName"), g_Config.szHotkeyDisplayName, szConfigPath);
	}

	BYTE mod = HIBYTE(g_Config.Hotkey);
	BYTE vkey = LOBYTE(g_Config.Hotkey);
	RegisterHotKey(hwnd, 0, mod, vkey);

	if (prevConfig.Hotkey != g_Config.Hotkey)
		SetupTaskTrayIcon(hwnd);
}

void SwitchMultiMonitorMode(int mode)
{
	switch (mode) {
	case MULTIMONITORMODE_CLONE:
		SetDisplayConfig(0, NULL, 0, NULL, SDC_TOPOLOGY_CLONE | SDC_APPLY);
		break;
	case MULTIMONITORMODE_EXTEND:
		SetDisplayConfig(0, NULL, 0, NULL, SDC_TOPOLOGY_EXTEND | SDC_APPLY);
		break;
	}
}
