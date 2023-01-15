#include "Resources/MaterialManager.hpp"

#include "Core/DataStructure/NameSearcher.hpp"

Resources::MaterialManager::MaterialManager()
{
}

Resources::MaterialManager::~MaterialManager()
{
}

size_t Resources::MaterialManager::CreateMaterial(ResourceManager* manager, const char* path)
{
	Material* mat = manager->Get<Material>(path);
	if (mat)
	{
		for (size_t i = 0; i < materials.size(); i++)
		{
			if (materials[i] == mat)
			{
				return i;
			}
		}
	}
	else
	{
		mat = manager->Create<Material>(path);
	}
	materials.push_back(mat);
	return materials.size() - 1llu;
}

size_t Resources::MaterialManager::AddMaterial(Material* in)
{
	for (size_t i = 0; i < materials.size(); i++)
	{
		if (materials[i] == in)
		{
			return i;
		}
	}
	materials.push_back(in);
	return materials.size() - 1;
}

std::vector<Resources::Material*> Resources::MaterialManager::GetMaterials()
{
	return materials;
}

std::vector<Resources::Material*> Resources::MaterialManager::GetMaterials(const char* filter)
{
	if (!filter || filter[0] == 0) return GetMaterials();
	return Core::DataStructure::NameSearcher::FindElementsPtr<Material>(materials, filter);
}
