#include"Game_Window.h"

namespace game
{
	void Win::Setup_Window()
	{
        WNDCLASS win{};

        win.style = CS_HREDRAW | CS_VREDRAW;
        win.hInstance = m_hInst;
        win.lpfnWndProc = m_wndproc;
        win.lpszClassName = L"VulkanRendererWindowClass";

        if (RegisterClass(&win))
        {
            m_window = CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_WINDOWEDGE, win.lpszClassName, L"Vulkan_Render", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, m_hInst, 0);
        }
        else
        {
            throw std::runtime_error(" Cannot create WNDCLASS ");
        }
	}
}