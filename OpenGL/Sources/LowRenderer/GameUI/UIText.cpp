#include "LowRenderer/GameUI/UIText.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_stdlib.hpp>
#include <glad/glad.h>

#include "Core/Debug/Log.hpp"

#include "Core/Util/TextHelper.hpp"
#include "Core/DataStructure/Node.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ResourceManager.hpp"
#include "Core/App/App.hpp"
#include "Resources/Font.hpp"

LowRenderer::GameUI::UIText::UIText()
{
}

LowRenderer::GameUI::UIText::~UIText()
{
}

void LowRenderer::GameUI::UIText::DeleteComponent()
{
	UIElement::~UIElement();
	this->~UIText();
}

void LowRenderer::GameUI::UIText::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new UIText());
}

void LowRenderer::GameUI::UIText::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	UIElement::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Label"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string txt = Text::getText(data, pos, size);
			if (!txt.c_str()[0])
			{
				err = "Expected name after \"Label\"";
				break;
			}
			Label = Text::DeSerialize(txt);
		}
		else if (Text::compareWord(data, pos, size, "TextSize"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			TextSize = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "TextColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, TextColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UIText::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UIElement::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Label " << Text::Serialize(Label) << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "TextSize " << TextSize << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "TextColor " << TextColor.x << " " << TextColor.y << " " << TextColor.z << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UIText::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	UIElement::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	size_t Size = Label.size();
	ImGui::InputTextMultiline("Label", &Label);
	if (Label.size() != Size) UpdateText();
	ImGui::DragFloat("Text Size", &TextSize, 0.1f);
	ImGui::ColorEdit3("Text Color", &TextColor.x);
}

std::string GetLine(std::string& in, size_t& index)
{
	std::string out;
	while (index < in.size())
	{
		char c = in.c_str()[index];
		out += c;
		if (c == '0' || c == '\n')
		{
			if (c == '\n') out.pop_back();
			break;
		}
		index++;
	}
	return out;
}

void LowRenderer::GameUI::UIText::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs)
{
	UIElement::RenderGameUI(container, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, MouseInputs);
	if (!Enabled || !Plane || !MFont) return;
	if (Label.c_str()[0] && lines.size() > 0)
	{
		TextMat = Core::Maths::Mat4D::CreateTransformMatrix(ElementMat.GetPositionFromTranslation(), 0, 0.01f * TextSize);
		Core::Maths::Mat4D mv2 = v * TextMat;
		glUniform1i((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::Texture), Core::App::DefaultTextures::Default_Font);
		glUniformMatrix4fv((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mv2.content);
		glUniform3f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAmbient), TextColor.x, TextColor.y, TextColor.z);
		glUniform1f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAlpha), -1);
		glDisable(GL_BLEND);
		Core::Maths::Vec2D pos;
		for (size_t n = 0; n < lines.size(); n++)
		{
			if (!lines[n].line.c_str()[0]) continue;
			pos = Core::Maths::Vec2D(-(((1.0f - 1.0f / lines[n].line.size()) * lines[n].TotalLength - 1 / 100.0f) * 0.5f * TextSize * ScreenRes.y / ScreenRes.x), (lines.size()-1) / 200.0f * TextSize * 2.2f - (n / 100.0f * TextSize * 2.2f));
			for (size_t i = 0; i < lines[n].line.size(); i++)
			{
				unsigned char c = lines[n].line.data()[i];
				if (c == '\n')
				{
					break;
				}
				glUniform2f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::DeltaPos), pos.x, pos.y);
				glUniform2f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::DeltaUV), (c % 16) / 16.0f, (c / 16) / 16.0f);
				Plane->Draw(VAOCurrent);
				pos.x += ((1 / 100.0f * 2.0f * MFont->GetSpacing(c)) + 1 / 400.0f) * TextSize * ScreenRes.y / ScreenRes.x;
			}
		}
		glEnable(GL_BLEND);
	}
	
}

void LowRenderer::GameUI::UIText::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (!Plane) Plane = resources->Get<Resources::Mesh>("DebugPlane");
	if (!MFont)
	{
		MFont = resources->Get<Resources::Font>("DefaultResources/Font/default_font.png");
		UpdateText();
	}
}

void LowRenderer::GameUI::UIText::OnClick()
{
}

void LowRenderer::GameUI::UIText::UpdateText()
{
	lines.clear();
	if (!MFont) return;
	size_t index = 0;
	while (index < Label.size())
	{
		LineData data;
		data.line = GetLine(Label, index);
		data.TotalLength = 0.0f;
		for (size_t i = 0; i < data.line.size(); i++) data.TotalLength += ((1 / 100.0f * 2.0f * MFont->GetSpacing(data.line.c_str()[i])) + 1 / 400.0f);
		index++;
		lines.push_back(data);
	}
}
