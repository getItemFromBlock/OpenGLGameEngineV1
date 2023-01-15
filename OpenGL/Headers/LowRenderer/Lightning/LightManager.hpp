#pragma once

#include <vector>

#include "Core/Maths/Maths.hpp"

#include "Resources/ShaderManager.hpp"
#include "LowRenderer/Lightning/ShadowMapBuffer.hpp"

namespace LowRenderer
{
	class DirectionalLight;
	class PointLight;
	class SpotLight;
}
namespace Resources
{
	class ResourceManager;
}

namespace LowRenderer::Lightning
{
	class LightManager
	{
	public:
		LightManager();
		~LightManager();

		void Init(Resources::ResourceManager* resources);

		std::vector<LowRenderer::DirectionalLight*> DLights;
		std::vector<LowRenderer::PointLight*> PLights;
		std::vector<LowRenderer::SpotLight*> SLights;
		std::vector<LowRenderer::SpotLight*> ShadowMapLights;

		void UpdateShaders(Resources::ShaderProgram** shaderProgramCurrent, Resources::ShaderManager* manager, const Core::Maths::Vec3D& cameraPos, std::vector<Core::Maths::Mat4D>* lvp, float GlobalTime);

		void EmptyLights();

	private:
		unsigned int MaxLights = 8;
		unsigned int MaxShadowMaps = 2;
		void WriteLights(Resources::ShaderProgram** shaderProgramCurrent, Resources::ShaderProgram* shader, const Core::Maths::Vec3D& cameraPos, std::vector<Core::Maths::Mat4D>* lvp, bool first, float GlobalTime);
		ShadowMapBuffer* defaultBuffer = nullptr;
	};
}