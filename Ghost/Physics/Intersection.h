#pragma once
#include "Shapes/Box_Shape.h"
#include "Shapes/Sphere_Shape.h"
#include "Body.h"
#include "Contact.h"


namespace phy
{
	bool sphereBoxIntersection(Body* bodyA, Body* bodyB, contact_t& contact);
}