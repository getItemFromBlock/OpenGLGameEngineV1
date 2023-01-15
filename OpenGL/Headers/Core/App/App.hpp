#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#include "LowRenderer/Rendering/EditorCamera.hpp"
#include "Resources/ResourceManager.hpp"
#include "Core/App/Input.hpp"
#include "Resources/ShaderProgram.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Core/Debug/FrameGraph.hpp"
#include "Core/DataStructure/Node.hpp"
#include "LowRenderer/Lightning/LightManager.hpp"
#include "Core/DataStructure/Graph.hpp"
#include "Physics/PhysicsHandler.hpp"

namespace Core::App
{
	enum class InputType : unsigned int
	{
		Front = 0,
		Back,
		Left,
		Right,
		Jump,
		Crouch,
		Camera,
		CameraFront,
		CameraBack,
		CameraRight,
		CameraLeft,
		All,
	};

	enum class GameState : unsigned int
	{
		MainMenu = 0,
		Option,
		Pause,
		Level,
	};

	struct AppInit
	{
		const unsigned int width;
		const unsigned int height;
		const unsigned int majorVersion;
		const unsigned int minorVersion;
		const char* name;
		void (*glDebugOutput)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	};

	enum class PlayType : unsigned char
	{
		Editor = 0,
		Physic = 1,
		Game = 2,
	};

	enum DefaultTextures
	{
		Debug = 1,
		Blank,
		Screen_Buffer,
		Graph,
		Blank_Normal,
		Default_Font,
	};

	class App
	{
	private:
		GLFWwindow* window;
		GLFWimage* windowIcon;
		static inline Inputs inputs;
		static inline LowRenderer::Rendering::EditorCamera MainCamera = LowRenderer::Rendering::EditorCamera();
		static inline bool DebugMode = true;
		static inline bool wireframe = false;
		static inline PlayType playMode = PlayType::Editor;
		static inline int InputBindings[static_cast<unsigned int>(InputType::All)] = { 0 };
		static inline unsigned int SelectedComponent = 0;
		static inline unsigned int SelectedComponentAlt = 0;
		static inline unsigned int NextComponent = 0;
		static inline std::string RequestedScene;
		static inline bool ShouldClose = false;
		static inline GameState State = GameState::MainMenu;
		LowRenderer::Lightning::LightManager lights;
		Resources::ShaderManager* shaders;
		Resources::TextureManager textures;
		Resources::MaterialManager materials;
		Resources::MeshManager meshes;
		Core::Debug::FrameGraph frameGraph;
		Core::DataStructure::Graph graph;
		Core::DataStructure::SceneNode scene;
		Resources::ResourceManager res;
		Physics::PhysicsHandler physics;
		
		bool ortho = false;
		bool vsync = true;
		bool writeScreen = false;
		bool fullscreen = false;
		bool requestFullscreen = false;
		bool requestScreenShot = false;
		bool requestShaderReload = false;
		bool requestPlayMode = false;
		bool requestEsc = false;
		bool requestSave = false;
		Resources::Texture* ScreenBuffer = nullptr;

		unsigned int VAO;
		Resources::ShaderProgram* shaderProgram;
		float globalTime = 0;
		float deltaTime = 0;
		float fps = 60;
		float minFps = 60;
		float maxFps = 60;
		Core::Maths::Vec4D clearColor = Core::Maths::Vec4D(0.0f, 0.125f, 0.443f, 1.0f);
		std::string scenePath = "Scene";

		int InitOpenGL(const AppInit& app);

		void InitDatas();

		void ProcessInput();

		void DrawGUI();
		
	public:
		App();

		int InitApp(const AppInit& data);

		void ClearApp();

		void Update();

		void PrintScreen();

		static LowRenderer::Rendering::EditorCamera* GetMainCamera();

		static bool IsDebugMode() { return DebugMode; }

		static bool IsWireframe() { return wireframe; }

		void LoadScene(int mode = 0);

		void SaveScene(int mode = 0);

		static inline PlayType GetPlayType() { return playMode; }

		static inline int* GetInputBindings() { return InputBindings; }

		static inline unsigned int GetSelectedComponent() { return SelectedComponent; }

		static inline void SetSelectedComponent(unsigned int in) { NextComponent = in; }

		static inline int GetLastKeyPressed() { return inputs.GetLastKey(); }

		static inline void ClearLastKeyPressed() { inputs.ClearLastKey(); }

		static inline void RequestSceneLoad(const char* path) { RequestedScene = path; }

		static inline void RequestExit() { ShouldClose = true; }

		static bool DoesSceneExist(const char* path);

		static inline void SetGameState(GameState in) { State = in; }

	};

}