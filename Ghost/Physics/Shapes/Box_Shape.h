#pragma once
#include "Shapes.h"

namespace phy
{
	struct Shape_Box : public Shape
	{
		Shape_Box(float x, float y, float z)
		{
			dimensions = Vec3(x, y, z);
			m_CenterOfMass.Zero();
		}

		Vec3 dimensions;
		shapeType GetType() const override { return Box; }
	};
}