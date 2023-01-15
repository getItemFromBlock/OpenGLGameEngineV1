#pragma once

#include "LowRenderer/GameUI/UIElement.hpp"
#include "Resources/Material.hpp"

namespace Resources
{
	class Mesh;
}

namespace LowRenderer::GameUI
{
	class UIImage : public UIElement
	{
	public:
		UIImage();
		~UIImage();

		virtual const char* GetName() override { return "UI Image"; }
		virtual void DeleteComponent() override;
		virtual void Create(Core::DataStructure::Node* container) override;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs) override;
		virtual void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) override;
		virtual void OnClick() override;
	private:
		Core::Maths::Vec3D BaseColor = Core::Maths::Vec3D(0.6f);
		Resources::Mesh* Plane = nullptr;
		Resources::ShaderProgram* ImageShader = nullptr;
		Resources::Material Mat;
		std::string tmpTexturePath = "";
		float DeltaF = 0.0f;
		bool IsPixelPerfect = false;
	};

}