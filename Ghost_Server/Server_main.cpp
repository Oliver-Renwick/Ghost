#include "Server/Server.h"
#include "Server/Physics.h"

int main()
{
	using _clock = std::chrono::high_resolution_clock;

	auto lastTime = _clock::now();

	Server server;

	phy::gameobj Player;

	server.initializeServer();
	while (true)
	{

		auto currentTime = _clock::now();
		std::chrono::duration<float> deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		float duration = deltaTime.count() + 0.03f; //  = 0.032f change it if there is jump in duration

		server.receiveData();
		//Physics Update
	//	std::cout << " ------------ DURATION --------------: " << duration << std::endl;
		Player.Integrator(server.receivedPacket, duration);
		server.sendData();
	}
	server.cleanUp();

	system("pause");
}