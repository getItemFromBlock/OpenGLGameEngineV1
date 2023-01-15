#include "Scripts/SceneLoaderArea.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_stdlib.hpp>

#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

#include "Core/DataStructure/Node.hpp"
#include "Physics/Colliders/Collider.hpp"
#include "Physics/RigidBody.hpp"
#include "Scripts/Entity/EntityBase.hpp"

const char* const SceneTypeDesc[] =
{
	"Main Menu",
	"Option Menu",
	"Pause Menu",
	"Level",
};

Scripts::SceneLoaderArea::SceneLoaderArea()
{
	SceneType = Core::App::GameState::MainMenu;
}

Scripts::SceneLoaderArea::~SceneLoaderArea()
{
}

void Scripts::SceneLoaderArea::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new SceneLoaderArea());
}

void Scripts::SceneLoaderArea::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (!parent) parent = container;
	if (Core::App::App::GetPlayType() != Core::App::PlayType::Editor && SceneToLoad.c_str()[0] && CheckExist && !Unactive && !Core::App::App::DoesSceneExist(SceneToLoad.c_str()))
	{
		Unactive = true;
	}
}

void Scripts::SceneLoaderArea::DeleteComponent()
{
	this->~SceneLoaderArea();
}

void Scripts::SceneLoaderArea::OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other)
{
	if (!Enabled || !parent || !other->AttachedBody) return;
	if (Core::App::App::GetPlayType() == Core::App::PlayType::Editor || Unactive || !SceneToLoad.c_str()[0]) return;
	if (other->AttachedBody && other->AttachedBody->GetParent())
	{
		for (size_t i = 0; i < other->AttachedBody->GetParent()->components.size(); i++)
		{
			Scripts::Entity::EntityBase* n = dynamic_cast<Scripts::Entity::EntityBase*>(other->AttachedBody->GetParent()->components[i]);
			if (n)
			{
				Core::App::App::RequestSceneLoad(SceneToLoad.c_str());
				Core::App::App::SetGameState(SceneType);
			}
		}
	}
}

void Scripts::SceneLoaderArea::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Component::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent"))
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

void Scripts::SceneLoaderArea::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "SceneToLoad " << SceneToLoad << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "CheckExist " << (CheckExist ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "SceneType " << static_cast<unsigned int>(SceneType) << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void Scripts::SceneLoaderArea::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::InputTextWithHint("Scene To Load", "Relative Path (ex: \"Menu/MainMenu\")", &SceneToLoad);
	ImGui::Checkbox("Check If Scene Exists", &CheckExist);
	ImGui::Combo("Scene Type", (int*)&SceneType, SceneTypeDesc, 4, -1);
}
