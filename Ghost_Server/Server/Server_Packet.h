#pragma once

#include <iostream>

struct Packet
{
	uint32_t ID_PROT = 0;
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



void Serialize(const Packet& packet, char* buffer);
void Deserialize(const char* buffer, Packet& packet);