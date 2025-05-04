#include "Application.h"
#include <iostream>
#include <Windows.h>
#include "Base/B_Console.h"
#include "Input.h"


game::Application* app = new game::Application();

LRESULT CALLBACK mainWindowCallBack(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (app)
	{
		app->Handle_Message(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);  

}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)

{
	base::setUp_Console();


	app->win.m_hInst = hInst;
	app->win.m_wndproc = mainWindowCallBack;

	app->Initialize();
	app->Run();

	delete app;
	std::cin.get();
}