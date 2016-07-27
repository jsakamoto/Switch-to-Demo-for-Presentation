#include "stdafx.h"
#include "Utility.h"

void SetWindowPos(HWND hwnd, const RECT & rect)
{
	if (rect.top == 0 && rect.bottom == 0) return;
	SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
}

void ShowWindow(SLIDESHOWWNDS slideshowWnds)
{
	ShowWindow(slideshowWnds.hSlideShowWnd, SW_SHOW);
	if (IsWindow(slideshowWnds.hPresenterViewWnd)) ShowWindow(slideshowWnds.hPresenterViewWnd, SW_SHOW);
}

void HideWindow(SLIDESHOWWNDS slideshowWnds)
{
	ShowWindow(slideshowWnds.hSlideShowWnd, SW_HIDE);
	if (IsWindow(slideshowWnds.hPresenterViewWnd)) ShowWindow(slideshowWnds.hPresenterViewWnd, SW_HIDE);
}
