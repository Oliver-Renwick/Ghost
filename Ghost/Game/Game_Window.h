#pragma once
#include <windows.h>
#include <exception>
#include <iostream>

namespace game
{
	struct Win
	{
		HWND m_window = nullptr;
		HINSTANCE m_hInst = nullptr;
		WNDPROC m_wndproc = nullptr;

		void Setup_Window();
	};
}
