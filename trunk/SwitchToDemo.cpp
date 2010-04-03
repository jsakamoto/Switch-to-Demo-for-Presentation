#include "stdafx.h"
#include "SwitchToDemo.h"

#define MAX_LOADSTRING 100
#define WM_TRAYICONMESSAGE	(WM_USER + 1)

// Global variables.
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
UINT WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));

// Function prototypes.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void SetupTaskTrayIcon(HWND hWnd);
void OnHotKey(HWND);
void OnTaskTrayMessage(HWND hWnd, LPARAM lParam);
HWND FindApplicationWindow();
HWND FindSlideShowWindow();

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	// Initialize global strings.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SWITCHTODEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Initialize applicationinstance.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SWITCHTODEMO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SWITCHTODEMO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SWITCHTODEMO));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Create main window.
   hInst = hInstance;
   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd) return FALSE;

   // Register Hot-key "Windows + H".
   RegisterHotKey(hWnd, 0, MOD_WIN, 'H');
   
   // Setup task tray icon.
   SetupTaskTrayIcon(hWnd);

   ShowWindow(hWnd, SW_HIDE);

   return TRUE;
}

void SetupTaskTrayIcon(HWND hWnd)
{
   NOTIFYICONDATA nid;
   ZeroMemory(&nid, sizeof(nid));
   nid.cbSize = sizeof(nid);
   nid.hWnd = hWnd;
   nid.hIcon = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SWITCHTODEMO),IMAGE_ICON,16,16,LR_SHARED);
   lstrcpy(nid.szTip, szTitle);
   nid.uCallbackMessage = WM_TRAYICONMESSAGE;
   lstrcpy(nid.szInfoTitle, szTitle);
   lstrcpy(nid.szInfo, TEXT("Enter hot-key \"Win + H\" to Show or Hide Slide-show."));
   nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE|NIF_INFO;
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
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
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

	case WM_DESTROY:
		{
			HWND hSlideShowWnd = FindSlideShowWindow();
			if (IsWindow(hSlideShowWnd) && IsWindowVisible(hSlideShowWnd) == FALSE)
			{
				ShowWindow(hSlideShowWnd, SW_SHOW);
				SetForegroundWindow(hSlideShowWnd);
			}
		
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

HWND FindSlideShowWindow()
{
	// Microsoft PowerPoint (2007 Viewer, 2010Beta) 
	HWND hSlideShowWnd = FindWindow(TEXT("screenClass"), NULL);
	if (IsWindow(hSlideShowWnd) == FALSE)
		// Kingsoft Presentation (2010) 
		hSlideShowWnd = FindWindow(TEXT("TSlideShowDlg.UnicodeClass"), NULL);
	return hSlideShowWnd;
}

HWND FindApplicationWindow()
{
	// Microsoft PowerPoint (2010Beta) 
	HWND hAppWnd = FindWindow(TEXT("PP12FrameClass"), NULL);
	if (IsWindow(hAppWnd) == FALSE)
		// Kingsoft Presentation (2010) 
		hAppWnd = FindWindow(TEXT("TfmMain.UnicodeClass"), NULL);
	return hAppWnd;
}

void OnHotKey(HWND hWnd)
{
	HWND hAppWnd = FindApplicationWindow();
	HWND hSlideShowWnd = FindSlideShowWindow();
	
	// Minimize application window.
	if (IsWindow(hAppWnd) && IsWindow(hSlideShowWnd))
	{
		ShowWindow(hAppWnd, SW_SHOWMINNOACTIVE);
	}
	
	// Hide or Show slide-show window.
	if (IsWindow(hSlideShowWnd))
	{
		BOOL isVisible = IsWindowVisible(hSlideShowWnd);
		ShowWindow(hSlideShowWnd, isVisible ? SW_HIDE : SW_SHOW);
		if (!isVisible) SetForegroundWindow(hSlideShowWnd);
	}
}

