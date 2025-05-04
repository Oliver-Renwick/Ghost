#pragma once
#include "game/Scene.h"

namespace net
{
	struct Packet
	{
		uint32_t ID_PROTO = 0;
		uint32_t sequenceNumber = 0;

		float position[3];
		float velocity[3];
		float acceleration[3];


		float invMass = 0.0f;
		float damping = 0.0f;

		bool forward = false;
		bool backward = false;
		bool left = false;
		bool right = false;

		float enemyPos[3];
	};

	Packet scenePacket(const game::Scene& scene);
	void Serialize(const Packet& packet, char* buffer);
	void Deserialize(const char* buffer, Packet& packet);
}