#pragma once

#include "Windows.h"
#include "Graphics/Vulkan_Init.h"
#include "Graphics/Command_Buffer.h"
#include "Graphics/Render.h"
#include "GameObject.h"
#include "Input.h"
#include "Game_Window.h"
#include "Camera.h"
#include "Scene.h"
#include "Graphics/Model.h"
#include "UI/UI.h"
#include <chrono>

namespace game
{
	struct Application
	{
		graphics::DeviceInfo deviceInfo{};
		graphics::OffscreenData offscreenInfo{};
		graphics::GPUDevice GpuDevice{};
		graphics::CommandBuffer commandBuffer{};
		graphics::Renderer render{};
		std::vector<graphics::Model*> m_models;
		std::vector<graphics::RenderModel> m_renderModel;


		game::Input input{};
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

		ui::UI GUI{};

		void Handle_Message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		Application() = default;
		~Application();
		void InitCamera();
		bool Initialize();
		void UpdateUniformBuffer();
		void UpdateUniformBufferOffScreen();
		void Run();
	};
}
