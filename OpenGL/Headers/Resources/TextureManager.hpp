#pragma once

#include <vector>

#include "Resources/Texture.hpp"
#include "Resources/Font.hpp"
#include "Resources/ResourceManager.hpp"

namespace LowRenderer
{
	namespace Lightning
	{
		class ShadowMapBuffer;
	}
	namespace Rendering
	{
		class FrameBuffer;
	}
}

namespace Resources
{
	class TextureManager
	{
	public:
		TextureManager();
		~TextureManager();
		size_t CreateTexture(ResourceManager* manager, const char* path, unsigned int textureParam);
		size_t CreateTexture(ResourceManager* manager, const char* path);
		size_t CreateFont(ResourceManager* manager, const char* path, unsigned int textureParam);
		size_t AddShadowMap(LowRenderer::Lightning::ShadowMapBuffer* in);
		size_t AddFrameBuffer(LowRenderer::Rendering::FrameBuffer* in);
		void ClearShadowMaps(ResourceManager* manager);
		std::vector<Texture*> GetTextures();
		std::vector<Texture*> GetTextures(const char* filter);
	private:
		std::vector<Texture*> textures;
		std::vector<LowRenderer::Lightning::ShadowMapBuffer*> shadowMaps;
		bool IsFont = false;
	};
}