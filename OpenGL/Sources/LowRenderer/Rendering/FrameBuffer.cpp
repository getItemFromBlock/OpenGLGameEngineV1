#include "LowRenderer/Rendering/FrameBuffer.hpp"

#include <glad/glad.h>
#include <ImGUI/imgui.h>

#include "Core/Debug/Log.hpp"
#include "Core/Debug/Assert.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/TextureManager.hpp"

unsigned int GetTextureWrap(TextureWrapType type)
{
	switch (type)
	{
	case TextureWrapType::Repeat:
		return GL_REPEAT;
	case TextureWrapType::MirroredRepeat:
		return GL_MIRRORED_REPEAT;
	case TextureWrapType::MirroredClamp:
		return GL_MIRROR_CLAMP_TO_EDGE;
	case TextureWrapType::ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case TextureWrapType::ClampToBorder:
		return GL_CLAMP_TO_BORDER;
	}
	return GL_REPEAT;
}

unsigned int GetTextureFilter(TextureFilterType type)
{
	return type == TextureFilterType::Linear ? GL_LINEAR : GL_NEAREST;
}
LowRenderer::Rendering::FrameBuffer::FrameBuffer()
{
}

LowRenderer::Rendering::FrameBuffer::~FrameBuffer()
{
}

bool LowRenderer::Rendering::FrameBuffer::Init(unsigned int width, unsigned int height, TextureFilterType TFilter, TextureWrapType TWrap)
{
	glGenFramebuffers(1, &FBO);

	sizeX = width;
	sizeY = height;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, (void*)0);

	glGenSamplers(1, &textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_S, GetTextureWrap(TWrap));
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_T, GetTextureWrap(TWrap));
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GetTextureFilter(TFilter));
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTextureUnit(textureID, textureID);
	glBindSampler(textureID, textureSampler);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

	glGenRenderbuffers(1, &depthID);
	glBindRenderbuffer(GL_RENDERBUFFER, depthID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	depthID);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		LOG("Error, could not create FrameBuffer : 0x%x\n", Status);
		return false;
	}
	loaded = true;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	return true;
}

bool LowRenderer::Rendering::FrameBuffer::BindForWriting()
{
	if (FBO == 0) return false;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glViewport(0, 0, sizeX, sizeY);
	return true;
}

void LowRenderer::Rendering::FrameBuffer::BindForReading()
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void LowRenderer::Rendering::FrameBuffer::Update(unsigned int width, unsigned int height, TextureFilterType TFilter, TextureWrapType TWrap)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, (void*)0);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_S, GetTextureWrap(TWrap));
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_T, GetTextureWrap(TWrap));
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GetTextureFilter(TFilter));
	WrapType = TWrap;
	FilterType = TFilter;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, depthID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	sizeX = width;
	sizeY = height;
}

void LowRenderer::Rendering::FrameBuffer::Load(const char* path)
{
	fullPath = path;
	if (loaded) UnLoad();
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
	loaded = true;
}

void LowRenderer::Rendering::FrameBuffer::UnLoad()
{
	if (!loaded) return;
	Texture::UnLoad();
	glDeleteFramebuffers(1,&FBO);
	glDeleteRenderbuffers(1, &depthID);
	this->~FrameBuffer();
}