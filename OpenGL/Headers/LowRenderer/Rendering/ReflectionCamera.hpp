#pragma once

#include "LowRenderer/Rendering/RenderCamera.hpp"

namespace LowRenderer::Rendering
{
	class ReflectionCamera : public RenderCamera
	{
	public:
		friend class Core::DataStructure::SceneNode;
		ReflectionCamera();
		~ReflectionCamera();

		void Update(Core::DataStructure::Node* container, std::vector<RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime) override;
		void Create(Core::DataStructure::Node* container) override;
		const char* GetName() override { return "Reflection Camera"; }
		void DeleteComponent() override;
		void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;

	protected:
		Core::Maths::Vec3D PlaneNormal;
		Core::Maths::Vec3D PlaneRight;
		Core::Maths::Vec3D PlaneUp;
	};
}