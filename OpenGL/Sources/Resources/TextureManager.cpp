#include "Resources/TextureManager.hpp"

#include "Core/DataStructure/NameSearcher.hpp"
#include "LowRenderer/Lightning/ShadowMapBuffer.hpp"
#include "LowRenderer/Rendering/FrameBuffer.hpp"

using namespace Resources;

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}

size_t TextureManager::CreateTexture(ResourceManager* manager, const char* path)
{
	std::string path2;
	for (size_t c = 0; path[c] != 0; c++)
	{
		if (path[c] == '/') path2.append("\\\\");
		else path2 += path[c];
	}
	Texture* tex;
	if (IsFont) tex = dynamic_cast<Texture*>(manager->Get<Font>(path2.c_str()));
	else tex = manager->Get<Texture>(path2.c_str());
	if (tex)
	{
		for (size_t i = 0; i < textures.size(); i++)
		{
			if (textures[i] == tex)
			{
				return i;
			}
		}
	}
	else
	{
		if (IsFont)
		{
			tex = dynamic_cast<Texture*>(manager->Create<Font>(path));
			IsFont = false;
		}
		else tex = manager->Create<Texture>(path);
	}
	textures.push_back(tex);
	return textures.size() - 1llu;
}
size_t TextureManager::AddShadowMap(LowRenderer::Lightning::ShadowMapBuffer* in)
{
	shadowMaps.push_back(in);
	return shadowMaps.size() - 1llu;
}

size_t TextureManager::AddFrameBuffer(LowRenderer::Rendering::FrameBuffer* in)
{
	textures.push_back((Texture*)in);
	return textures.size() - 1llu;
}

size_t TextureManager::CreateTexture(ResourceManager* manager, const char* path, unsigned int textureParam)
{
	Texture::SetFilterType(textureParam);
	return CreateTexture(manager, path);
}

size_t Resources::TextureManager::CreateFont(ResourceManager* manager, const char* path, unsigned int textureParam)
{
	Texture::SetFilterType(textureParam);
	IsFont = true;
	return CreateTexture(manager, path);
}

std::vector<Texture*> TextureManager::GetTextures()
{
	return textures;
}

std::vector<Texture*> TextureManager::GetTextures(const char* filter)
{
	if (!filter || filter[0] == 0) return GetTextures();
	return Core::DataStructure::NameSearcher::FindElementsPtr<Texture>(textures, filter);
}

void TextureManager::ClearShadowMaps(ResourceManager* manager)
{
	for (size_t n = 0; n < shadowMaps.size(); n++)
	{
		manager->Delete(shadowMaps[n]->GetPath());
	}
	shadowMaps.clear();
}