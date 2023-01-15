#include "LowRenderer/RotationController.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Core/Util/TextHelper.hpp"

LowRenderer::RotationController::RotationController()
{
}

LowRenderer::RotationController::~RotationController()
{
}

void LowRenderer::RotationController::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
	if (!Enabled) return;
	container->SetLocalRotation(container->GetLocalRotation() + RotationDelta * deltaTime);
}

void LowRenderer::RotationController::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new LowRenderer::RotationController());
}

void LowRenderer::RotationController::DeleteComponent()
{
	this->~RotationController();
}

void LowRenderer::RotationController::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Component::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "RotationDelta"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, RotationDelta))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::RotationController::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "RotationDelta " << RotationDelta.x << " " << RotationDelta.y << " " << RotationDelta.z << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::RotationController::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Rotation Delta", &RotationDelta.x, 0.1f);
}
