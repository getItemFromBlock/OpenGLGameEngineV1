#pragma once

#include <string>

#include "IResource.hpp"

namespace Resources
{
	class Shader
	{
	public:
		Shader(int type);
		~Shader();
		void LoadShader(const char* path);
		void Reload();
		int GetShaderIndex();
		void DeleteShader();
	private:
		int shaderIndex;
		int shaderType;
	protected:
		std::string fullPath = "";
	};

	class FragmentShader : public Shader, public IResource
	{
	public:
		FragmentShader();
		~FragmentShader();
		void Load(const char* path) override;
		void UnLoad() override;
		const char* GetPath() override;
	};

	class VertexShader : public Shader, public IResource
	{
	public:
		VertexShader();
		~VertexShader();
		void Load(const char* path) override;
		void UnLoad() override;
		const char* GetPath() override;
	};
}