#include "Intersection.h"

namespace phy
{
	bool sphereBoxIntersection(Body* bodyA, Body* bodyB, contact_t& contact)
	{
		contact.bodyA = bodyA;
		contact.bodyB = bodyB;

		const Vec3 ab = bodyB->m_Pos - Vec3(0.0f, bodyA->m_Pos.y, 0.0f);

		contact.normal = ab;
		contact.normal.Normalize();

		const Shape_Sphere* sphereA = (const Shape_Sphere*)bodyA->m_shape;
		const Shape_Box* BoxB = (const Shape_Box*)bodyB->m_shape;


		contact.ptOnA_WorldSpace = bodyA->m_Pos + contact.normal * sphereA->m_radius;
		contact.ptOnB_WorldSpace = bodyB->m_Pos - contact.normal * (BoxB->dimensions.y); // Start with box center

	



		const float lengthSquaredx = ab.x * ab.x;
		const float lengthSquaredy = ab.y * ab.y;
		const float lengthSquaredz = ab.z * ab.z;

		const float x = BoxB->dimensions.x / 2.0f;
		const float y = BoxB->dimensions.y / 2.0f;
		const float z = BoxB->dimensions.z / 2.0f;

		const float length_x = sphereA->m_radius + x;
		const float length_y = sphereA->m_radius + y;
		const float length_z = sphereA->m_radius + z;

		if ((lengthSquaredx < (length_x * length_x)) && 
			(lengthSquaredy < (length_y * length_y)) &&
			(lengthSquaredz < (length_z * length_z))) { return true; }

		return false;
	}
}