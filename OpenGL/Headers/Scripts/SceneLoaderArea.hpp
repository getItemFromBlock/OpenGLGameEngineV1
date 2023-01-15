#pragma once

#include "LowRenderer/Component.hpp"
#include "Core/Maths/Maths.hpp"

namespace Resources
{
	class Mesh;
}

namespace Core::App
{
	enum class GameState : unsigned int;
}

namespace Scripts
{
	class SceneLoaderArea : public LowRenderer::Component
	{
	public:
		SceneLoaderArea();
		~SceneLoaderArea();

		void Create(Core::DataStructure::Node* container) override;
		void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) override;
		const char* GetName() override { return "Scene Loading Area"; }
		void DeleteComponent() override;
		void OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;

	private:
		std::string SceneToLoad;
		bool CheckExist = true;
		bool Unactive = false;
		Core::App::GameState SceneType;
		Core::DataStructure::Node* parent = nullptr;
	};
}