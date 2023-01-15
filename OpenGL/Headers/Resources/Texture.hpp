#pragma once

#include <string>

#include "Core/Maths/Maths.hpp"

#include "Resources/IResource.hpp"
#include "Core/DataStructure/INameable.hpp"

struct GLFWimage;

namespace Resources
{
	class Texture : public IResource, public Core::DataStructure::INameable
	{
	public:

		Texture();
		~Texture();

		virtual void Load(const char* path) override;
		void Overwrite(const unsigned char* data, unsigned int sizeX, unsigned int sizeY);
		const char* GetPath() override;
		virtual void UnLoad() override;

		void DeleteData();

		const char* GetName() override { return Name; };
		unsigned int GetTextureID() { return textureID; }
		int GetTextureWidth() { return sizeX; }
		int GetTextureHeight() { return sizeY; }
		Core::Maths::UChar4D ReadPixel(Core::Maths::Int2D pos);

		static void SetFilterType(unsigned int in);
		static void SetWrapType(unsigned int in);

		static void SaveImage(const char* path, unsigned char* data, unsigned int sizeX, unsigned int sizeY);
		static GLFWimage* ReadIcon(const char* path);
	protected:
		bool loaded = false;
		char Name[256] = "Texture";
		std::string fullPath = "";
		unsigned int textureID = 0;
		unsigned int textureSampler = 0;
		int sizeX = 0;
		int sizeY = 0;
		static unsigned int filter;
		static unsigned int wrap;
		bool ShouldDeleteData = true;
		unsigned char* ImageData = nullptr;
	};
}