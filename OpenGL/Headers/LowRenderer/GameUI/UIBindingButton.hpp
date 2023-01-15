#pragma once

#include "LowRenderer/GameUI/UIButton.hpp"

namespace Core::App
{
	enum class InputType : unsigned int;
}

namespace LowRenderer::GameUI
{
	class UIBindingButton : public UIButton
	{
	public:
		UIBindingButton();
		~UIBindingButton();

		virtual const char* GetName() override { return "UI Binding Button"; }
		virtual void DeleteComponent() override;
		virtual void Create(Core::DataStructure::Node* container) override;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs) override;
		virtual void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) override;
		virtual void OnClick() override;
	private:
		void UpdateKeyText();

		Core::App::InputType Type = static_cast<Core::App::InputType>(0);
		float Delay = 3.0f;
		float Counter = -1.0f;
		Core::Maths::Vec3D WaitColor = Core::Maths::Vec3D(0.4f, 0.4f, 1.0f);
		Core::Maths::Vec3D TmpColor[3] = {0};
	};

}
