#include "LowRenderer/GameUI/UISelectable.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

const char* const DirectionNames[4] =
{
	"Up Component ID",
	"Down Component ID",
	"Left Component ID",
	"Right Component ID",
};

LowRenderer::GameUI::UISelectable::UISelectable()
{
}

LowRenderer::GameUI::UISelectable::~UISelectable()
{
}

void LowRenderer::GameUI::UISelectable::DeleteComponent()
{
	UIText::DeleteComponent();
	this->~UISelectable();
}

void LowRenderer::GameUI::UISelectable::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	UIText::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "ID"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			ID = (unsigned int)Text::getInt(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "NavIDs"))
		{
			for (unsigned int i = 0; i < 4; i++)
			{
				pos = Text::skipCharSafe(data, pos, size);
				if (pos >= size)
				{
					err = "Malformated Int4";
					break;
				}
				NavIDs[i] = (unsigned int)Text::getInt(data, pos, size);
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UISelectable::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UIText::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "ID " << ID << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "NavIDs " << NavIDs[0] << " " << NavIDs[1] << " " << NavIDs[2] << " " << NavIDs[3] << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UISelectable::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	UIText::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::InputScalar("Component ID", ImGuiDataType_U32, &ID);
	if (ID == 0)
	{
		ImGui::SameLine();
		ImGui::Text("(Disabled)");
	}
	for (unsigned int i = 0; i < 4; i++)
	{
		ImGui::InputScalar(DirectionNames[i], ImGuiDataType_U32, &NavIDs[i]);
		if (NavIDs[i] == 0)
		{
			ImGui::SameLine();
			ImGui::Text("(Disabled)");
		}
	}
}

void LowRenderer::GameUI::UISelectable::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs)
{
	UIText::RenderGameUI(container, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, Inputs);
	if (!Enabled || !ID) return;
	if (ID == Core::App::App::GetSelectedComponent())
	{
		Hovered = true;
		if (Inputs & 0x100000)
		{
			Clicked = true;
			OnClick();
		}
		if (Inputs & 0xF0000)
		{
			for (unsigned int i = 0; i < 4; i++)
			{
				if (NavIDs[i] && (Inputs & 1 << (i + 16)))
				{
					Core::App::App::SetSelectedComponent(NavIDs[i]);
				}
			}
		}
	}
}
