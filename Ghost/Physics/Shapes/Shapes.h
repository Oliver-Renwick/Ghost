#pragma once
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Mat.h"

#include <string>
#include <vector>

namespace phy
{

	struct Shape
	{
		Vec3 m_CenterOfMass;
		enum shapeType
		{
			Sphere,
			Box,
			GLTF,
			Convex
		};

		virtual shapeType GetType() const = 0;
		virtual Vec3 GetCenterofMass()  { return m_CenterOfMass; }
	};
}