#include "Scripts/GravityArea.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

#include "Core/DataStructure/Node.hpp"
#include "Physics/Colliders/Collider.hpp"
#include "Physics/RigidBody.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ResourceManager.hpp"
#include "Scripts/Entity/EntityBase.hpp"

const char* const FieldTypeDesc[3] =
{
	"Box Field Type",
	"Sphere Field Type",
	"Inverted Sphere Field Type",
};

void Scripts::GravityArea::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new GravityArea());
}

void Scripts::GravityArea::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (!parent) parent = container;
	if (Core::App::App::IsDebugMode())
	{
		if (!SphereMesh) SphereMesh = resources->Get<Resources::Mesh>("DebugSphere");
		if (Type != FieldType::Box)
		{
			Core::Maths::Vec3D size = container->GetGlobalMatrix()->GetScaleFromTranslation();
			if (size.x == 0.0f || size.y == 0.0f || size.z == 0.0f) ModelMatrix = Core::Maths::Mat4D::Identity();
			else
			{
				ModelMatrix = Core::Maths::Mat4D::CreateTransformMatrix(container->GetGlobalMatrix()->GetPositionFromTranslation(),
					container->GetGlobalMatrix()->GetRotationFromTranslation(size), Core::Maths::Vec3D(MaxPlane));
			}
		}
	}
}

void Scripts::GravityArea::Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode mode, bool isSelected)
{
	if (!Enabled || mode != RenderMode::DebugHalo || Type == FieldType::Box) return;
	if (SphereMesh)
	{
		if (!Core::App::App::IsWireframe())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
		}
		Core::Maths::Mat4D mvp = vp * ModelMatrix;
		glUniformMatrix4fv((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
		glUniform3f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAmbient), 0.0f, 0.0f, 1.0f);
		glUniform1f((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MatAlpha), 1.0f);
		SphereMesh->Draw(VAOCurrent);
		if (!Core::App::App::IsWireframe())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
		}
	}
}

void Scripts::GravityArea::DeleteComponent()
{
	this->~GravityArea();
}

void Scripts::GravityArea::OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other)
{
	if (!Enabled || !parent || !other->AttachedBody) return;
	if (Type == FieldType::Box)
	{
		Core::Maths::Vec3D dir = parent->GetGlobalMatrix()->operator*(Core::Maths::Vec4D(0,1,0,0)).getVector().unitVector().negate();
		if (other->AttachedBody && other->AttachedBody->GetParent())
		{
			for (size_t i = 0; i < other->AttachedBody->GetParent()->components.size(); i++)
			{
				Scripts::Entity::EntityBase* n = dynamic_cast<Scripts::Entity::EntityBase*>(other->AttachedBody->GetParent()->components[i]);
				if (n)
				{
					n->CurrentUp = dir.unitVector().negate();
				}
			}
		}
		other->AttachedBody->Gravity = dir.unitVector() * GravityField.getLength();
	}
	else
	{
		Core::Maths::Vec3D dir = parent->GetGlobalMatrix()->GetPositionFromTranslation() - other->AttachedBody->GetParent()->GetGlobalMatrix()->GetPositionFromTranslation();
		float length = dir.getLength();
		if (MaxPlane > 0 && length > MaxPlane) return;
		if (length == 0) dir = Core::Maths::Vec3D(0,1,0);
		if (Type == FieldType::InverseSphere) dir = dir.negate();
		if (other->AttachedBody && other->AttachedBody->GetParent())
		{
			for (size_t i = 0; i < other->AttachedBody->GetParent()->components.size(); i++)
			{
				Scripts::Entity::EntityBase* n = dynamic_cast<Scripts::Entity::EntityBase*>(other->AttachedBody->GetParent()->components[i]);
				if (n)
				{
					n->CurrentUp = dir.unitVector().negate();
				}
			}
		}
		other->AttachedBody->Gravity = dir.unitVector() * GravityField.getLength();
	}
}

void Scripts::GravityArea::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "GravityField"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, GravityField))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "GravityType"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode > static_cast<unsigned char>(FieldType::InverseSphere))
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"GravityType\" : valid numbers are [0-2]", mode);
				err = buff;
				break;
			}
			Type = static_cast<FieldType>(mode);
		}
		else if (Text::compareWord(data, pos, size, "MaxPlane"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			MaxPlane = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Scripts::GravityArea::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "GravityField " << GravityField.x << " " << GravityField.y << " " << GravityField.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "GravityType " << static_cast<int>(Type) << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "MaxPlane " << MaxPlane << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void Scripts::GravityArea::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Gravity Vector", &GravityField.x, 0.1f);
	ImGui::DragFloat("Max Distance", &MaxPlane, 0.1f);
	ImGui::Combo("Gravity Field Type", (int*)&Type, FieldTypeDesc, 3, -1);
}
