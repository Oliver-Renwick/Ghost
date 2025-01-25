#include "Application.h"

namespace game
{

	Application::~Application()
	{
		GUI.Cleanup();
		scene.Clear();
		for (int i = 0; i < m_models.size(); i++)
		{
			m_models[i]->Cleanup();
		}
		render.Shutdown();
		commandBuffer.shutDown();
		GpuDevice.shutDown(&deviceInfo);
	}

	void Application::InitCamera()
	{
		cam.pos = glm::vec3(95.6418, -51.7139, 98.6336);
		cam.SetPerspective(glm::radians(45.0f), (float)deviceInfo.width / (float)deviceInfo.height, 0.1f, 2560.0f);
		cam.lookat(glm::vec3(0.0f, 1.0f, 0.0f));
	}

	bool Application::Initialize()
	{

		//Window Initialization
		win.Setup_Window();
		deviceInfo.hInst = win.m_hInst;
		deviceInfo.window = win.m_window;

		if (GetClientRect(win.m_window, &rect))
		{
			deviceInfo.width = rect.right - rect.left;
			deviceInfo.height = rect.bottom - rect.top;
		}

		InitCamera();

		//Vulkan Initialization
		GpuDevice.Init(&deviceInfo);

		//Scene System
		
		scene.Initialize();

		m_models.reserve(scene.m_bodies.size());

		for (int i = 0; i < scene.m_bodies.size(); i++)
		{
			
			graphics::Model* model = new graphics::Model();
			model->BuildShape(scene.m_bodies[i].m_shape);
			model->ApplyColor(scene.m_bodies[i].m_color);
			model->MakeVBO(&deviceInfo);

			m_models.push_back(model);
		}

		for (int i = 0; i < scene.m_bodies.size(); i++)
		{
			phy::Body& body = scene.m_bodies[i];

			//ToDo model Matrix Calculation here
			m_models[i]->translate = glm::vec3(body.m_Pos.x, body.m_Pos.y, body.m_Pos.z);
			m_models[i]->rotation = glm::vec3(body.m_orientation.x, body.m_orientation.y, body.m_orientation.z);
			m_models[i]->scale = glm::vec3(1.0f, 1.0f, 1.0f);

			m_models[i]->CreateModelMatrix();

			graphics::RenderModel renderModel;
			
			renderModel.model = m_models[i];
			m_renderModel.push_back(renderModel);
		}

		commandBuffer.Init(&deviceInfo, &offscreenInfo);
		render.Init(&deviceInfo, &offscreenInfo);
		commandBuffer.CreateDescriptorSet();
		render.preparePipeline();

		//UI
		GUI.m_deviceInfo = &deviceInfo;
		GUI.Init_ImguiUI();

		return true;
	}

	void Application::Run()
	{
		if (win.m_window)
		{
			MSG msg;
			//Run Loop
			bool quitMessageReceived = false;
			while (running) {
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if (msg.message == WM_CLOSE) {
						quitMessageReceived = true;
						break;
					}
				}

				if (!IsIconic(win.m_window) && prepare)
				{
					if (physics_update)
					{
						scene.Update(0.01f);
					}
					UpdateUniformBufferOffScreen();
					UpdateUniformBuffer();
					render.DrawFrame(m_renderModel, &GUI);
				}

			}

			vkDeviceWaitIdle(deviceInfo.Device);
			
		}
	}

	void Application::UpdateUniformBufferOffScreen()
	{

		glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(offscreenInfo.lightFOV), 1.0f, offscreenInfo.zNear, offscreenInfo.zFar);
		glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(ubo.lightPos.x, ubo.lightPos.y, ubo.lightPos.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0, 1, 0));

		offScreenUBO.depthMVP = depthProjectionMatrix * depthViewMatrix;
		commandBuffer.offscreenUniformBuffer.copyTo(&offScreenUBO, sizeof(offScreenUBO));
	}

	void Application::UpdateUniformBuffer()
	{
		if (cam.camRadius < 1.0)
		{
			cam.camRadius = 1.0f;
		}

		cam.UpdateOrbitCamera();

		

		//Camera Data
		ubo.proj = cam.proj;
		ubo.view = cam.view;
		ubo.viewPos = cam.pos;
		ubo.depthBiasMVP = offScreenUBO.depthMVP;
		ubo.zNear = offscreenInfo.zNear;
		ubo.zFar = offscreenInfo.zFar;

		commandBuffer.UniformBuffer.copyTo(&ubo, sizeof(ubo));
		std::cout << cam.pos.x << ", " << cam.pos.y << ", " << cam.pos.z << std::endl;
		for (int i = 0; i < scene.m_bodies.size(); i++)
		{
			phy::Body& body = scene.m_bodies[i];

			m_models[i]->translate = glm::vec3(body.m_Pos.x, body.m_Pos.y, body.m_Pos.z);
			m_models[i]->rotation = glm::vec3(body.m_orientation.x, body.m_orientation.y, body.m_orientation.z);
			m_models[i]->scale = glm::vec3(1.0f, 1.0f, 1.0f);

			m_models[i]->CreateModelMatrix();
		}

	}

	void Application::Handle_Message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		//Input Source
		static POINT lastMousePos = {0, 0};
		static bool dragging = false;

		switch (uMsg)
		{
		case WM_DESTROY:
		{
			prepare = false;
			running = false;

			OutputDebugStringA("WM_Destroy has been called\n");
			//Handle this as error : recreate Window?
		}break;
		case WM_CLOSE:
		{
			prepare = false;
			OutputDebugStringA("WM_Close has been called\n");
			//Handle this for user to ask questions like "are you sure to close this app"?
		}break;

		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
		}break;

		case WM_MOVE:
		{
			OutputDebugStringA("The Window has been moved\n");
		}break;

		case WM_PAINT:
		{
		}break;

		case WM_QUIT:
		{
			OutputDebugStringA("WM_QUIT has been called\n");
		}break;

		case WM_KEYDOWN :
		{
			switch (wParam)
			{
			case Key_T:
				physics_update = (physics_update) ? false : true;
				break;

			case Key_R:
				physics_update = false;
				scene.Reset();
				break;

			case Key_1:
				deviceInfo.displayShadowMap = (deviceInfo.displayShadowMap) ? false : true;
				break;

			}
		}break;

		case WM_LBUTTONDOWN:
		{
			dragging = true;
			GetCursorPos(&lastMousePos);
			ScreenToClient(hwnd, &lastMousePos);

		}break;

		case WM_LBUTTONUP:
		{
			dragging = false;
		}break;

		case WM_MOUSEMOVE:
		{
			if (dragging)
			{
				POINT currentMousePos;
				GetCursorPos(&currentMousePos);
				ScreenToClient(hwnd, &currentMousePos);

				float deltaX = static_cast<float>(currentMousePos.x - lastMousePos.x);
				float deltaY = static_cast<float>(currentMousePos.y - lastMousePos.y);

				cam.Rotate(deltaX, deltaY);

				lastMousePos = currentMousePos;
			}

		}break;

		case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			
			cam.camRadius -= delta * dt;
		}break;

		}
	}
}