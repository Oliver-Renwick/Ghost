#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "Server_Packet.h"

struct Player
{
	uint32_t ID_PROT = 0;
	uint32_t curerntSequenceNumber = 0;
	uint32_t lastSequenceNumber = 0;

	float m_position[3];
	float m_velocity[3];
	float m_acceleration[3];
	
	float m_damping = 0.0f;
	float m_invMass = 0.0f;

	std::string m_clientKey;
	sockaddr_in m_clientaddr;
};

struct Server
{
	WSAData m_wsaData;
	sockaddr_in m_serveraddr;
	SOCKET m_serverSocket;

	Player* player1 = nullptr;
	Player* player2 = nullptr;

	Packet receivedPacket{};
	std::string currentClientKey;

	using _clock = std::chrono::high_resolution_clock;


	void initializeServer();
	void sendData();
	void receiveData();
	void cleanUp();
	void connectClients(Packet& packet, sockaddr_in& client_addr);

	void sendDatatoClient(Packet packet, sockaddr_in& client_addr);
	void DebugPlayerState(const Packet& packet);
	void PlayerUpdate(Player* player, Player* enemy, Packet& packet);
	void EnemyUpdate(Player* enemy, Player* player, Packet& packet);
};

