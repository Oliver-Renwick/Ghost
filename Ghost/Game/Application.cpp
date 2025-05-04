#include "Application.h"
#include "Input.h"

constexpr double fixedTimeStep = 1.0 / 120.0;

struct glTFDrawTask : public enki::ITaskSet {
	void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override {
		std::cout << "This task is running on the thread number => " << threadnum_ << std::endl;
	}

};

void submit_draw_command(enki::TaskScheduler* schedule)
{
	glTFDrawTask draw_task;
	schedule->AddTaskSetToPipe(&draw_task);
	schedule->WaitforTask(&draw_task);
}


// IOTasks ////////////////////////////////////////////////////////////////
//
//

struct RunPinnedTaskLoopTask : enki::IPinnedTask
{
	void Execute() override
	{
		while (execute)
		{

			task_Scheduler->WaitForNewPinnedTasks();// this thread will 'sleep' until there are new pinned tasks

			task_Scheduler->RunPinnedTasks();
		}
	}

	enki::TaskScheduler* task_Scheduler;
	bool				 execute	     = true;
};// RunPinned Task Loop;

struct AsynchronousLoadTask : enki::IPinnedTask
{
	void Execute() override
	{
		while (execute)
		{
			async_Loader->Update();
		}
	}

	graphics::AsynchronousLoader* async_Loader;
	enki::TaskScheduler*		  task_scheduler;
	bool						  execute;

};

namespace game
{

	Application::~Application()
	{
		scene.Clear();
		for (int i = 0; i < m_models.size(); i++)
		{
			m_models[i]->Cleanup();
		}
		for (int i = 0; i < m_gltfModel.size(); i++)
		{
			m_gltfModel[i]->Destroy();
		}
		m_materials.Destroy();
		Skybox.Destroy();
		render.Shutdown();
		commandBuffer.shutDown();
		GpuDevice.shutDown(&deviceInfo);
		client.CleanUp();
	}

	void Application::InitCamera()
	{
		cam.SetPerspective(glm::radians(45.0f), (float)deviceInfo.width / (float)deviceInfo.height, 0.1f, 2560.0f);
		cam.lookat(glm::vec3(scene.player->m_position.x, -scene.player->m_position.y, scene.player->m_position.x));
	}

	void Application::InitAssets(graphics::AsynchronousLoader* asyncLoader)
	{
		//Materials
		m_materials.InitMaterials(&deviceInfo);
		asyncLoader->request_texture_request("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/metalplate01_rgba.ktx");
		asyncLoader->request_texture_request("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/ground_dry_rgba.ktx");
		asyncLoader->request_texture_request("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/gridlines.ktx");

		//m_materials.m_paths.push_back("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/metalplate01_rgba.ktx");
		//m_materials.m_paths.push_back("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/ground_dry_rgba.ktx");
		//m_materials.m_paths.push_back("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/gridlines.ktx");
	}

	bool Application::Initialize()
	{
		//Client Initialization
		client.SetUpClient();

		//Initializing Task Scheduler
		TSconfig.numTaskThreadsToCreate;

		m_taskScheduler.Initialize(TSconfig); 



		//Window Initialization
		win.Setup_Window();
		deviceInfo.hInst = win.m_hInst;
		deviceInfo.window = win.m_window;

		if (GetClientRect(win.m_window, &rect))
		{
			deviceInfo.width = rect.right - rect.left;
			deviceInfo.height = rect.bottom - rect.top;
		}


		//Vulkan Initialization
		GpuDevice.Init(&deviceInfo);
		commandBuffer.Init(&deviceInfo, &offscreenInfo, &Skybox);

		//Async Loader
		//Asset Initialization
		m_asyncLoader.Init(&deviceInfo, &m_materials);
		InitAssets(&m_asyncLoader);

		// Start multithreading IO
		// Create IO threads at the end
	

		//Scene System
		
		scene.Initialize();
		InitCamera();

		//m_materials.InitMaterials(&deviceInfo, VK_FORMAT_R8G8B8A8_SRGB);

		m_models.reserve(scene.m_bodies.size());
		m_gltfModel.reserve(scene.m_gltfBodies.size());

		for (int i = 0; i < scene.m_bodies.size(); i++)
		{
			graphics::Model* model = new graphics::Model();
			model->BuildShape(scene.m_bodies[i]->m_shape);
			model->ApplyColor(scene.m_bodies[i]->m_color);
			model->AssignMaterial(scene.m_bodies[i]->m_materialID);
			model->MakeVBO(&deviceInfo);

			m_models.push_back(model);
		}

		for (int i = 0; i < scene.m_gltfBodies.size(); i++)
		{
			graphics::GltfModel* model = new graphics::GltfModel();
			const phy::Shape_Gltf* shape_gltf = (const phy::Shape_Gltf*)scene.m_gltfBodies[i]->m_shape;
			model->loadGLTF(shape_gltf->gltfPath, &deviceInfo);
			model->AssignMaterial(scene.m_gltfBodies[i]->m_materialID);
			m_gltfModel.push_back(model);
		}

		for (int i = 0; i < scene.m_bodies.size(); i++)
		{
			phy::Body* body = scene.m_bodies[i];

			//ToDo model Matrix Calculation here
			m_models[i]->translate = glm::vec3(body->m_Pos.x, body->m_Pos.y, body->m_Pos.z);
			m_models[i]->rotation = glm::vec3(body->m_orientation.x, body->m_orientation.y, body->m_orientation.z);
			m_models[i]->scale = glm::vec3(1.0f, 1.0f, 1.0f);

			m_models[i]->CreateModelMatrix();

			graphics::RenderModel renderModel;
			
			renderModel.model = m_models[i];
			m_renderModel.push_back(renderModel);
		}

		for (int i = 0; i < scene.m_gltfBodies.size(); i++)
		{
			phy::Body* body = scene.m_gltfBodies[i];

			m_gltfModel[i]->translate = glm::vec3(body->m_Pos.x, body->m_Pos.y, body->m_Pos.z);
			m_gltfModel[i]->rotation = glm::vec3(body->m_orientation.x, body->m_orientation.y, body->m_orientation.z);
			m_gltfModel[i]->scale = glm::vec3(10.0f, 10.0f, 10.0f);

			m_gltfModel[i]->CreateModelMatrix();

			graphics::RenderModel renderModel;
			renderModel.gltfModel = m_gltfModel[i];
			m_renderModel.push_back(renderModel);
		}

		// TODO:-  Create Asset System

		Skybox.createCubemap("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Textures/cubemap_vulkan.ktx", VK_FORMAT_R8G8B8A8_SRGB, &deviceInfo);
		// This is where all the render target are and the rendering is done in here.
		render.Init(&deviceInfo, &offscreenInfo, &Skybox, &m_materials);
		commandBuffer.CreateDescriptorSet();
		render.preparePipeline();

		//Should be called after all the pipeline creation
		commandBuffer.SavePipelineCache();

		return true;
	}

