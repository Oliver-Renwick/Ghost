#ifdef _DEBUG
#define Result(err,msg) std::cout << err << " : " << msg << std::endl;

#else

#define Result(err,msg);

#endif 

#include "Server.h"


void Server::initializeServer()
{
	//Initialize WSA
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
	{
		std::cout << "Unable to start WSA" <<  std::endl;
	}
	else
	{
		Result("SYSTEM", m_wsaData.szSystemStatus);
	}

	// Create Socket
	m_serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_serverSocket == INVALID_SOCKET)
	{
		Result("Error : " + std::to_string(WSAGetLastError()), "Unable to create Server socket");
	}
	else
	{
		Result("SUCCESS", "Successfully Created Socket");
	}

	//Binding Socket
	m_serveraddr.sin_family = AF_INET;
	if (InetPton(AF_INET, L"192.168.1.5", &m_serveraddr.sin_addr) != 1)
	{
		Result("ERROR", "Invalid Ip Address");
	}
	m_serveraddr.sin_port = htons(55555);

	if (bind(m_serverSocket, (SOCKADDR*)&m_serveraddr, sizeof(m_serveraddr)) == SOCKET_ERROR)
	{
		Result("ERROR " + std::to_string(WSAGetLastError()), "Unable to bind the socket");
		closesocket(m_serverSocket);
		WSACleanup();
	}

	else
	{
		Result("SUCCESS", "Successfully binded Socket");
	}

	u_long mode = 0; // Change it to 1 when you need non blocking
	ioctlsocket(m_serverSocket, FIONBIO, &mode);

}



void Server::connectClients(Packet& packet, sockaddr_in& clientaddr)
{

	

	char ipStr[INET_ADDRSTRLEN] = { 0 };
	if (!InetNtopA(AF_INET, &clientaddr.sin_addr, ipStr, INET_ADDRSTRLEN))
	{
		std::cerr << "Failed to convert client IP address.\n";
		return;
	}

	std::string clientKey = std::string(std::string(ipStr) + ':' + std::to_string(ntohs(clientaddr.sin_port)));
	currentClientKey = clientKey;
	bool Player1Key = (player1 && (clientKey == player1->m_clientKey)) ? true : false;

	if (!player1)
	{
		player1							= new Player();
		player1->m_clientKey			= clientKey;
		player1->ID_PROT				= packet.ID_PROT;
		player1->curerntSequenceNumber  = packet.sequenceNumber;
		player1->m_clientaddr			= clientaddr;
	}


	else if (!player2 && !Player1Key)
	{
		player2							= new Player();
		player2->m_clientKey			= clientKey;
		player2->ID_PROT				= packet.ID_PROT;
		player2->curerntSequenceNumber  = packet.sequenceNumber;
		player2->m_clientaddr			= clientaddr;

		//Temporary High-tech Spawning system
		packet.position[0] = 10.0f;
		packet.position[1] = 10.0f;
		packet.position[2] = 0.0f;

		packet.velocity[0] = 2.0f;
	}

}

void Server::PlayerUpdate(Player* player, Player* enemy, Packet& packet)
{
	//caching players data
	player->m_position[0] = packet.position[0];
	player->m_position[1] = packet.position[1];
	player->m_position[2] = packet.position[2];

	player->m_velocity[0] = packet.velocity[0];
	player->m_velocity[1] = packet.velocity[1];
	player->m_velocity[2] = packet.velocity[2];

	player->m_acceleration[0] = packet.acceleration[0];
	player->m_acceleration[1] = packet.acceleration[1];
	player->m_acceleration[2] = packet.acceleration[2];

	player->m_invMass = packet.invMass;
	player->m_damping = packet.damping;

	if (enemy != nullptr)
	{
		packet.enemyPos[0] = enemy->m_position[0];
		packet.enemyPos[1] = enemy->m_position[1];
		packet.enemyPos[2] = enemy->m_position[2];
	}

	else
	{
		packet.enemyPos[0] = 10.0f;
		packet.enemyPos[1] = 10.0f;
		packet.enemyPos[2] = 0.0f;
	}
}

void Server::EnemyUpdate(Player* enemy, Player* player, Packet& packet)
{
	if (!enemy || !player) return;

	packet.enemyPos[0] = player->m_position[0];
	packet.enemyPos[1] = player->m_position[1];
	packet.enemyPos[2] = player->m_position[2];

	packet.position[0] = enemy->m_position[0];
	packet.position[1] = enemy->m_position[1];
	packet.position[2] = enemy->m_position[2];

	packet.velocity[0] = enemy->m_velocity[0];
	packet.velocity[1] = enemy->m_velocity[1];
	packet.velocity[2] = enemy->m_velocity[2];

	packet.acceleration[0] = enemy->m_acceleration[0];
	packet.acceleration[1] = enemy->m_acceleration[1];
	packet.acceleration[2] = enemy->m_acceleration[2];

	packet.invMass = enemy->m_invMass;
	packet.damping = enemy->m_damping;
}

