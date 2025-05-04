#pragma once
#include "glm/glm.hpp"
#include <iostream>
#include <Windows.h>


#if defined(_WIN32)

#define Key_T 0x54
#define Key_R 0x52
#define Key_W 0x57
#define Key_A 0x41
#define Key_S 0x53
#define Key_D 0x44
#define Key_1 0x31
#define Key_2 0x32
#define Key_LEFT 0x25
#define Key_UP 0x26
#define Key_RIGHT 0x27
#define Key_DOWN 0x28
#define Key_E 0x45
#define Key_Q 0x51

#endif

namespace game
{
	struct Input
	{
		bool forward  = false;
		bool backward = false;
		bool left     = false;
		bool right    = false;
	};

	extern Input input;

	// Monitor Moving State
	inline bool moving()
	{
		return input.forward || input.backward || input.left || input.right;
	}


}