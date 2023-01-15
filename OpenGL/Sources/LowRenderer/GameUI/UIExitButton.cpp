#include "LowRenderer/GameUI/UIExitButton.hpp"

#include "Core/App/App.hpp"
#include "Core/Util/TextHelper.hpp"

LowRenderer::GameUI::UIExitButton::UIExitButton()
{
}

LowRenderer::GameUI::UIExitButton::~UIExitButton()
{
}

void LowRenderer::GameUI::UIExitButton::DeleteComponent()
{
	UIButton::DeleteComponent();
	this->~UIExitButton();
}

void LowRenderer::GameUI::UIExitButton::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new UIExitButton());
}

void LowRenderer::GameUI::UIExitButton::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	UIButton::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UIExitButton::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UIButton::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UIExitButton::OnClick()
{
	if (Core::App::App::GetPlayType() == Core::App::PlayType::Editor) return;
	Core::App::App::RequestExit();
}
