#pragma once

#include "LowRenderer/GameUI/UIButton.hpp"

namespace LowRenderer::GameUI
{
	class UIExitButton : public UIButton
	{
	public:
		UIExitButton();
		~UIExitButton();

		virtual const char* GetName() override { return "UI Exit Button"; }
		virtual void DeleteComponent() override;
		virtual void Create(Core::DataStructure::Node* container) override;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		virtual void OnClick() override;
	protected:
	};

}