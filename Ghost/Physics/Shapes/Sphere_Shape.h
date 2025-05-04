#pragma once
#include "Shapes.h"


namespace phy
{
	struct Shape_Sphere : public Shape
	{
		Shape_Sphere(float radius) { 
			m_radius = radius; 
			m_CenterOfMass.Zero();
		}

		float m_radius = 0.0f;

		shapeType GetType() const override { return Sphere; }
	};
}