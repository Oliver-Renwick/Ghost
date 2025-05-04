#ifdef _DEBUG
#define Result(err,msg);

#else
#define Result(err,msg) std::cout << err << " : " << msg << std::endl;


#endif 


#include "Client.h"


namespace net
{
	void Client::SetUpClient()
	{
		//WSA Startup
		if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
		{
			throw std::runtime_error("Unable to start Windows Socket API");
		}
		else
		{
			Result("SYSTEM", m_wsaData.szSystemStatus);
		}

		//Socket Initialization
		m_clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (m_clientSocket == INVALID_SOCKET)
		{
			Result("Error : " + std::to_string(WSAGetLastError()), "Unable to create client socket");
			throw std::runtime_error("Error on creating socket");
			
		}
		else
		{
			Result("SUCCESS", "Successfully Created Socket");
		}


		m_server_addr.sin_family = AF_INET;
		if (InetPton(AF_INET, L"192.168.1.5", &m_server_addr.sin_addr) != 1)
		{
			Result("ERROR", "Invalid Ip Address");
		}
		m_server_addr.sin_port = htons(55555);  

		u_long mode = 0; // change to 1 if you need non blocking
		ioctlsocket(m_clientSocket, FIONBIO, &mode);
	}


	void Client::Connection()
	{
		// TODO:-
	}


	void Client::SendData(Packet& packet)
	{
		
			char buffer[1024];

			clientSequenceNumber++;
			packet.sequenceNumber = clientSequenceNumber;

			Serialize(packet, buffer);

			int sendMessage = sendto(m_clientSocket, buffer, sizeof(Packet), 0, (SOCKADDR*)&m_server_addr, sizeof(m_server_addr));

			if (sendMessage == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error == WSAEWOULDBLOCK)
				{
					std::cout << "Send would block, retrying..." << std::endl;
					Sleep(10);
				}

				else
				{
					Result("ERROR " + std::to_string(WSAGetLastError()), "Failed To Send Message to Server");
				}
			}
		

	}

	void Client::ReceiveData(Packet& packet)
	{
		char buffer[1024];
		sockaddr_in server_addr;
		int serveraddrlen = sizeof(server_addr);

		int receiveBytes = recvfrom(m_clientSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&server_addr, &serveraddrlen);

		if (receiveBytes == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
			{
				return;
			}

			Result("ERROR " + std::to_string(error), "Failed to Receive message from server");
		}

		buffer[(receiveBytes < sizeof(buffer) - 1) ? receiveBytes : (sizeof(buffer) - 1)] = '\0';

		Deserialize(buffer, packet);

	//	std::cout << "Sequence Number : " << clientSequenceNumber << std::endl;
	//	std::cout << "Packet Player Position : " << packet.position[0] << ", " << packet.position[1] << ", " << packet.position[2] << std::endl;
	//	std::cout << "Packet Enemy Position : " << packet.enemyPos[0] << ", " << packet.enemyPos[1] << ", " << packet.enemyPos[2] << std::endl;
	
	}
		
	void Client::CleanUp()
	{
		closesocket(m_clientSocket);
		WSACleanup();
	}

}