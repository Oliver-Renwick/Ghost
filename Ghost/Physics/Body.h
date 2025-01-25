#pragma once

#include "Math/Vector.h"
#include "Math/Mat.h"
#include "Math/Quat.h"

#include "Shapes/Shapes.h"

namespace phy
{
	struct Body
	{

		Vec3	m_Pos = Vec3(0.0f,0.0f,0.0f);
		Quat    m_orientation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
		Vec3    m_LinearVelocity;
		Vec3    m_color;
		float   m_invMass;
		float   m_elasticity;

		Shape*  m_shape = nullptr;

		Vec3 GetCenterofMassWorldSpace() const;
		Vec3 GetCenterofMassBodySpace() const;

		Vec3 WorldSpaceToBodySpace(const Vec3& pt) const;
		Vec3 BodySpaceToWorldSpace(const Vec3& pt) const;

		void ApplyImpulseLinear(const Vec3& impulse);

		void Delete();
	};
}