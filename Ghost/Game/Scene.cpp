#include "Scene.h"

namespace game
{
	void Scene::Initialize()
	{
		phy::Body body{};
		body.m_Pos = Vec3(0.0f, 10.0, 0.0f);
		body.m_color = Vec3(0.8f, 0.0f, 0.0f);
		body.m_invMass = 1.0f;
		body.m_elasticity = 0.5f;
		body.m_shape = new phy::Shape_Sphere(1.0f);
		m_bodies.push_back(body);


		body.m_Pos = Vec3(5.0f, 10.0, 0.0f);
		body.m_color = Vec3(0.8f, 0.0f, 0.0f);
		body.m_invMass = 1.0f;
		body.m_elasticity = 0.5f;
		body.m_shape = new phy::Shape_Sphere(1.0f);
		m_bodies.push_back(body);
		
		body.m_Pos = Vec3(-5.0f, 10.0, 0.0f);
		body.m_color = Vec3(0.8f, 0.0f, 0.0f);
		body.m_invMass = 1.0f;
		body.m_elasticity = 0.5f;
		body.m_shape = new phy::Shape_Sphere(1.0f);
		m_bodies.push_back(body);

		body.m_Pos = Vec3(0.0f, -5.0f, 0.0f);
		body.m_color = Vec3(0.7f, 0.4f, 0.0f);
		body.m_invMass = 0.0f;
		body.m_elasticity = 1.0f;
		body.m_shape = new phy::Shape_Box(1000.0f, 2.0f, 1000.0f);
		m_bodies.push_back(body);
	}

	void Scene::Clear()
	{
		for (int i = 0; i < m_bodies.size(); i++)
		{
			delete m_bodies[i].m_shape;
		}
		m_bodies.clear();
	}

	void Scene::Reset()
	{
		for (int i = 0; i < m_bodies.size(); i++)
		{
			delete m_bodies[i].m_shape;
		}
		m_bodies.clear();
		Initialize();
	}

	void Scene::Update(const float dt_sec)
	{
		for (int i = 0; i < m_bodies.size(); i++)
		{
			float mass = 1.0f / m_bodies[i].m_invMass;
			Vec3 impulse = Vec3(0.0f,-10.0f,0.0f) * mass * dt_sec;
			m_bodies[i].ApplyImpulseLinear(impulse);
		}

		// For Ground Interactions
		int end = m_bodies.size() - 1;
		for (int i = 0; i < m_bodies.size() - 1; i++)
		{
			phy::Body * A = &m_bodies[i];
			phy::Body * B = &m_bodies[end];

			if (A->m_invMass == 0.0 && B->m_invMass == 0.0)
				continue;

			phy::contact_t contact;
			if (phy::sphereBoxIntersection(A, B, contact))
			{
				phy::ResolveContact(contact);
			}
		}

		for (int i = 0; i < m_bodies.size(); i++)
		{
			m_bodies[i].m_Pos += m_bodies[i].m_LinearVelocity * dt_sec;
		}
	}
}