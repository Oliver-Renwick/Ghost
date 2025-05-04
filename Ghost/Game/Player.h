#pragma once

#include "Physics/Body.h"
#include "Physics/Shapes/Sphere_Shape.h"
#include "Physics/Shapes/Box_Shape.h"

namespace game
{
	struct Player
	{
		//Data
		phy::Body body;

		Vec3  m_position;
		Quat  m_orientation;
		Vec3  m_velocity;
		Vec3  m_acceleration;
		float m_size = 1.0f;

		float m_invMass = 0;
		float m_damping = 0;


		//MoveState
		bool m_forward  = false;
		bool m_backward = false;
		bool m_left     = false;
		bool m_right    = false;

		void Initialize();
		void MoveState();
		void Update_Player();

		void setAcceleration(float x, float y, float z);
		void setVelocity(float x, float y, float z);
		void setPosition(float x, float y, float z);
		void setMass(float mass);
		void setSize(float size);
		void setDamping(float damping);
	};
}
