#include "Server_Packet.h"


void Serialize(const Packet& packet, char* buffer)
{
	memcpy(buffer, &packet, sizeof(Packet));
}

void Deserialize(const char* buffer, Packet& packet)
{
	memcpy(&packet, buffer, sizeof(Packet));
}