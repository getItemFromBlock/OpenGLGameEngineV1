#pragma once

namespace Resources
{
	class ResourceManager;
	class ShaderManager;
	class TextureManager;
	class MaterialManager;
	class MeshManager;
}

namespace Resources::ResourceLoader
{
	void LoadResources(ResourceManager* manager, MeshManager* meshes, MaterialManager* materials, TextureManager* textures, ShaderManager* shaders);
}