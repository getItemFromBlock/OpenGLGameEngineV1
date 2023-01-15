#pragma once

#include "LowRenderer/Rendering/Camera.hpp"
#include "LowRenderer/Component.hpp"
#include "LowRenderer/Rendering/FrameBuffer.hpp"

namespace Core::DataStructure
{
	class SceneNode;
}

namespace LowRenderer::Rendering
{
	class RenderCamera : public Component, public Camera
	{
	public:
		friend class Core::DataStructure::SceneNode;
		RenderCamera();
		~RenderCamera();

		virtual void Update(Core::DataStructure::Node* container, std::vector<RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime) override;
		virtual void Create(Core::DataStructure::Node* container) override;
		void DeleteComponent() override;
		const char* GetName() override { return "Render Camera"; }
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		void RefreshResolution() { Resolution = tmpResolution; }

		Core::Maths::Int2D tmpResolution = Core::Maths::Int2D(256, 256);
		bool ClearBuffer = true;
		Core::Maths::Vec3D ClearColor = Core::Maths::Vec3D();
	protected:
		char UUID[17] = "";
		LowRenderer::Rendering::FrameBuffer* buffer = nullptr;
		TextureFilterType Filter = TextureFilterType::Linear;
		TextureWrapType Wrap = TextureWrapType::ClampToEdge;
	};
}