#include "Player.h"
#include "Input.h"

namespace game
{
	void Player::Initialize()
	{
		body.m_Pos = m_position;
		body.m_orientation = m_orientation;
		body.m_color = Vec3(0.8f, 0.0f, 0.0f);
		body.m_shape = new phy::Shape_Sphere(m_size);
	}

	void Player::MoveState()
	{
		m_forward = input.forward;
		m_backward = input.backward;
		m_left = input.left;
		m_right = input.right;
	}



	void Player::Update_Player()
	{
		body.m_Pos = m_position;
	}

	void Player::setAcceleration(float x, float y, float z)
	{
		m_acceleration.x = x;
		m_acceleration.y = y;
		m_acceleration.z = z;
	}

	void Player::setMass(float mass)
	{
		assert(mass > 0.0f);
		m_invMass = 1.0f / mass;
	}

	void Player::setSize(float size)
	{
		m_size = size;
	}

	void Player::setVelocity(float x, float y, float z)
	{
		m_velocity.x = x;
		m_velocity.y = y;
		m_velocity.z = z;
	}

	void Player::setPosition(float x, float y, float z)
	{
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
	}

	void Player::setDamping(float damping)
	{
		m_damping = damping;
	}
}
