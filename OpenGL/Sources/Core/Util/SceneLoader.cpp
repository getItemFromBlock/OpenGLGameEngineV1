#include "Core/Util/SceneLoader.hpp"

#include "Core/DataStructure/Node.hpp"
#include "Core/Util/ModelLoader.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Core/DataStructure/Graph.hpp"
#include "LowRenderer/Component.hpp"
#include "Physics/PhysicsHandler.hpp"

#include <vector>
#include <iomanip>

bool Core::Util::SceneLoader::SaveScene(const char* path, const Core::DataStructure::SceneNode* scene, Physics::PhysicsHandler* handler, Core::DataStructure::Graph* provider)
{
	std::ofstream save;
	save.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	char errCode[128];
	if (!save.is_open() || save.fail())
	{
		LOG("Error, cannot save to file %s : %s", path, strerror_s(errCode, 128, errno));
		return false;
	}
	save << std::fixed << std::setprecision(6);
	save << "Version 2.0" << std::endl;
	save << "SceneName " << scene->Name << std::endl;
	handler->Serialize(save);
	scene->Serialize(save, 1);
	save << "EndScene" << std::endl << std::endl;
	save.close();
	LOG("Scene saved as %s", path);
	return true;
}

bool Core::Util::SceneLoader::LoadScene(const char* path, Core::DataStructure::Node* scene, Core::DataStructure::Graph* provider, Physics::PhysicsHandler* handler, Resources::ResourceManager* resources, Resources::ShaderManager* shaders)
{
	int64_t size = 0;
	const char* data = ModelLoader::loadFile(path, size);
	if (!path || !size) return false;
	int64_t pos = 0;
	int64_t line = 1;
	std::string err = "";
	Core::DataStructure::Node* current = scene;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "SceneName"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string name = Text::getText(data, pos, size);
			if (!name.c_str())
			{
				err = "Expected name after \"SceneName\"";
				break;
			}
			name.copy(scene->Name, 64, 0);
			current->Name[63] = 0;
		}
		else if (Text::compareWord(data, pos, size, "Version"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string ver = Text::getText(data, pos, size);
			if (!ver.c_str())
			{
				err = "Expected Version number after \"Version\"";
				break;
			}
			LOG("File Version : %s", ver.c_str());
		}
		else if (Text::compareWord(data, pos, size, "CollisionLayers"))
		{
			pos = Text::endLine(data, pos, size);
			line++;
			handler->Deserialize(data, pos, size, line, err);
			if (err.c_str()[0]) break;
		}
		else if (Text::compareWord(data, pos, size, "EndScene"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "NewNode"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string name = Text::getText(data, pos, size);
			if (!name.c_str())
			{
				err = "Expected name after \"NewNode\"";
				break;
			}
			current->childs.push_back(new Core::DataStructure::Node(current, name.c_str()));
			current = current->childs[current->childs.size() - 1llu];
		}
		else if (Text::compareWord(data, pos, size, "EndNode"))
		{
			if (!current->GetParent())
			{
				err = "Unexpected statement \"EndNode\"";
				break;
			}
			current = current->GetParent();
		}
		else if (Text::compareWord(data, pos, size, "NewComponent"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string type = Text::getText(data, pos, size);
			if (!type.c_str())
			{
				err = "Expected type after \"NewComponent\"";
				break;
			}
			if (!CreateComponentType(type, provider, current))
			{
				LOG("Warning : Not a Component type at line %llu", line);
			}
			pos = Text::endLine(data, pos, size);
			line++;
			current->components[current->components.size() - 1llu]->Deserialize(resources, shaders, data, pos, size, line, err);
			if (err.c_str()[0]) break;
		}
		else if (Text::compareWord(data, pos, size, "EndComponent"))
		{
			//err = "Unexpected statement \"EndComponent\"";
			//break;
		}
		else if (Text::compareWord(data, pos, size, "Position"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			current->SetLocalPosition(Core::Maths::Vec3D(a,b,c));
		}
		else if (Text::compareWord(data, pos, size, "Rotation"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			current->SetLocalRotation(Core::Maths::Vec3D(a, b, c));
		}
		else if (Text::compareWord(data, pos, size, "Scale"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			current->SetLocalScale(Core::Maths::Vec3D(a, b, c));
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
	delete[] data;
	if (err.c_str()[0])
	{
		LOG("Error : %s at line %llu", err.c_str(), line);
	}
	return true;
}

bool Core::Util::SceneLoader::CreateComponentType(std::string type, Core::DataStructure::Graph* provider, Core::DataStructure::Node* parent)
{
	const std::vector<LowRenderer::Component*> components = provider->GetComponents();
	for (unsigned int i = 0; i < components.size(); i++)
	{
		if (Text::compareWord(type.c_str(), 0, type.size(), components[i]->GetName()))
		{
			components[i]->Create(parent);
			return true;
		}
	}
	return false;
}