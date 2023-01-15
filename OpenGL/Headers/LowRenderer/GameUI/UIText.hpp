#pragma once

#include <string>

#include "LowRenderer/GameUI/UIElement.hpp"

namespace Resources
{
	class Mesh;
	class Font;
}

namespace LowRenderer::GameUI
{
	struct LineData
	{
		float TotalLength;
		std::string line;
	};

	class UIText : public UIElement
	{
	public:
		UIText();
		~UIText();

		virtual const char* GetName() override { return "UI Text"; }
		virtual void DeleteComponent() override;
		virtual void Create(Core::DataStructure::Node* container) override;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs) override;
		virtual void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) override;
		virtual void OnClick() override;
	protected:
		std::string Label = "Text";
		Resources::Mesh* Plane = nullptr;
		Resources::Font* MFont = nullptr;
		float TextSize = 4.0f;
		Core::Maths::Vec3D TextColor = Core::Maths::Vec3D(0.0f, 0.0f, 0.0f);
		Core::Maths::Mat4D TextMat = Core::Maths::Mat4D::Identity();
		std::vector<LineData> lines;

		void UpdateText();
	};

}