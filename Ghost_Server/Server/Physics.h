#pragma once

#include "Server_Packet.h"
#include "Math/Vector.h"


namespace phy
{
	struct gameobj
	{
		Vec3 m_position;
		Vec3 m_rotation;
		Vec3 m_scale;

		Vec3 m_velocity;
		Vec3 m_acceleration;
		float m_invMass = 0;

		Vec3 m_forceAccum;

		float m_damping = 0;

		//Changes game obj state
		void Integrator(Packet& packet, float duration);

		void clearForceAccumator();

		void encodePacket(Packet& packet);
		void decodePacket(Packet& packet);
		void addMovement(Packet& packet);
	};
}