#include "Body.h"

namespace phy
{
	Vec3 Body::GetCenterofMassBodySpace() const
	{
		Vec3 centerofMass = m_shape->GetCenterofMass();
		return centerofMass;
	}

	Vec3 Body::GetCenterofMassWorldSpace() const
	{
		Vec3 centerofMass = m_shape->GetCenterofMass();
		Vec3 pos = m_Pos + m_orientation.RotatePoint(centerofMass);
		return pos;
	}

	Vec3 Body::WorldSpaceToBodySpace(const Vec3& pt) const
	{
		Vec3 tmp = pt - GetCenterofMassWorldSpace();
		Quat InverseQuat = m_orientation.Inverse();
		Vec3 bodySpace = InverseQuat.RotatePoint(tmp);

		return bodySpace;
	}

	Vec3 Body::BodySpaceToWorldSpace(const Vec3& pt) const
	{
		Vec3 WorldSpace = GetCenterofMassWorldSpace() + m_orientation.RotatePoint(pt);
		return WorldSpace;
	}

	void Body::ApplyImpulseLinear(const Vec3& impulse)
	{
		if (m_invMass == 0.0)
			return;

		m_LinearVelocity += impulse * m_invMass;
	}

	void Body::Delete()
	{
		if (m_shape)
		{
			delete m_shape;
		}
	}
}