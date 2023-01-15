#include "Core/App/App.hpp"

#include <iostream>
#include <filesystem>

#include "Resources/Shader.hpp"
#include "LowRenderer/Model.hpp"
#include "LowRenderer/SkinnedModel.hpp"
#include "LowRenderer/Lightning/Light.hpp"
#include "Resources/Mesh.hpp"
#include "LowRenderer/RotationController.hpp"
#include "LowRenderer/CameraConstraint.hpp"
#include "Core/Maths/Maths.hpp"
#include "Core/Util/SceneLoader.hpp"
#include "LowRenderer/Rendering/RenderCamera.hpp"
#include "LowRenderer/Rendering/ReflectionCamera.hpp"
#include "Resources/ResourceLoader.hpp"
#include "Physics/RigidBody.hpp"
#include "Physics/Colliders/BallCollider.hpp"
#include "Physics/Colliders/BoxCollider.hpp"
#include "Physics/Colliders/CapsuleCollider.hpp"
#include "Scripts/GravityArea.hpp"
#include "Scripts/SceneLoaderArea.hpp"
#include "Scripts/Entity/PlayerController.hpp"
#include "LowRenderer/GameUI/UIButton.hpp"
#include "LowRenderer/GameUI/UIImage.hpp"
#include "LowRenderer/GameUI/UIText.hpp"
#include "LowRenderer/GameUI/UIBindingButton.hpp"
#include "LowRenderer/GameUI/UISceneLoaderButton.hpp"
#include "LowRenderer/GameUI/UIExitButton.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_opengl3.h>
#include <ImGUI/imgui_stdlib.hpp>

#define MAX_LAG_DELTA 1/15.0

const char* const defaultModels[] =
{
	"DefaultResources/Models/Cube.obj",
	"DefaultResources/Models/Sphere.obj",
	"DefaultResources/Models/Cylinder.obj",
	"DefaultResources/Models/Plane.obj",
	"DefaultResources/Models/Torus.obj",
	"DefaultResources/Models/Cone.obj",
	"DebugSphere",
	"DebugCube",
	"DebugCapsuleA",
	"DebugCapsuleB",
	"DebugCapsuleC",
	"DebugPlane",
};

namespace Core::App
{
	void error_callback(int error, const char* description)
	{
		LOG("! Error ! %s", description);
	}

