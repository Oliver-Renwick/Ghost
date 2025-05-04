#include "B_Console.h"

namespace base
{
	void setUp_Console()
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		FILE* stream;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w+", stdout);
		freopen_s(&stream, "CONOUT$", "w+", stderr);
		// Enable flags so we can color the output
		HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(consoleHandle, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(consoleHandle, dwMode);
		SetConsoleTitle(TEXT("VULKAN CONSOLE"));
	}
}