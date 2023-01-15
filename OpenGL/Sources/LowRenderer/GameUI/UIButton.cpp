#include "LowRenderer/GameUI/UIButton.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_stdlib.hpp>
#include <glad/glad.h>

#include "Core/Debug/Log.hpp"

#include "Core/Util/TextHelper.hpp"
#include "Core/DataStructure/Node.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ResourceManager.hpp"
#include "Core/App/App.hpp"

LowRenderer::GameUI::UIButton::UIButton()
{
}

LowRenderer::GameUI::UIButton::~UIButton()
{
}

void LowRenderer::GameUI::UIButton::DeleteComponent()
{
	UISelectable::DeleteComponent();
	this->~UIButton();
}

void LowRenderer::GameUI::UIButton::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new UIButton());
}

void LowRenderer::GameUI::UIButton::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	UISelectable::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "BaseColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, BaseColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "HoverColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, HoverColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "ClickColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, ClickColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UIButton::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UISelectable::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "BaseColor " << BaseColor.x << " " << BaseColor.y << " " << BaseColor.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "HoverColor " << HoverColor.x << " " << HoverColor.y << " " << HoverColor.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "ClickColor " << ClickColor.x << " " << ClickColor.y << " " << ClickColor.z << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UIButton::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	UISelectable::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::ColorEdit3("Base Color", &BaseColor.x);
	ImGui::ColorEdit3("Hover Color", &HoverColor.x);
	ImGui::ColorEdit3("Click Color", &ClickColor.x);
}

void LowRenderer::GameUI::UIButton::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs)
{
	if (!Enabled || !Plane) return;
	Core::Maths::Mat4D mv = v * ElementMat;
	glUniform1i((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::Texture), Core::App::DefaultTextures::Blank);
	glUniformMatrix4fv((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mv.content);
	if (Clicked) glUniform3f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAmbient), ClickColor.x, ClickColor.y, ClickColor.z);
	else if (Hovered) glUniform3f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAmbient), HoverColor.x, HoverColor.y, HoverColor.z);
	else glUniform3f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAmbient), BaseColor.x, BaseColor.y, BaseColor.z);
	glUniform1f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAlpha), Alpha);
	Plane->Draw(VAOCurrent);
	UISelectable::RenderGameUI(container, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, MouseInputs);
}

void LowRenderer::GameUI::UIButton::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	UISelectable::Update(container, cameras, resources, textureManager, lightManager, DeltaTime);
}

void LowRenderer::GameUI::UIButton::OnClick()
{
}
