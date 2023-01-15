#include "Resources/Texture.hpp"
#include "Core/Debug/Log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB_Image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB_Image/stb_image_write.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace Resources;

unsigned int Texture::filter = GL_LINEAR;
unsigned int Texture::wrap = GL_REPEAT;
Texture::Texture()
{
}

Texture::~Texture()
{
}

Core::Maths::UChar4D Resources::Texture::ReadPixel(Core::Maths::Int2D pos)
{
	if (!loaded || !ImageData || pos.x >= sizeX || pos.y >= sizeY)
		return Core::Maths::UChar4D();
	size_t index = ((size_t)pos.x + ((size_t)sizeX - pos.y - 1) * sizeX) * 4;
	return Core::Maths::UChar4D(ImageData[index], ImageData[index+1], ImageData[index+2], ImageData[index+3]);
}

void Texture::SetFilterType(unsigned int in)
{
	filter = in;
}

void Texture::SetWrapType(unsigned int in)
{
	wrap = in;
}

void Texture::Load(const char* path)
{
	fullPath = path;
	if (loaded) UnLoad();
	int nrChannels;
	int startIndex = 0;
	char tmp;
	for (int i = 0; i < 255; i++)
	{
		tmp = path[i];
		if (tmp == 0)
		{
			break;
		}
		else if (tmp == '\\' || tmp == '/')
		{
			startIndex = i + 1;
		}
	}
	tmp = path[startIndex];
	int index = 0;
	for (int i = startIndex + 1; i < 255 && tmp != 0 && tmp != '.'; i++)
	{
		Name[index] = tmp;
		tmp = path[i];
		index++;
	}
	Name[index] = 0;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &sizeX, &sizeY, &nrChannels, 4);
	if (!data)
	{
		LOGRAW("ERROR could not load file %s: ", path);
		LOG(stbi_failure_reason());
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (ShouldDeleteData)
	{
		stbi_image_free(data);
	}
	else
	{
		ImageData = data;
	}

	glGenSamplers(1, &textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_S, wrap);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_T, wrap);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, filter);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameterf(textureSampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);

	glBindTextureUnit(textureID, textureID);
	glBindSampler(textureID, textureSampler);

	loaded = true;
}

void Resources::Texture::DeleteData()
{
	if (ImageData) stbi_image_free(ImageData);
}

void Texture::UnLoad()
{
	if (!loaded) return;
	glDeleteTextures(1, &textureID);
	glDeleteSamplers(1, &textureSampler);
	loaded = false;
	this->~Texture();
}

void Texture::Overwrite(const unsigned char* data, unsigned int sizeX, unsigned int sizeY)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	this->sizeX = sizeX;
	this->sizeY = sizeY;
}

const char* Texture::GetPath()
{
	return fullPath.c_str();
}

void Texture::SaveImage(const char* path, unsigned char* data, unsigned int sizeX, unsigned int sizeY)
{
	stbi_flip_vertically_on_write(true);
	std::string name = path;
	name.append("@");
	time_t timeLocal;
	struct tm dateTime;
	char text[64];
	time(&timeLocal);
	localtime_s(&dateTime, &timeLocal);
	strftime(text, 64, "%Y_%m_%d-%H_%M_%S", &dateTime);
	name.append(text);
	name.append(".png");
	if (!stbi_write_png(name.c_str(), sizeX, sizeY, 4, data, sizeX*4))
	{
		LOGRAW("ERROR could not save file %s: ", path);
		LOG(stbi_failure_reason());
	}
	else
	{
		LOG("Saved screenshot as %s", name.c_str());
	}
}

GLFWimage* Resources::Texture::ReadIcon(const char* path)
{
	int x, y, n;
	unsigned char* data = stbi_load(path, &x, &y, &n, 4);
	if (!data)
	{
		LOGRAW("ERROR could not load file %s: ", path);
		LOG(stbi_failure_reason());
	}
	GLFWimage* iconOut = new GLFWimage();
	iconOut->height = y;
	iconOut->width = x;
	iconOut->pixels = data;
	return iconOut;
}
