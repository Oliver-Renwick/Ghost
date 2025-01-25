#pragma once
#include "Body.h"

namespace phy
{
	struct contact_t {

		Vec3 ptOnA_WorldSpace;
		Vec3 ptOnB_WorldSpace;
		Vec3 ptOnA_LocalSpace;
		Vec3 ptOnB_LocalSpace;

		Vec3 normal;
		float seperationTime;
		float timeOfImpact;

		Body* bodyA;
		Body* bodyB;

	};

	void ResolveContact(contact_t& contact);
}