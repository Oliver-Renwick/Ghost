#include "Packet.h"

namespace net
{
	Packet scenePacket(const game::Scene& scene)
	{
		Packet packet{};

		if (scene.player)
		{
			packet.ID_PROTO = 1234;

			packet.position[0] = scene.player->m_position.x;
			packet.position[1] = scene.player->m_position.y;
			packet.position[2] = scene.player->m_position.z;

			packet.velocity[0] = scene.player->m_velocity.x;
			packet.velocity[1] = scene.player->m_velocity.y;
			packet.velocity[2] = scene.player->m_velocity.z;

			packet.acceleration[0] = scene.player->m_acceleration.x;
			packet.acceleration[1] = scene.player->m_acceleration.y;
			packet.acceleration[2] = scene.player->m_acceleration.z;

			packet.invMass = scene.player->m_invMass;

			packet.damping = scene.player->m_damping;

			packet.forward = scene.player->m_forward;
			packet.backward = scene.player->m_backward;
			packet.left = scene.player->m_left;
			packet.right = scene.player->m_right;


		}

		return packet;
	}

	void Serialize(const Packet& packet, char* buffer)
	{
		memcpy(buffer, &packet, sizeof(Packet));
	}

	void Deserialize(const char* buffer, Packet& packet)
	{

		memcpy(&packet, buffer, sizeof(Packet));
	}
}