void Server::sendDatatoClient(Packet packet, sockaddr_in& clientaddr)
{
	char buffer[1024];
	Serialize(packet, buffer);
	int sendBytes = sendto(m_serverSocket, buffer, sizeof(Packet), 0, (SOCKADDR*)&clientaddr, sizeof(clientaddr));

	if (sendBytes == SOCKET_ERROR)
	{
		Result("ERROR " + std::to_string(WSAGetLastError()), "Failed to send message to client -> " + std::to_string(ntohs(clientaddr.sin_port)));
	}

}


void Server::sendData()
{
	if (player1 != nullptr)
	{
		if (currentClientKey == player1->m_clientKey)
		{
			//std::cout << "---------PLAYER 1------------" << std::endl;
			//std::cout << "-------CLIENT KEY--------- => " << currentClientKey << std::endl;
			//std::cout << "DEBUGGING PACKETS AFTER PHYSICS => " << std::endl;
			DebugPlayerState(receivedPacket);

			Packet packet = receivedPacket;
			PlayerUpdate(player1, player2, packet);

			//send data to the player 1
			{
				sendDatatoClient(packet, player1->m_clientaddr);
			}

			packet = receivedPacket;

			EnemyUpdate(player2, player1, packet);

			//send data to player 2
			{
				if (player2 != nullptr)
				{
					sendDatatoClient(packet, player2->m_clientaddr);
				}
			}
		}
	}


	if (player2 != nullptr)
	{
		if (currentClientKey == player2->m_clientKey)
		{
			//std::cout << "---------PLAYER 2------------" << std::endl;
			//std::cout << "-------CLIENT KEY--------- => " << currentClientKey << std::endl;
			//std::cout << "DEBUGGING PACKETS AFTER PHYSICS => " << std::endl;
			DebugPlayerState(receivedPacket);
			Packet packet = receivedPacket;

			PlayerUpdate(player2, player1, packet);

			//send Data to player 2
			{
				sendDatatoClient(packet, player2->m_clientaddr);
			}

			packet = receivedPacket;

			EnemyUpdate(player1, player2, packet);

			//send data to player 1
			{
				if (player1 != nullptr)
				{
					sendDatatoClient(packet, player1->m_clientaddr);
				}
			}
		}
	}

	
}



void Server::DebugPlayerState(const Packet& packet)
{
	//std::cout << "Sequence Number : " << packet.sequenceNumber << std::endl;
	//std::cout << "Protocol ID : " << packet.ID_PROT << std::endl;
	//std::cout << "Recieved Packet Position -> " << packet.position[0] << ", " << packet.position[1] << ", " << packet.position[2] << std::endl;
	//std::cout << "Recieved Packet Acceleration -> " << packet.acceleration[0] << ", " << packet.acceleration[1] << ", " << packet.acceleration[2] << std::endl;
	//std::cout << "Recieved Packet Velocity -> " << packet.velocity[0] << ", " << packet.velocity[1] << ", " << packet.velocity[2] << std::endl;
	//
	//std::cout << "Inverse Mass : " << packet.invMass << std::endl;
	//std::cout << "Damping : " << packet.damping << std::endl;
	if (packet.forward || packet.backward || packet.left || packet.right)
	{
		if (currentClientKey == player1->m_clientKey) { std::cout << " Player 1 " << std::endl; }
		else { std::cout << " Player 2 " << std::endl; }

		if (packet.forward) { std::cout << "Player Pressed Forward Button" << std::endl; }
		if (packet.backward) { std::cout << "Player Pressed Backward Button" << std::endl; }
		if (packet.left) { std::cout << "Player Pressed Left Button" << std::endl; }
		if (packet.right) { std::cout << "Player Pressed Right Button" << std::endl; }
	}
}

void Server::receiveData()
{
	receivedPacket = {};
	char buffer[1024];
	sockaddr_in clientaddr;

	int clientAddrSize = sizeof(clientaddr);

	int bytesReceived = recvfrom(m_serverSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&clientaddr, &clientAddrSize);

	if (bytesReceived == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			return;
		}

		Result("ERROR " + std::to_string(error), "Failed to recive the Data");
		return;

	}


	buffer[(bytesReceived < sizeof(buffer) - 1) ? bytesReceived : (sizeof(buffer) - 1)] = '\0';

	
	Packet packet{};
	Deserialize(buffer, packet);


	if (packet.ID_PROT != 1234)
	{
		return;
	}

	connectClients(packet, clientaddr);

	receivedPacket = packet;

//	if (packet.backward)
//	{
//		std::cout << "Lets start Debugging" << std::endl;
//		receivedPacket.position[0] = 20.0f;
//	}

	//std::cout << "DEBUGGING PACKETS BEFORE PHYSICS => " << std::endl;
	DebugPlayerState(receivedPacket);
	 
}

void Server::cleanUp()
{
	if (player1)
	{
		delete player1;
	}

	if (player2)
	{
		delete player2;
	}

	closesocket(m_serverSocket);
	WSACleanup();

}