	int App::InitApp(const AppInit& data)
	{
		glfwInit();
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, data.majorVersion);
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, data.minorVersion);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwSetErrorCallback(error_callback);
		//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

		// glfw window creation
		// --------------------
		window = glfwCreateWindow(data.width, data.height, data.name, NULL, NULL);
		if (window == NULL)
		{
			LOG("Failed to create GLFW window");
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);
		windowIcon = Resources::Texture::ReadIcon("DefaultResources/Icon/Icon_48.png");
		if (windowIcon) glfwSetWindowIcon(window,1,windowIcon);
		//glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
		inputs.InitInputs(window, Core::Maths::Int2D(data.width, data.height));
		return InitOpenGL(data);
	}

	int App::InitOpenGL(const AppInit& data)
	{
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LOG("Failed to initialize GLAD; Error code : %d", glad_glGetError());
			return -1;
		}

		GLint flags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(data.glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}

		glfwSwapInterval(1);

		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		InitDatas();
		return 0;
	}

	void App::InitDatas()
	{
#ifdef GAME
		playMode = PlayType::Game;
		DebugMode = false;
#endif
		InputBindings[static_cast<unsigned int>(InputType::Front)] = ImGuiKey_W;
		InputBindings[static_cast<unsigned int>(InputType::Back)] = ImGuiKey_S;
		InputBindings[static_cast<unsigned int>(InputType::Left)] = ImGuiKey_A;
		InputBindings[static_cast<unsigned int>(InputType::Right)] = ImGuiKey_D;
		InputBindings[static_cast<unsigned int>(InputType::Jump)] = ImGuiKey_Space;
		InputBindings[static_cast<unsigned int>(InputType::Crouch)] = ImGuiKey_LeftCtrl;
		InputBindings[static_cast<unsigned int>(InputType::Camera)] = ImGuiKey_RightCtrl;
		InputBindings[static_cast<unsigned int>(InputType::CameraFront)] = ImGuiKey_UpArrow;
		InputBindings[static_cast<unsigned int>(InputType::CameraBack)] = ImGuiKey_DownArrow;
		InputBindings[static_cast<unsigned int>(InputType::CameraLeft)] = ImGuiKey_LeftArrow;
		InputBindings[static_cast<unsigned int>(InputType::CameraRight)] = ImGuiKey_RightArrow;
		res = Resources::ResourceManager();
		Resources::Texture::SetFilterType(GL_NEAREST);
		res.SetPathAutoAppend(true);
		textures.CreateTexture(&res, "DefaultResources/Textures/debug.png", GL_NEAREST);
		textures.CreateTexture(&res, "DefaultResources/Textures/Blank.png");
		size_t index = textures.CreateTexture(&res, "DefaultResources/Textures/ScreenBuffer.png");
		ScreenBuffer = textures.GetTextures()[index];
		textures.CreateTexture(&res, "DefaultResources/Textures/graph.png");
		frameGraph = Core::Debug::FrameGraph(res.Get<Resources::Texture>("DefaultResources/Textures/graph.png"), Core::Maths::Int2D(240, 160));
		textures.CreateTexture(&res, "DefaultResources/Textures/normal.png");

		textures.CreateFont(&res, "DefaultResources/Font/default_font.png", GL_NEAREST);

		LowRenderer::Model* obj;
		for (size_t i = 0; i < sizeof(defaultModels) / sizeof(const char*); i++)
		{
			obj = new LowRenderer::Model();
			obj->LoadMesh(defaultModels[i], &res, &materials, &textures, &meshes);
			meshes.AddModel(obj, defaultModels[i]);
		}

		graph.AddComponent(new LowRenderer::Model());
		graph.AddComponent(new LowRenderer::SkinnedModel());
		graph.AddComponent(new LowRenderer::DirectionalLight());
		graph.AddComponent(new LowRenderer::PointLight());
		graph.AddComponent(new LowRenderer::SpotLight());
		graph.AddComponent(new LowRenderer::RotationController());
		graph.AddComponent(new LowRenderer::CameraConstraint());
		graph.AddComponent(new LowRenderer::Rendering::RenderCamera());
		graph.AddComponent(new LowRenderer::Rendering::ReflectionCamera());
		graph.AddComponent(new Physics::RigidBody());
		graph.AddComponent(new Physics::Colliders::BallCollider());
		graph.AddComponent(new Physics::Colliders::BoxCollider());
		graph.AddComponent(new Physics::Colliders::CapsuleCollider());
		graph.AddComponent(new Scripts::GravityArea());
		graph.AddComponent(new Scripts::SceneLoaderArea());
		graph.AddComponent(new Scripts::Entity::PlayerController());
		graph.AddComponent(new LowRenderer::GameUI::UIImage());
		graph.AddComponent(new LowRenderer::GameUI::UIText());
		graph.AddComponent(new LowRenderer::GameUI::UIButton());
		graph.AddComponent(new LowRenderer::GameUI::UIBindingButton());
		graph.AddComponent(new LowRenderer::GameUI::UISceneLoaderButton());
		graph.AddComponent(new LowRenderer::GameUI::UIExitButton());

		Physics::Colliders::Collider::SetPhysicHandler(&physics);

		lights.Init(&res);
		shaders = res.Create<Resources::ShaderManager>("ShaderManager");
		Resources::ResourceLoader::LoadResources(&res, &meshes, &materials, &textures, shaders);
		res.SetPathAutoAppend(true);

		Resources::FragmentShader* frag = res.Create<Resources::FragmentShader>("DefaultResources/Shaders/outline_fragment.frag");
		shaders->AddShader(frag);
		Resources::VertexShader* vert = res.Create<Resources::VertexShader>("DefaultResources/Shaders/outline_vertex.vert");
		shaders->AddShader(vert);
		shaders->CreateShaderProgram(vert, frag, "Outline Shader");
		frag = res.Create<Resources::FragmentShader>("DefaultResources/Shaders/ui_fragment.frag");
		shaders->AddShader(frag);
		vert = res.Create<Resources::VertexShader>("DefaultResources/Shaders/ui_vertex.vert");
		shaders->AddShader(vert);
		shaders->CreateShaderProgram(vert, frag, "UI Shader");
		frag = res.Create<Resources::FragmentShader>("DefaultResources/Shaders/ui_pixel_fragment.frag");
		shaders->AddShader(frag);
		shaders->CreateShaderProgram(vert, frag, "UI Pixel Shader");
		
		LoadScene(1);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void App::Update()
	{
		while (!glfwWindowShouldClose(window))
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			glfwPollEvents();
			ProcessInput();
			deltaTime = (float)glfwGetTime() - globalTime;
			globalTime = (float)glfwGetTime();

			fps = 1 / deltaTime;
			if ((int)(globalTime / 5) != (int)((globalTime + deltaTime) / 5))
			{
				minFps = fps;
				maxFps = fps;
			}
			else
			{
				if (minFps > fps)
					minFps = fps;
				if (maxFps < fps)
					maxFps = fps;
			}
			frameGraph.Update(deltaTime);
			//if (playMode != PlayType::Game)
				DrawGUI();

			if (deltaTime > MAX_LAG_DELTA)
			{
				LOG("Warning : Lagspike of %.3f ms, %.3f ms skipped", deltaTime * 1000.0, (deltaTime - MAX_LAG_DELTA) * 1000.0);
				deltaTime = (float)(MAX_LAG_DELTA);
			}

			MainCamera.Update(inputs, deltaTime);

			glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT);
			glClear(GL_DEPTH_BUFFER_BIT);
			if (playMode != PlayType::Game)
				graph.RenderGUI(&res, &physics, shaders, &materials, &textures, &meshes, &scene);

			scene.RenderScene(shaders, &lights, &textures, &res, VAO, &physics, &shaderProgram, ortho, (playMode != PlayType::Editor ? deltaTime : 0.0f), globalTime);

			if (ShouldClose) glfwSetWindowShouldClose(window, true);
			if (RequestedScene.c_str()[0])
			{
				SelectedComponent = 0;
				SelectedComponentAlt = 0;
				NextComponent = 0;
				scenePath = RequestedScene;
				RequestedScene.clear();
				LoadScene();
			}
			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}
	}

	void App::ClearApp()
	{
		scene.Destroy();
		graph.DeleteComponents();
		meshes.ClearModels();
		frameGraph.Destroy();
		shaders->DestroyShaderPrograms();
		res.ClearResources();

		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();

		glfwTerminate();
		delete[] windowIcon->pixels;
		delete windowIcon;
	}

	App::App()
	{
		window = nullptr;
		windowIcon = nullptr;
		VAO = 0xffffffff;
		shaderProgram = nullptr;
		shaders = nullptr;
	}

	// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
	// ---------------------------------------------------------------------------------------------------------
	void App::ProcessInput()
	{
		static Core::Maths::Int2D savedSize;
		static Core::Maths::Int2D savedPos;
		if (playMode == PlayType::Game && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			if (!requestEsc)
			{
				requestEsc = true;
				switch (State)
				{
				case Core::App::GameState::MainMenu:
					glfwSetWindowShouldClose(window, true);
					break;
				case Core::App::GameState::Option:
					RequestedScene = "Menu/MainMenu";
					State = GameState::MainMenu;
					break;
				case Core::App::GameState::Pause:
					break;
				case Core::App::GameState::Level:
					RequestedScene = "Menu/MainMenu";
					State = GameState::MainMenu;
					break;
				default:
					break;
				}
			}
		}
		else
		{
			requestEsc = false;
		}
		if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
		{
			if (!requestShaderReload)
			{
				shaders->ReloadShaders();
				shaderProgram = nullptr;
				requestShaderReload = true;
			}
		}
		else
		{
			requestShaderReload = false;
		}
		if (playMode == PlayType::Game && glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
		{
			if (!requestSave)
			{
				requestSave = true;
				scenePath = "Save/Save";
				SaveScene();
			}
		}
		else
		{
			requestSave = false;
		}
		if (playMode != PlayType::Game && glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS)
		{
			if (!requestPlayMode)
			{
				requestPlayMode = true;
				if (playMode == PlayType::Physic)
				{
					LoadScene(2);
					playMode = PlayType::Editor;
				}
				else
				{
					SaveScene(1);
					playMode = PlayType::Physic;
				}
			}
		}
		else
		{
			requestPlayMode = false;
		}
		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
		{
			if (!requestFullscreen)
			{
				requestFullscreen = true;
				if (fullscreen)
				{
					glfwSetWindowMonitor(window, NULL, savedPos.x, savedPos.y, savedSize.x, savedSize.y, GLFW_DONT_CARE);
				}
				else
				{
					glfwGetWindowPos(window, &savedPos.x, &savedPos.y);
						glfwGetWindowSize(window, &savedSize.x, &savedSize.y);
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					const GLFWvidmode* mode = glfwGetVideoMode(monitor);
					glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
					glfwSwapInterval(1);
				}
				fullscreen = !fullscreen;
			}
		}
		else
		{
			requestFullscreen = false;
		}
		inputs.UpdateInputs(window);
		if (writeScreen && inputs.ScreenSize.x*inputs.ScreenSize.y > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, ScreenBuffer->GetTextureID());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inputs.ScreenSize.x, inputs.ScreenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, inputs.ScreenSize.x, inputs.ScreenSize.y);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		{
			if (!requestScreenShot)
			{
				requestScreenShot = true;
				PrintScreen();
			}
		}
		else
		{
			requestScreenShot = false;
		}
		if (playMode != PlayType::Editor)
		{
			if (SelectedComponent == 0 && SelectedComponentAlt == 0) SelectedComponentAlt = 1;
			if (NextComponent != 0)
			{
				SelectedComponent = NextComponent;
				NextComponent = 0;
				SelectedComponentAlt = 0;
			}
			unsigned int MInputs = 0;
			for (unsigned int i = 0; i < static_cast<unsigned int>(InputType::Jump); i++)
			{
				MInputs |= ImGui::IsKeyDown(GetInputBindings()[i]) << i;
			}
			if (MInputs && SelectedComponent == 0)
			{
				SelectedComponent = SelectedComponentAlt;
				SelectedComponentAlt = 0;
			}
			if (inputs.deltaMouse.getLength() > 5 && SelectedComponentAlt == 0)
			{
				SelectedComponentAlt = SelectedComponent;
				SelectedComponent = 0;
			}
		}
		else if ((SelectedComponent + SelectedComponentAlt + NextComponent) != 0)
		{
			SelectedComponent = 0;
			SelectedComponentAlt = 0;
			NextComponent = 0;
		}
	}

	void App::DrawGUI()
	{
		if (ImGui::Begin("Config"))
		{
#ifndef GAME
			if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::InputText("Scene Name",&scenePath, ImGuiInputTextFlags_AutoSelectAll);
				if (ImGui::Button("Save Scene"))
				{
					SaveScene();
				}
				ImGui::SameLine();
				if (ImGui::Button("Load Scene"))
				{
					LoadScene();
				}
			}
			//FrameBuffer Overlay
			if (ImGui::CollapsingHeader("Framebuffer"))
			{
				ImGui::ColorEdit4("clearColor", &clearColor.x);
			}
			if (ImGui::CollapsingHeader("Physics"))
			{
				physics.RenderGUI();
			}
#endif
			//Camera Overlay
			if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
			{
#ifndef GAME
				MainCamera.RenderGUI();
#endif
				bool newVal = wireframe;
				ImGui::Checkbox("Wireframe render", &newVal);
				if (newVal != wireframe)
				{
					wireframe = newVal;
					if (wireframe)
					{
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						glDisable(GL_CULL_FACE);
					}
					else
					{
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						glEnable(GL_CULL_FACE);
					}
				}
				newVal = vsync;
				ImGui::Checkbox("VSync", &newVal);
				if (newVal != vsync)
				{
					vsync = newVal;
					if (vsync)
					{
						glfwSwapInterval(1);
					}
					else
					{
						glfwSwapInterval(0);
					}
				}
				ImGui::Checkbox("Debug View", &DebugMode);
				ImGui::Checkbox("Orthographic Projection", &ortho);
				ImGui::Checkbox("Write Screen Buffer to Texture", &writeScreen);
			}
			//Draw FPS
			ImGui::Text("FPS: %.1f  Min: %.1f  Max: %.1f", fps, minFps, maxFps);
			frameGraph.Print();
		}
		ImGui::End();
	}

	void App::PrintScreen()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		unsigned char* buf = new unsigned char[(size_t)(4llu * inputs.ScreenSize.x * inputs.ScreenSize.y)];
		glReadPixels(0, 0, inputs.ScreenSize.x, inputs.ScreenSize.y, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		ScreenBuffer->Overwrite(buf, inputs.ScreenSize.x, inputs.ScreenSize.y);
		Resources::Texture::SaveImage("Screenshots/Screenshot", buf, inputs.ScreenSize.x, inputs.ScreenSize.y);
		delete[] buf;
	}

	LowRenderer::Rendering::EditorCamera* Core::App::App::GetMainCamera()
	{
		return &MainCamera;
	}
	void App::LoadScene(int mode)
	{
		scene.Destroy();
		graph.EmptySelected();
		lights.EmptyLights();
		lights.UpdateShaders(&shaderProgram, shaders, MainCamera.position, nullptr, globalTime);
		textures.ClearShadowMaps(&res);
		std::string path;
		if (mode == 1)
		{
#ifndef GAME
			path = "DefaultResources/Scenes/Default.sdata";
#else
			path = "Scenes/Menu/MainMenu.sdata";
#endif
		}
		else if (mode == 2)
		{
			path = "DefaultResources/Scenes/PlayTmp.sdata";
		}
		else
		{
			path = "Scenes/";
			path.append(scenePath);
			path.append(".sdata");
		}
		if (Core::Util::SceneLoader::LoadScene(path.c_str(), &scene, &graph, &physics, &res, shaders))
		{
			scene.RefreshChilds();
		}
	}
	void App::SaveScene(int mode)
	{
		std::string path;
		if (mode == 1)
		{
			path = "DefaultResources/Scenes/PlayTmp.sdata";
		}
		else
		{
			path = "Scenes/";
			path.append(scenePath);
			path.append(".sdata");
		}
		Core::Util::SceneLoader::SaveScene(path.c_str(), &scene, &physics, &graph);
	}
	bool App::DoesSceneExist(const char* path)
	{
		std::string fullPath = "Scenes/";
		fullPath.append(path);
		fullPath.append(".sdata");
		return std::filesystem::exists(fullPath);
	}
}