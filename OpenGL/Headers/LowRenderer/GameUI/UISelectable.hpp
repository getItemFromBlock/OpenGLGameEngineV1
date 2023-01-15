#pragma once

#pragma once

#include <string>

#include "LowRenderer/GameUI/UIText.hpp"

namespace LowRenderer::GameUI
{
	class UISelectable : public UIText
	{
	public:
		UISelectable();
		~UISelectable();

		virtual void DeleteComponent() override;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs) override;
	protected:
		unsigned int ID = 0;
		unsigned int NavIDs[4] = { 0 };
	};

}