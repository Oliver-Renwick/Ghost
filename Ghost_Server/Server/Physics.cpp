#include "Physics.h"


namespace phy
{
	void gameobj::encodePacket(Packet& packet)
	{
		packet.acceleration[0] = m_acceleration.x;
		packet.acceleration[1] = m_acceleration.y;
		packet.acceleration[2] = m_acceleration.z;

		packet.velocity[0] = m_velocity.x;
		packet.velocity[1] = m_velocity.y;
		packet.velocity[2] = m_velocity.z;

		packet.position[0] = m_position.x;
		packet.position[1] = m_position.y;
		packet.position[2] = m_position.z;

		packet.invMass = m_invMass;

		packet.damping = m_damping;
	}

	void gameobj::decodePacket(Packet& packet)
	{
		m_position.x = packet.position[0];
		m_position.y = packet.position[1];
		m_position.z = packet.position[2];

		m_acceleration.x = packet.acceleration[0];
		m_acceleration.y = packet.acceleration[1];
		m_acceleration.z = packet.acceleration[2];

		m_velocity.x = packet.velocity[0];
		m_velocity.y = packet.velocity[1];
		m_velocity.z = packet.velocity[2];

		m_damping = packet.damping;
		m_invMass = packet.invMass;
	}

	void gameobj::addMovement(Packet& packet)
	{
		if (packet.left || packet.right || packet.backward || packet.forward)
		{
			if (packet.left) { m_velocity.x = -4.0f; std::cout << "Velocity has be Added by =======>>>>>" << m_velocity.x << std::endl; }
			if (packet.right) { m_velocity.x = 4.0f; std::cout << "Velocity has be Added by =======>>>>>" << m_velocity.x << std::endl;}
			if (packet.forward) { m_velocity.z = -4.0f; std::cout << "Velocity has be Added by =======>>>>>" << m_velocity.z << std::endl;}
			if (packet.backward) { m_velocity.z = 4.0f; std::cout << "Velocity has be Added by =======>>>>>" << m_velocity.z << std::endl;}
		}
	}

	void gameobj::Integrator(Packet& packet, float duration)
	{
		decodePacket(packet);

		//Physics integrator
		if (m_invMass <= 0.0f) return;

		assert(duration > 0.0f);


		addMovement(packet);

		//Getting Acceleration from force
		Vec3 resultAcceleration = m_acceleration;
		resultAcceleration.AddScaledVector(m_forceAccum, m_invMass);

		//Udate Linear velocity from acceleration
		m_velocity.AddScaledVector(resultAcceleration, duration);

		m_velocity *= pow(m_damping, duration);
		m_position.AddScaledVector(m_velocity, duration);

		//Clear Force
		clearForceAccumator();

		encodePacket(packet);
	}

	void gameobj::clearForceAccumator()
	{
		m_forceAccum.Zero();
		m_acceleration.Zero();
		m_velocity.Zero();
	}
}