#include "LowRenderer/CameraConstraint.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Core/App/App.hpp"
#include "Core/Util/TextHelper.hpp"

LowRenderer::CameraConstraint::CameraConstraint()
{
}

LowRenderer::CameraConstraint::~CameraConstraint()
{
}

void LowRenderer::CameraConstraint::DeleteComponent()
{
	this->~CameraConstraint();
}

void LowRenderer::CameraConstraint::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::Checkbox("Copy Camera Position", &CopyPosition);
	ImGui::Checkbox("Copy Camera Rotation", &CopyRotation);
}

void LowRenderer::CameraConstraint::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "CopyPosition"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, CopyPosition))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "CopyRotation"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, CopyRotation))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::CameraConstraint::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "CopyPosition " << (CopyPosition ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "CopyRotation " << (CopyRotation ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::CameraConstraint::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
	if (!Enabled) return;
	Core::Maths::Vec3D cameraRot = Core::App::App::GetMainCamera()->rotation;
	cameraRot = Core::Maths::Vec3D(cameraRot.y, cameraRot.x + 180, cameraRot.z);
	Core::Maths::Mat4D dest = container->GetParent()->GetGlobalMatrix()->CreateInverseMatrix() * Core::Maths::Mat4D::CreateTransformMatrix(Core::App::App::GetMainCamera()->position, cameraRot, Core::Maths::Vec3D(1));
	if (CopyPosition) container->SetLocalPosition(dest.GetPositionFromTranslation());
	if (CopyRotation) container->SetLocalRotation(dest.GetRotationFromTranslation(dest.GetScaleFromTranslation()));
	container->Refresh();
}

void LowRenderer::CameraConstraint::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new CameraConstraint());
}