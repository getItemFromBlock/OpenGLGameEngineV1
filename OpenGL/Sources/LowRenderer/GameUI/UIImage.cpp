#include "LowRenderer/GameUI/UIImage.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_stdlib.hpp>
#include <glad/glad.h>

#include "Core/Debug/Log.hpp"

#include "Core/Util/TextHelper.hpp"
#include "Core/DataStructure/Node.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ResourceManager.hpp"
#include "Core/App/App.hpp"

LowRenderer::GameUI::UIImage::UIImage()
{
}

LowRenderer::GameUI::UIImage::~UIImage()
{
}

void LowRenderer::GameUI::UIImage::DeleteComponent()
{
	Mat.UnLoad();
	UIElement::~UIElement();
	this->~UIImage();
}

void LowRenderer::GameUI::UIImage::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new UIImage());
}

void LowRenderer::GameUI::UIImage::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "BaseColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, BaseColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "PixelPerfect"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, IsPixelPerfect))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "Image"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string path = Text::getText(data, pos, size);
			if (!path.c_str())
			{
				err = "Expected path after \"Image\"";
				break;
			}
			Resources::Texture* texture = resources->Get<Resources::Texture>(path.c_str());
			if (!texture)
			{
				tmpTexturePath = path;
				texture = resources->Get<Resources::Texture>("DefaultResources/Textures/Blank.png");
			}
			Mat.SetTexture(texture);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UIImage::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UIElement::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "BaseColor " << BaseColor.x << " " << BaseColor.y << " " << BaseColor.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "PixelPerfect " << (IsPixelPerfect ? "True" : "False") << std::endl;
	if (Mat.GetTexture())
	{
		Parsing::Shift(fileOut, rec);
		fileOut << "Image " << Mat.GetTexture()->GetPath() << std::endl;
	}
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UIImage::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	UIElement::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::ColorEdit3("Base Color", &BaseColor.x);
	ImGui::Checkbox("Pixel Perfect", &IsPixelPerfect);
	if (Mat.GetTexture())
	{
		ImGui::Image((ImTextureID)(size_t)Mat.GetTexture()->GetTextureID(), ImVec2(32, 32));
		ImGui::SameLine();
		ImGui::Text(Mat.GetTexture()->GetName());
	}
	else
	{
		ImGui::Image((ImTextureID)1llu, ImVec2(32, 32));
		ImGui::SameLine();
		ImGui::Text("Texture");
	}
	ImGui::SameLine();
	ImGui::Button("Change Texture");
	if (ImGui::IsItemClicked())
	{
		Mat.SetSearchData(Mat.GetTexture());
		ImGui::OpenPopup("Texture Selection");
		ImGui::SetNextWindowSize(ImVec2(700, 400));
	}
	if (ImGui::BeginPopupModal("Texture Selection")) Mat.TexturePopUp(textureManager, false);
}

void LowRenderer::GameUI::UIImage::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs)
{
	UIElement::RenderGameUI(container, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, MouseInputs);
	if (!Enabled || !Plane) return;
	Core::Maths::Mat4D mv = v * ElementMat;
	bool switched = false;
	if (ImageShader && IsPixelPerfect)
	{
		switched = true;
		glUseProgram(ImageShader->GetProgramID());
	}
	if (Mat.GetTexture()) glUniform1i((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::Texture), Mat.GetTexture()->GetTextureID());
	else glUniform1i((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::Texture), Core::App::DefaultTextures::Blank);
	glUniformMatrix4fv((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mv.content);
	glUniform3f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAmbient), BaseColor.x, BaseColor.y, BaseColor.z);
	glUniform1f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAlpha), Alpha);
	Plane->Draw(VAOCurrent);
	if (switched)
	{
		glUseProgram((*shaderProgramCurrent)->GetProgramID());
	}
}

void LowRenderer::GameUI::UIImage::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (!Plane) Plane = resources->Get<Resources::Mesh>("DebugPlane");
	if (!ImageShader) ImageShader = resources->Get<Resources::ShaderManager>("ShaderManager")->GetShaderProgram("ui pixel shader");
	if (tmpTexturePath.c_str()[0])
	{
		DeltaF++;
		Resources::Texture* tmp = resources->Get<Resources::Texture>(tmpTexturePath.c_str());
		if (tmp)
		{
			Mat.SetTexture(tmp);
			tmpTexturePath = "";
		}
		else
		{
			if (DeltaF > 8)
			{
				LOG("Warning, could not find texture %s", tmpTexturePath.c_str());
				tmpTexturePath = "";
			}
		}
	}
}

void LowRenderer::GameUI::UIImage::OnClick()
{
}
