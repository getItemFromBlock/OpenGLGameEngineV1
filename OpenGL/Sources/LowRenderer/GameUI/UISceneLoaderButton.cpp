#include "LowRenderer/GameUI/UISceneLoaderButton.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_stdlib.hpp>

#include "Core/DataStructure/Node.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

const char* const SceneTypeDesc[] =
{
	"Main Menu",
	"Option Menu",
	"Pause Menu",
	"Level",
};

LowRenderer::GameUI::UISceneLoaderButton::UISceneLoaderButton()
{
	SceneType = Core::App::GameState::MainMenu;
}

LowRenderer::GameUI::UISceneLoaderButton::~UISceneLoaderButton()
{
}

void LowRenderer::GameUI::UISceneLoaderButton::DeleteComponent()
{
	UIButton::DeleteComponent();
	this->~UISceneLoaderButton();
}

void LowRenderer::GameUI::UISceneLoaderButton::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new UISceneLoaderButton());
}

void LowRenderer::GameUI::UISceneLoaderButton::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "SceneToLoad"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string path = Text::getText(data, pos, size);
			if (!path.c_str())
			{
				err = "Expected path after \"SceneToLoad\"";
				break;
			}
			SceneToLoad = path;
		}
		else if (Text::compareWord(data, pos, size, "CheckExist"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, CheckExist))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "UnavailableColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, UnavailableColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "SceneType"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode > static_cast<unsigned char>(Core::App::GameState::Level))
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"SceneType\" : valid numbers are [0-3]", mode);
				err = buff;
				break;
			}
			SceneType = static_cast<Core::App::GameState>(mode);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UISceneLoaderButton::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UIButton::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "SceneToLoad " << SceneToLoad << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "CheckExist " << (CheckExist ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "UnavailableColor " << UnavailableColor.x << " " << UnavailableColor.y << " " << UnavailableColor.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "SceneType " << static_cast<unsigned int>(SceneType) << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UISceneLoaderButton::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	UIButton::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::InputTextWithHint("Scene To Load", "Relative Path (ex: \"Menu/MainMenu\")", &SceneToLoad);
	ImGui::Checkbox("Check If Scene Exists", &CheckExist);
	ImGui::ColorEdit3("Unavailable Color", &UnavailableColor.x);
	ImGui::Combo("Scene Type", (int*)&SceneType, SceneTypeDesc, 4, -1);
}

void LowRenderer::GameUI::UISceneLoaderButton::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs)
{
	if (!Enabled || !Plane) return;
	UIButton::RenderGameUI(container, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, Inputs);
}

void LowRenderer::GameUI::UISceneLoaderButton::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (Core::App::App::GetPlayType() != Core::App::PlayType::Editor && SceneToLoad.c_str()[0] && CheckExist && !Unactive && !Core::App::App::DoesSceneExist(SceneToLoad.c_str()))
	{
		BaseColor = UnavailableColor;
		HoverColor = UnavailableColor * 1.3f;
		ClickColor = UnavailableColor * 1.3f;
		Unactive = true;
	}
	UIButton::Update(container, cameras, resources, textureManager, lightManager, DeltaTime);
}

void LowRenderer::GameUI::UISceneLoaderButton::OnClick()
{
	if (Core::App::App::GetPlayType() == Core::App::PlayType::Editor || Unactive || !SceneToLoad.c_str()[0]) return;
	Core::App::App::RequestSceneLoad(SceneToLoad.c_str());
	Core::App::App::SetGameState(SceneType);
}
