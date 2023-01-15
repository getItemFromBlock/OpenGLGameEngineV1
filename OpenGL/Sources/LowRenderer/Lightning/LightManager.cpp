#include "LowRenderer/Lightning/LightManager.hpp"

#include <glad/glad.h>

#include "LowRenderer/Lightning/Light.hpp"

LowRenderer::Lightning::LightManager::LightManager()
{
}

LowRenderer::Lightning::LightManager::~LightManager()
{
}

void LowRenderer::Lightning::LightManager::WriteLights(Resources::ShaderProgram** shaderProgramCurrent, Resources::ShaderProgram* shader, const Core::Maths::Vec3D& cameraPos, std::vector< Core::Maths::Mat4D>* lvp, bool first, float GlobalTime)
{
	Resources::ShaderProgram* shaderId = shader;
	if (*shaderProgramCurrent != shaderId)
	{
		glUseProgram(shaderId->GetProgramID());
		*shaderProgramCurrent = shaderId;
	}
	size_t max = MaxLights < DLights.size() ? MaxLights :  DLights.size();
	float* data = new float[max*DIRECTIONAL_SIZE];
	size_t index = 0;
	for (size_t i = 0; i < max; i++)
	{
		float* lightData = DLights[i]->GetValues();
		for (size_t j = 0; j < DIRECTIONAL_SIZE; j++)
		{
			data[index] = lightData[j];
			index++;
		}
		delete[] lightData;
	}
	glUniform1fv(shader->GetLocation(Resources::ShaderData::LDirectional), (int)(max * DIRECTIONAL_SIZE), data);
	glUniform1ui(shader->GetLocation(Resources::ShaderData::LDirectionalCount), (unsigned int)max);
	delete[] data;

	max = MaxLights < PLights.size() ? MaxLights : PLights.size();
	data = new float[max*POINT_SIZE];
	index = 0;
	for (int i = 0; i < max; i++)
	{
		float* lightData = PLights[i]->GetValues();
		for (size_t j = 0; j < POINT_SIZE; j++)
		{
			data[index] = lightData[j];
			index++;
		}
		delete[] lightData;
	}
	glUniform1fv(shader->GetLocation(Resources::ShaderData::LPoint), (int)(max * POINT_SIZE), data);
	glUniform1ui(shader->GetLocation(Resources::ShaderData::LPointCount), (unsigned int)max);
	delete[] data;

	max = MaxLights < SLights.size() ? MaxLights : SLights.size();
	data = new float[max*SPOT_SIZE];
	index = 0;
	size_t index2 = 0;
	int* indexes = new int[max];
	unsigned int* maps = new unsigned int[MaxShadowMaps];
	for (unsigned int i = 0; i < MaxShadowMaps; i++)
	{
		maps[i] = defaultBuffer->GetTextureID();
	}
	for (size_t i = 0; i < max; i++)
	{
		indexes[i] = -1;
		if (SLights[i]->shadowMap && index2 + 1llu < MaxShadowMaps)
		{
			for (size_t s = 0; s < ShadowMapLights.size(); s++)
			{
				if (ShadowMapLights[s] == SLights[i])
				{
					indexes[i] = (int)index2;
					maps[index2] = SLights[i]->shadowMap->GetTextureID();
					if (first) lvp->push_back(SLights[i]->shadowMap->VPLight);
					index2++;
				}
			}
		}
		float* lightData = SLights[i]->GetValues();
		for (size_t j = 0; j < SPOT_SIZE; j++)
		{
			data[index] = lightData[j];
			index++;
		}
		delete[] lightData;
	}
	glUniform1iv(shader->GetLocation(Resources::ShaderData::LShadowMapIndex), (int)max, indexes);
	glUniform1iv(shader->GetLocation(Resources::ShaderData::LShadowMap), (int)MaxShadowMaps, (int*)maps);
	glUniform1fv(shader->GetLocation(Resources::ShaderData::LSpot), (int)(max * SPOT_SIZE), data);
	glUniform1ui(shader->GetLocation(Resources::ShaderData::LSpotCount), (unsigned int)max);
	delete[] data;
	delete[] maps;
	delete[] indexes;

	glUniform3f(shader->GetLocation(Resources::ShaderData::CameraPosition), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(shader->GetLocation(Resources::ShaderData::GlobalTime), GlobalTime);
	glUniform1f(shader->GetLocation(Resources::ShaderData::RNGState), rand()*1.0f/RAND_MAX);
}

void LowRenderer::Lightning::LightManager::EmptyLights()
{
	DLights.clear();
	PLights.clear();
	SLights.clear();
	ShadowMapLights.clear();
}

void LowRenderer::Lightning::LightManager::UpdateShaders(Resources::ShaderProgram** shaderProgramCurrent, Resources::ShaderManager* manager, const Core::Maths::Vec3D& cameraPos, std::vector<Core::Maths::Mat4D>* lvp, float GlobalTime)
{
	std::vector<Resources::ShaderProgram*> shaders = manager->GetShaderPrograms();
	for (unsigned int i = 0; i < shaders.size(); i++)
	{
		WriteLights(shaderProgramCurrent, shaders[i], cameraPos, lvp, i==0, GlobalTime);
	}
}

void LowRenderer::Lightning::LightManager::Init(Resources::ResourceManager* resources)
{
	ShadowMapBuffer::SetShadowMapResolution(1);
	defaultBuffer = resources->Create<ShadowMapBuffer>("Default Shadow Map Buffer");
	ShadowMapBuffer::SetShadowMapResolution(2048);
}