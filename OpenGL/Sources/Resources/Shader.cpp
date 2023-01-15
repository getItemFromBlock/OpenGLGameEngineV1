#include "Resources/Shader.hpp"

#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Core/Debug/Log.hpp"
#include "Core/Util/ModelLoader.hpp"

namespace Resources
{
	Shader::Shader(int type)
	{
		shaderType = type;
		shaderIndex = 0;
	}

	void Shader::LoadShader(const char* path)
	{
		fullPath = path;
		int64_t size;
		const char* data = Core::Util::ModelLoader::loadFile(path,size);
		if (!data) return;
		shaderIndex = glCreateShader(shaderType);
		glShaderSource(shaderIndex, 1, &data, NULL);
		glCompileShader(shaderIndex);
		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(shaderIndex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderIndex, 512, NULL, infoLog);
			Core::Debug::Log::Print("ERROR : Shader %s of type %s failed compiling!\n%s",path, (shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"), infoLog);
		}
		delete[] data;
	}

	void Resources::Shader::Reload()
	{
		if (!fullPath.c_str()[0]) return;
		int64_t size;
		const char* data = Core::Util::ModelLoader::loadFile(fullPath.c_str(), size);
		if (!data) return;
		glShaderSource(shaderIndex, 1, &data, NULL);
		glCompileShader(shaderIndex);
		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(shaderIndex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderIndex, 512, NULL, infoLog);
			Core::Debug::Log::Print("ERROR : Shader %s of type %s failed compiling!\n%s", fullPath, (shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"), infoLog);
		}
		delete[] data;
	}

	int Shader::GetShaderIndex()
	{
		return shaderIndex;
	}

	Shader::~Shader()
	{
	}

	void Shader::DeleteShader()
	{
		glDeleteShader(shaderIndex);
	}

	FragmentShader::FragmentShader() : Shader(GL_FRAGMENT_SHADER)
	{
	}

	FragmentShader::~FragmentShader()
	{
	}

	void FragmentShader::Load(const char* path)
	{
		LoadShader(path);
	}

	void FragmentShader::UnLoad()
	{
		DeleteShader();
		Shader::~Shader();
		this->~FragmentShader();
	}

	const char* FragmentShader::GetPath()
	{
		return fullPath.c_str();
	}

	VertexShader::VertexShader() : Shader(GL_VERTEX_SHADER)
	{
	}

	VertexShader::~VertexShader()
	{
	}

	void VertexShader::Load(const char* path)
	{
		LoadShader(path);
	}
	void VertexShader::UnLoad()
	{
		DeleteShader();
		Shader::~Shader();
		this->~VertexShader();
	}
	const char* VertexShader::GetPath()
	{
		return fullPath.c_str();
	}
}