#include "LowRenderer/Lightning/Light.hpp"

#include <ImGUI/imgui.h>
#include <glad/glad.h>

#include "Core/DataStructure/Node.hpp"
#include "LowRenderer/Lightning/LightManager.hpp"
#include "Core/Util/TextHelper.hpp"
#include "LowRenderer/Rendering/RenderCamera.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"

const char* const ShadowUpdateTypeDesc[3] =
{
	"None",
	"Baked",
	"Real Time",
};

void LowRenderer::Light::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Component::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "AmbientLight"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, AmbientLight))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "DiffuseLight"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, DiffuseLight))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "SpecularLight"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, SpecularLight))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "Smoothness"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Smoothness = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "ShadowType"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string type = Text::getText(data, pos, size);
			if (!type.c_str())
			{
				err = "Expected type after \"ShadowType\"";
				break;
			}
			bool valid = false;
			for (unsigned int i = 0; i < 3; i++)
			{
				if (Text::compareWord(type.c_str(), 0, type.size(), ShadowUpdateTypeDesc[i]))
				{
					valid = true;
					ShadowType = (ShadowUpdateType)i;
					break;
				}
			}
			if (!valid)
			{
				err = "Incorrect type after \"ShadowType\"";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "ShadowNear"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			nearPlane = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "ShadowFar"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			farPlane = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::Light::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "AmbientLight " << AmbientLight.x << " " << AmbientLight.y << " " << AmbientLight.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "DiffuseLight " << DiffuseLight.x << " " << DiffuseLight.y << " " << DiffuseLight.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "SpecularLight " << SpecularLight.x << " " << SpecularLight.y << " " << SpecularLight.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Smoothness " << Smoothness << std::endl;
	if (shadowMap)
	{
		Parsing::Shift(fileOut, rec);
		fileOut << "ShadowType " << ShadowUpdateTypeDesc[(int)ShadowType] << std::endl;
		Parsing::Shift(fileOut, rec);
		fileOut << "ShadowNear " << nearPlane << std::endl;
		Parsing::Shift(fileOut, rec);
		fileOut << "ShadowFar " << farPlane << std::endl;
	}
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::Light::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
}

void LowRenderer::Light::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::Combo("Shadow Type", (int*)&ShadowType, ShadowUpdateTypeDesc, 3, -1);
	if (ShadowType != ShadowUpdateType::None && shadowMap)
	{
		ImGui::DragFloat("Near Plane", &nearPlane, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Far Plane", &farPlane, 0.1f, 0.0f, 0.0f, "%.2f");
		if (nearPlane < 0.01f) nearPlane = 0.01f;
		if (farPlane < nearPlane) farPlane = nearPlane;
	}
	ImGui::ColorEdit3("Ambient Light", &AmbientLight.x);
	ImGui::ColorEdit3("Diffuse Light", &DiffuseLight.x);
	ImGui::ColorEdit3("Specular Light", &SpecularLight.x);
	ImGui::DragFloat("Smoothness", &Smoothness, 1.0f, 0.0f, 0.0f, "%.1f");
	if (Smoothness <= 0.0f) Smoothness = 0.001f;
}

void LowRenderer::DirectionalLight::DeleteComponent()
{
	Light::~Light();
	this->~DirectionalLight();
}

void LowRenderer::DirectionalLight::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Light::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Rotation"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, Rotation))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::DirectionalLight::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Light::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Rotation " << Rotation.x << " " << Rotation.y << " " << Rotation.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::DirectionalLight::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
	if (!Enabled) return;
	GlobalDirection = (*container->GetGlobalMatrix() * (Core::Maths::Mat4D::CreateTransformMatrix(Core::Maths::Vec3D(), Rotation, Core::Maths::Vec3D(1)) * Core::Maths::Vec4D(0, 0, 1, 0))).getVector().unitVector();
	lightManager->DLights.push_back(this);
	Light::Update(container, cameras, resources, textureManager, lightManager, deltaTime);
}

float* LowRenderer::DirectionalLight::GetValues()
{
	float* result = new float[DIRECTIONAL_SIZE];
	result[0] = AmbientLight.x;
	result[1] = AmbientLight.y;
	result[2] = AmbientLight.z;
	result[3] = DiffuseLight.x;
	result[4] = DiffuseLight.y;
	result[5] = DiffuseLight.z;
	result[6] = SpecularLight.x;
	result[7] = SpecularLight.y;
	result[8] = SpecularLight.z;
	result[9] = Smoothness;
	result[10] = GlobalDirection.x;
	result[11] = GlobalDirection.y;
	result[12] = GlobalDirection.z;
	return result;
}

