#pragma once
#include "Shapes.h"

namespace phy
{
	struct Shape_Gltf : public Shape
	{
		Shape_Gltf(std::string filename)
		{
			m_CenterOfMass.Zero();
			gltfPath = filename;
		}

		Vec3 dimensions;
		shapeType GetType() const override { return GLTF; }
		std::string gltfPath;
	};
}