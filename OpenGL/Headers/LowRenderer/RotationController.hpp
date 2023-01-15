#pragma once

#include "LowRenderer/Component.hpp"
#include "Core/Maths/Maths.hpp"

namespace LowRenderer
{
	class RotationController : public Component
	{
	public:
		RotationController();
		~RotationController();

		void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime) override;
		void Create(Core::DataStructure::Node* container) override;
		const char* GetName() override { return "Rotation Controller"; }
		void DeleteComponent() override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;

		Core::Maths::Vec3D RotationDelta;
	private:
	};
}