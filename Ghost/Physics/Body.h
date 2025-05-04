#pragma once

#include "Math/Vector.h"
#include "Math/Mat.h"
#include "Math/Quat.h"

#include "Shapes/Shapes.h"

namespace phy
{
	struct Body
	{

		Vec3	 m_Pos = Vec3(0.0f,0.0f,0.0f);
		Quat     m_orientation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		Vec3	 m_color;
		uint32_t m_materialID = 0;

		Shape*  m_shape = nullptr;

	

		void Delete();
	};
}