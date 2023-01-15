#pragma once

#include <vector>

#include "ShaderProgram.hpp"

#include "IResource.hpp"

namespace Resources
{
	class ShaderManager : public IResource
	{
	public:
		ShaderManager();
		~ShaderManager();

		void Load(const char* path) override;
		void UnLoad() override;
		const char* GetPath() override;

		size_t CreateShaderProgram(Resources::VertexShader* vertex, Resources::FragmentShader* fragment, const char* name);
		std::vector<ShaderProgram*> GetShaderPrograms();
		std::vector<ShaderProgram*> GetShaderPrograms(const char* filter);
		ShaderProgram* GetShaderProgram(const char* name);
		size_t GetShaderProgramIndex(const char* name);
		void AddShader(Resources::Shader* in);
		void ReloadShaders();
		void DestroyShaderPrograms();
	private:
		std::vector<ShaderProgram> shaderPrograms;
		std::vector<Resources::Shader*> shaders;
		std::string pathName;
	};
}