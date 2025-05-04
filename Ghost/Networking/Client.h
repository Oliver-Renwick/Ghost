#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Packet.h"

#include <winsock2.h>
#include <ws2tcpip.h>


namespace net
{
	struct Client
	{
		//Data
		WSAData m_wsaData;
		sockaddr_in m_server_addr;
		SOCKET m_clientSocket;


		uint32_t clientSequenceNumber = 0;

		Packet scene_packet;

		void SetUpClient();
		void Connection();
		void SendData(Packet& packet);
		void ReceiveData(Packet& packet);
		void CleanUp();
	};
}