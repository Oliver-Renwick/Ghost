#pragma once

#include <vector>

#include "Physics/Body.h"
#include "Physics/Shapes/Sphere_Shape.h"
#include "Physics/Shapes/Box_Shape.h"
#include "Physics/Shapes/GltfShape.h"
#include "Player.h"

namespace game
{
	struct Scene
	{
		std::vector<phy::Body*> m_bodies;
		std::vector<phy::Body*> m_gltfBodies;
		Player* player = nullptr;
		Player* enemy = nullptr;
		phy::Body floor;
		phy::Body gltfTest;

		void Initialize();
		void Reset();
		void Clear();
		void Update(const float dt_sec);
	};
}