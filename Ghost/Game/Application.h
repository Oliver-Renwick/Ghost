#pragma once

#include "Networking/Client.h"
#include "Windows.h"
#include "Graphics/Vulkan_Init.h"
#include "Graphics/Command_Buffer.h"
#include "Graphics/Render.h"
#include "Game_Window.h"
#include "Camera.h"
#include "Scene.h"
#include "Graphics/Asynchronous_Loader.h"
#include "Graphics/Model.h"
#include "Graphics/Material.h"
#include "Graphics/SkyBox.h"
#include <chrono>

namespace game
{
	struct Application
	{
		graphics::DeviceInfo deviceInfo{};
		graphics::OffscreenData offscreenInfo{};
		graphics::skybox  Skybox{};
		graphics::GPUDevice GpuDevice{};
		graphics::CommandBuffer commandBuffer{};
		graphics::Renderer render{};
		std::vector<graphics::Model*> m_models;
		std::vector<graphics::GltfModel*> m_gltfModel;
		std::vector<graphics::RenderModel> m_renderModel;
		graphics::AsynchronousLoader m_asyncLoader;
		enki::TaskSchedulerConfig TSconfig;
		enki::TaskScheduler m_taskScheduler;
		

		Win win{};
		RECT rect;
		bool running = true;
		bool prepare = true;
		bool physics_update = false;
		float dt = 0.006f;
		Camera cam{};
		Scene scene;

		graphics::UboInfo ubo{};
		graphics::UboInfoOffscreen offScreenUBO{};
		graphics::MaterialManager m_materials;

		net::Client client;
		net::Packet sendPacket{};
		net::Packet receivePacket{};


		void Handle_Message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		Application() = default;
		~Application();
		void InitCamera();
		void InitAssets(graphics::AsynchronousLoader* async);
		bool Initialize();
		void UpdateUniformBuffer();
		void UpdateUniformBufferOffScreen();
		void Run();
	};
}
