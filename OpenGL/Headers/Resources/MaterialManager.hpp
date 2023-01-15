#pragma once

#include <vector>

#include "Resources/Material.hpp"
#include "Resources/ResourceManager.hpp"

namespace Resources
{
	class MaterialManager
	{
	public:
		MaterialManager();
		~MaterialManager();
		size_t CreateMaterial(ResourceManager* manager, const char* path);
		size_t AddMaterial(Material* in);
		std::vector<Material*> GetMaterials();
		std::vector<Material*> GetMaterials(const char* filter);
	private:
		std::vector<Material*> materials;
	};
}