	void Application::Run()
	{

		//Time
		using _clock = std::chrono::high_resolution_clock;
		auto previousTime = _clock::now();
		double accumulatedTime = 0.0;

		RunPinnedTaskLoopTask run_pinned_task;
		run_pinned_task.threadNum = m_taskScheduler.GetNumTaskThreads() - 1;
		run_pinned_task.task_Scheduler = &m_taskScheduler;
		m_taskScheduler.AddPinnedTask(&run_pinned_task);
		
		// Send async load task to external thread FILE_IO
		AsynchronousLoadTask async_load_task;
		async_load_task.threadNum = run_pinned_task.threadNum;
		async_load_task.async_Loader = &m_asyncLoader;
		async_load_task.task_scheduler = &m_taskScheduler;
		m_taskScheduler.AddPinnedTask(&async_load_task);

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

				//std::cout << "Player Position => " << scene.player->m_position.x << ", " << scene.player->m_position.y << ", " << scene.player->m_position.z << std::endl;
				//std::cout << "Enemy Position => " << scene.enemy->m_position.x << ", " << scene.enemy->m_position.y << ", " << scene.enemy->m_position.z << std::endl;

				//NetCode
				
				sendPacket = net::scenePacket(scene);
				client.SendData(sendPacket);

				client.ReceiveData(receivePacket);
				if (receivePacket.ID_PROTO == 1234)
				{
					scene.player->m_acceleration.x = receivePacket.acceleration[0]; scene.player->m_acceleration.y = receivePacket.acceleration[1]; scene.player->m_acceleration.z = receivePacket.acceleration[2];
					scene.player->m_velocity.x = receivePacket.velocity[0]; scene.player->m_velocity.y = receivePacket.velocity[1]; scene.player->m_velocity.z = receivePacket.velocity[2];
					scene.player->m_position.x = receivePacket.position[0]; scene.player->m_position.y = receivePacket.position[1]; scene.player->m_position.z = receivePacket.position[2];
					scene.enemy->m_position.x = receivePacket.enemyPos[0]; scene.enemy->m_position.y = receivePacket.enemyPos[1]; scene.enemy->m_position.z = receivePacket.enemyPos[2];
				}

				//Time Code
				auto Currentime = _clock::now();
				std::chrono::duration<double> frameTime = Currentime - previousTime;
				previousTime = Currentime;


				scene.Update(static_cast<float>(fixedTimeStep) + 0.003f);


				if (!IsIconic(win.m_window) && prepare)
				{
					//submit_draw_command(&m_taskScheduler);
					UpdateUniformBufferOffScreen();
					UpdateUniformBuffer();
					render.DrawFrame(m_renderModel);
				}

				

			}

			// Multi-Threading Shutdown 
			run_pinned_task.execute = false;
			async_load_task.execute = false;

			m_taskScheduler.WaitforAllAndShutdown();
			m_asyncLoader.Shutdown();
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

		Vec3 pos = Vec3(0.0f, 0.0f, 0.0f);
		//cam.UpdateOrbitCamera(scene.player->m_position);
		cam.UpdateOrbitCamera(pos);

		

		//Camera Data
		ubo.proj = cam.proj;
		ubo.view = cam.view;
		ubo.viewPos = cam.pos;
		ubo.depthBiasMVP = offScreenUBO.depthMVP;
		ubo.zNear = offscreenInfo.zNear;
		ubo.zFar = offscreenInfo.zFar;

		commandBuffer.UniformBuffer.copyTo(&ubo, sizeof(ubo));
		//std::cout << cam.pos.x << ", " << cam.pos.y << ", " << cam.pos.z << std::endl;
		for (int i = 0; i < scene.m_bodies.size(); i++)
		{
			phy::Body* body = scene.m_bodies[i];

			m_models[i]->translate = glm::vec3(body->m_Pos.x, body->m_Pos.y, body->m_Pos.z);
			m_models[i]->rotation = glm::vec3(body->m_orientation.x, body->m_orientation.y, body->m_orientation.z);
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
			case Key_W:
				input.forward = true;
				break;

			case Key_S:
				input.backward = true;
				break;

			case Key_A:
				input.left = true;
				break;

			case Key_D:
				input.right = true;
				break;

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


		case WM_KEYUP:
		{
			switch (wParam)
			{
			case Key_W:
				input.forward = false;
				break;

			case Key_S:
				input.backward = false;
				break;

			case Key_A:
				input.left = false;
				break;

			case Key_D:
				input.right = false;
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