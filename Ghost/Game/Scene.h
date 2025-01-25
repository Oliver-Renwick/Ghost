#pragma once

#include <vector>

#include "Physics/Body.h"
#include "Physics/Shapes/Sphere_Shape.h"
#include "Physics/Shapes/Box_Shape.h"
#include "Physics/Intersection.h"

namespace game
{
	struct Scene
	{
		std::vector<phy::Body> m_bodies;

		void Initialize();
		void Reset();
		void Clear();
		void Update(const float dt_sec);
	};
}