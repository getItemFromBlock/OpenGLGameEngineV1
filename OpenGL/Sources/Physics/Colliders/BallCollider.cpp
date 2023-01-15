#include "Physics/Colliders/BallCollider.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Core/App/App.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/ShaderProgram.hpp"

Physics::Colliders::BallCollider::BallCollider()
{
	Type = ColliderType::Ball;
}

Physics::Colliders::BallCollider::~BallCollider()
{
}

void Physics::Colliders::BallCollider::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new BallCollider());
}

void Physics::Colliders::BallCollider::DeleteComponent()
{
	Collider::~Collider();
	this->~BallCollider();
}

void Physics::Colliders::BallCollider::PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime)
{
	Collider::PhysicUpdate(container, colliders, DeltaTime);
	if (Enabled)
	{
		colliders->push_back(this);
		Core::Maths::Vec3D rot = container->GetGlobalMatrix()->GetRotationFromTranslation(container->GetGlobalMatrix()->GetScaleFromTranslation());
		ModelMatrix = Core::Maths::Mat4D::CreateTransformMatrix(container->GetGlobalMatrix()->GetPositionFromTranslation(), rot, Core::Maths::Vec3D(1)) * Core::Maths::Mat4D::CreateTransformMatrix(Position, Core::Maths::Vec3D(), Core::Maths::Vec3D(Range));
	}
}

void Physics::Colliders::BallCollider::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Collider::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Position", &Position.x, 0.1f);
	ImGui::DragFloat("Rayon", &Range, 0.1f);
}

void Physics::Colliders::BallCollider::DrawCollider(Resources::ResourceManager* resources, Resources::ShaderProgram* program, unsigned int& VAOCurrent, const Core::Maths::Mat4D& vp)
{
	Resources::Mesh* mt = resources->Get<Resources::Mesh>("DebugSphere");
	if (mt)
	{
		Core::Maths::Mat4D mvp = vp * ModelMatrix;
		glUniformMatrix4fv(program->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
		glUniform3f(program->GetLocation(Resources::ShaderData::MatAmbient), Entered ? 1.0f : 0.0f, Entered ? 0.0f : 1.0f, 0.0f);
		glUniform1f(program->GetLocation(Resources::ShaderData::MatAlpha), 1.0f);
		mt->Draw(VAOCurrent);
	}
}

void Physics::Colliders::BallCollider::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "Rayon"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Range = Text::getFloat(data, pos, size);
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
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Physics::Colliders::BallCollider::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Collider::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Rayon " << Range << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Position " << Position.x << " " << Position.y << " " << Position.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}