void LowRenderer::DirectionalLight::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new LowRenderer::DirectionalLight());
}

void LowRenderer::DirectionalLight::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Light::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Direction", &Rotation.x, 0.1f);
}

void LowRenderer::PointLight::DeleteComponent()
{
	Light::~Light();
	this->~PointLight();
}

void LowRenderer::PointLight::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Light::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Attenuation"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec2D(data, pos, size, Attenuation))
			{
				err = "Malformated Vec2";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::PointLight::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Light::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Attenuation " << Attenuation.x << " " << Attenuation.y << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::PointLight::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
	if (!Enabled) return;
	Position = (*container->GetGlobalMatrix() * Core::Maths::Vec4D(0, 0, 0, 1)).getVector();
	lightManager->PLights.push_back(this);
	Light::Update(container, cameras, resources, textureManager, lightManager, deltaTime);
}

float* LowRenderer::PointLight::GetValues()
{
	float* result = new float[POINT_SIZE];
	result[0] = Position.x;
	result[1] = Position.y;
	result[2] = Position.z;
	result[3] = AmbientLight.x;
	result[4] = AmbientLight.y;
	result[5] = AmbientLight.z;
	result[6] = DiffuseLight.x;
	result[7] = DiffuseLight.y;
	result[8] = DiffuseLight.z;
	result[9] = SpecularLight.x;
	result[10] = SpecularLight.y;
	result[11] = SpecularLight.z;
	result[12] = Smoothness;
	result[13] = Attenuation.x;
	result[14] = Attenuation.y;
	return result;
}

void LowRenderer::PointLight::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Light::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::SliderFloat2("Attenuation Factors", &Attenuation.x, 0.0f, 1.0f, "%.3f");
}

void LowRenderer::PointLight::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new LowRenderer::PointLight());
}

void LowRenderer::SpotLight::DeleteComponent()
{
	Light::~Light();
	PointLight::~PointLight();
	this->~SpotLight();
}

void LowRenderer::SpotLight::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	LowRenderer::PointLight::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Rotation"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, Rotation))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "SpotAngle"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Spotangle = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "SpotRatio"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Spotratio = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::SpotLight::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	PointLight::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Rotation " << Rotation.x << " " << Rotation.y << " " << Rotation.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "SpotAngle " << Spotangle << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "SpotRatio " << Spotratio << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::SpotLight::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, Lightning::LightManager* lightManager, float deltaTime)
{
	if (!Enabled) return;
	Position = (*container->GetGlobalMatrix() * Core::Maths::Vec4D(0, 0, 0, 1)).getVector();
	Core::Maths::Mat4D tranform = Core::Maths::Mat4D::CreateTransformMatrix(Core::Maths::Vec3D(), Rotation, Core::Maths::Vec3D(1));
	GlobalDirection = (*container->GetGlobalMatrix() * (tranform * Core::Maths::Vec4D(0, 0, 1, 0))).getVector().unitVector();
	GlobalUp = (*container->GetGlobalMatrix() * (tranform * Core::Maths::Vec4D(0, 1, 0, 0))).getVector().unitVector();
	lightManager->SLights.push_back(this);
	if (ShadowType == ShadowUpdateType::Baked || ShadowType == ShadowUpdateType::Realtime)
	{
		lightManager->ShadowMapLights.push_back(this);
	}
	Light::Update(container, cameras, resources, textureManager, lightManager, deltaTime);
}

float* LowRenderer::SpotLight::GetValues()
{
	float* result = new float[SPOT_SIZE];
	result[0] = Position.x;
	result[1] = Position.y;
	result[2] = Position.z;
	result[3] = AmbientLight.x;
	result[4] = AmbientLight.y;
	result[5] = AmbientLight.z;
	result[6] = DiffuseLight.x;
	result[7] = DiffuseLight.y;
	result[8] = DiffuseLight.z;
	result[9] = SpecularLight.x;
	result[10] = SpecularLight.y;
	result[11] = SpecularLight.z;
	result[12] = Smoothness;
	result[13] = Attenuation.x;
	result[14] = Attenuation.y;
	result[15] = GlobalDirection.x;
	result[16] = GlobalDirection.y;
	result[17] = GlobalDirection.z;
	result[18] = Spotangle;
	result[19] = Spotratio;
	return result;
}

void LowRenderer::SpotLight::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	PointLight::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Direction", &Rotation.x, 0.1f);
	ImGui::SliderAngle("Spot Angle", &Spotangle, 0.0f, 180.0f, "%.3f");
	ImGui::SliderFloat("Spot Ratio", &Spotratio, 0.0f, 1.0f, "%.3f");
}

void LowRenderer::SpotLight::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new LowRenderer::SpotLight());
}