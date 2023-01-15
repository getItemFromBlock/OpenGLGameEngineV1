#include "Physics/Colliders/CapsuleCollider.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Core/App/App.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ShaderProgram.hpp"

Physics::Colliders::CapsuleCollider::CapsuleCollider()
{
	Type = ColliderType::Capsule;
}

Physics::Colliders::CapsuleCollider::~CapsuleCollider()
{
}

void Physics::Colliders::CapsuleCollider::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new CapsuleCollider());
}

void Physics::Colliders::CapsuleCollider::DeleteComponent()
{
	Collider::~Collider();
	this->~CapsuleCollider();
}

void Physics::Colliders::CapsuleCollider::PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime)
{
	Collider::PhysicUpdate(container, colliders, DeltaTime);
	if (Enabled)
	{
		colliders->push_back(this);
		Core::Maths::Vec3D rot = container->GetGlobalMatrix()->GetRotationFromTranslation(container->GetGlobalMatrix()->GetScaleFromTranslation());
		Core::Maths::Mat4D parentMat = Core::Maths::Mat4D::CreateTransformMatrix(container->GetGlobalMatrix()->GetPositionFromTranslation(), rot, Core::Maths::Vec3D(1));
		ModelMatrix = parentMat * Core::Maths::Mat4D::CreateTransformMatrix(Position, Rotation, Core::Maths::Vec3D(Rayon, Length / 2, Rayon));
		ModelMatrixUp = parentMat * Core::Maths::Mat4D::CreateRotationMatrix(Rotation) * Core::Maths::Mat4D::CreateTransformMatrix(Position + Core::Maths::Vec3D(0, Length / 2, 0), Core::Maths::Vec3D(), Core::Maths::Vec3D(Rayon));
		ModelMatrixDown = parentMat * Core::Maths::Mat4D::CreateRotationMatrix(Rotation) * Core::Maths::Mat4D::CreateTransformMatrix(Position - Core::Maths::Vec3D(0, Length / 2, 0), Core::Maths::Vec3D(), Core::Maths::Vec3D(Rayon));
		Range = (Length / 2) + Rayon;
	}
}

void Physics::Colliders::CapsuleCollider::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Collider::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Position", &Position.x, 0.1f);
	ImGui::DragFloat3("Rotation", &Rotation.x, 0.1f);
	ImGui::DragFloat("Length", &Length, 0.1f);
	ImGui::DragFloat("Rayon", &Rayon, 0.1f);
}

void Physics::Colliders::CapsuleCollider::DrawCollider(Resources::ResourceManager* resources, Resources::ShaderProgram* program, unsigned int& VAOCurrent, const Core::Maths::Mat4D& vp)
{
	Resources::Mesh* mUp = resources->Get<Resources::Mesh>("DebugCapsuleA");
	Resources::Mesh* mDw = resources->Get<Resources::Mesh>("DebugCapsuleB");
	Resources::Mesh* mMd = resources->Get<Resources::Mesh>("DebugCapsuleC");
	if (mUp && mDw && mMd)
	{
		Core::Maths::Mat4D mvp = vp * ModelMatrixUp;
		glUniform3f(program->GetLocation(Resources::ShaderData::MatAmbient), Entered ? 1.0f : 0.0f, Entered ? 0.0f : 1.0f, 0.0f);
		glUniform1f(program->GetLocation(Resources::ShaderData::MatAlpha), 1.0f);
		glUniformMatrix4fv(program->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
		mUp->Draw(VAOCurrent);
		mvp = vp * ModelMatrixDown;
		glUniformMatrix4fv(program->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
		mDw->Draw(VAOCurrent);
		mvp = vp * ModelMatrix;
		glUniformMatrix4fv(program->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
		mMd->Draw(VAOCurrent);
	}
}

void Physics::Colliders::CapsuleCollider::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Collider::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Position"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, Position))
			{
				err = "Malformated Vec3";
				break;
			}
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
		else if (Text::compareWord(data, pos, size, "Length"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Length = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Rayon"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Rayon = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Physics::Colliders::CapsuleCollider::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Collider::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Position " << Position.x << " " << Position.y << " " << Position.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Rotation " << Rotation.x << " " << Rotation.y << " " << Rotation.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Length " << Length << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Rayon " << Rayon << Rotation.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}
