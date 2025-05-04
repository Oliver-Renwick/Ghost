#include "Scene.h"

namespace game
{
	void Scene::Initialize()
	{

		player = new Player();

		player->setPosition(-10.0f, -2.6f, 0.0f);
		player->setMass(200.0f);
		player->setVelocity(0.0f, 0.0f, 0.0f);
		player->setAcceleration(0.0f, 0.0f, 0.0f);
		player->setDamping(0.99f);
		player->setSize(1.2f);
		player->body.m_materialID = 0;
		
		player->Initialize();

		m_bodies.push_back(&player->body);


		enemy = new Player();
		
		enemy->setPosition(10.0f, -2.6f, 0.0f);
		enemy->setMass(200.0f);
		enemy->setVelocity(0.0f, 0.0f, 0.0f);
		enemy->setAcceleration(0.0f, 0.0f, 0.0f);
		enemy->setDamping(0.99f);
		enemy->setSize(1.2f);
		enemy->body.m_materialID = 0;
	
		enemy->Initialize();

		m_bodies.push_back(&enemy->body);


		floor.m_Pos = Vec3(0.0f, -5.0f, 0.0f);
		floor.m_color = Vec3(0.8f, 0.0f, 0.0f);
		floor.m_materialID = 2;
		floor.m_shape = new phy::Shape_Box(200.0f, 2.0f, 200.0f);
		m_bodies.push_back(&floor);

		gltfTest.m_Pos = Vec3(0.0f, 0.5f, 0.0f);
		gltfTest.m_color = Vec3(0.8f, 0.0f, 0.0f);
		gltfTest.m_materialID = 1;
		gltfTest.m_shape = new phy::Shape_Gltf("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Models/FlightHelmet/glTF/FlightHelmet.gltf");
		m_gltfBodies.push_back(&gltfTest);
	}

	void Scene::Clear()
	{
		for (int i = 0; i < m_bodies.size(); i++)
		{
			m_bodies[i]->Delete();
		}

		for (int i = 0; i < m_gltfBodies.size(); i++)
		{
			m_gltfBodies[i]->Delete();
		}

		m_bodies.clear();

		if (player)
		{
			delete player;
		}

		if (enemy)
		{
			delete enemy;
		}
	}

	void Scene::Reset()
	{
		Clear();
		Initialize();
	}

	void Scene::Update(const float dt_sec)
	{
		player->MoveState();
		player->Update_Player();
		enemy->Update_Player();
	}
}