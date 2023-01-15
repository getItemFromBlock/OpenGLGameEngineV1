#include "Resources/ShaderManager.hpp"

#include "Core/DataStructure/NameSearcher.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Core/Debug/Log.hpp"

Resources::ShaderManager::ShaderManager()
{
}

Resources::ShaderManager::~ShaderManager()
{
}

void Resources::ShaderManager::Load(const char* path)
{
	pathName = path;
}

void Resources::ShaderManager::UnLoad()
{
}

const char* Resources::ShaderManager::GetPath()
{
	return pathName.c_str();
}

size_t Resources::ShaderManager::CreateShaderProgram(Resources::VertexShader* vertex, Resources::FragmentShader* fragment, const char* name)
{
	if (GetShaderProgram(name))
	{
		return GetShaderProgramIndex(name);
	}
	ShaderProgram sh = ShaderProgram();
	sh.Create(vertex, fragment, name);
	shaderPrograms.push_back(sh);
	return shaderPrograms.size() - 1;
}

std::vector<Resources::ShaderProgram*> Resources::ShaderManager::GetShaderPrograms()
{
	std::vector<ShaderProgram*> result;
	for (size_t i = 0; i < shaderPrograms.size(); i++) result.push_back(&shaderPrograms[i]);
	return result;
}

std::vector<Resources::ShaderProgram*> Resources::ShaderManager::GetShaderPrograms(const char* value)
{
	if (!value || value[0] == 0) return GetShaderPrograms();
	return Core::DataStructure::NameSearcher::FindElements<Resources::ShaderProgram>(shaderPrograms, value);
}

Resources::ShaderProgram* Resources::ShaderManager::GetShaderProgram(const char* name)
{
	ShaderProgram* result = nullptr;
	for (unsigned int i = 0; i < shaderPrograms.size(); i++)
	{
		if (Text::compareWord(shaderPrograms[i].GetName(), 0, 64, name))
		{
			result = &shaderPrograms[i];
			break;
		}
	}
	return result;
}

size_t Resources::ShaderManager::GetShaderProgramIndex(const char* name)
{
	for (unsigned int i = 0; i < shaderPrograms.size(); i++)
	{
		if (Text::compareWord(shaderPrograms[i].GetName(), 0, 64, name))
		{
			return i;
		}
	}
	return 0;
}

void Resources::ShaderManager::DestroyShaderPrograms()
{
	for (size_t i = 0; i < shaderPrograms.size(); i++)
	{
		shaderPrograms[i].Destroy();
	}
	shaderPrograms.clear();
}

void Resources::ShaderManager::AddShader(Shader* in)
{
	if (!in) return;
	for (size_t i = 0; i < shaders.size(); i++)
	{
		if (in == shaders[i]) return;
	}
	shaders.push_back(in);
}

void Resources::ShaderManager::ReloadShaders()
{
	LOG("Reloading shaders");
	for (size_t i = 0; i < shaders.size(); i++)
	{
		shaders[i]->Reload();
	}
	for (size_t i = 0; i < shaderPrograms.size(); i++)
	{
		shaderPrograms[i].Reload();
	}
	LOG("Shaders reloaded!");
}