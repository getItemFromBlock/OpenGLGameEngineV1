#include "Physics/RigidBody.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Physics/Colliders/Collider.hpp"

#define TERMINAL_VEL 50.0f

Physics::RigidBody::RigidBody()
{
}

Physics::RigidBody::~RigidBody()
{
}

void Physics::RigidBody::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new Physics::RigidBody());
}

void Physics::RigidBody::DeleteComponent()
{
	this->~RigidBody();
}

void Physics::RigidBody::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Velocity", &Velocity.x, 0.1f);
	ImGui::DragFloat3("RotVelocity", &RotVelocity.x, 0.1f);
	ImGui::DragFloat3("Gravity", &Gravity.x, 0.1f);
	ImGui::DragFloat("Mass", &Mass, 0.1f, 0.0f, HUGE_VALF);
	ImGui::DragFloat("Bounciness", &Bounciness, 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat("Dynamic Friction", &DynamicFriction, 0.1f, 0.0f, 1.0f);
	ImGui::DragFloat("Static Friction", &StaticFriction, 0.1f, 0.0f, 1.0f);
}

void Physics::RigidBody::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "Velocity"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, Velocity))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "RotVelocity"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, RotVelocity))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "Gravity"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, Gravity))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "Mass"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Mass = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Bounciness"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Bounciness = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "DynamicFriction"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			DynamicFriction = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "StaticFriction"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			StaticFriction = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Physics::RigidBody::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Velocity " << Velocity.x << " " << Velocity.y << " " << Velocity.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "RotVelocity " << RotVelocity.x << " " << RotVelocity.y << " " << RotVelocity.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Gravity " << Gravity.x << " " << Gravity.y << " " << Gravity.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Mass " << Mass << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Bounciness " << Bounciness << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "DynamicFriction " << DynamicFriction << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "StaticFriction " << StaticFriction << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void Physics::RigidBody::PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float deltaTime)
{
	if (!parent) parent = container;
	if (!Enabled || deltaTime == 0.0f) return;
	Velocity = Velocity + Gravity * deltaTime;
	if (Velocity.getLength() > TERMINAL_VEL)
	{
		Velocity = Velocity.unitVector() * TERMINAL_VEL;
	}
	container->SetLocalPosition(container->GetLocalPosition() + Velocity * deltaTime);
	container->SetLocalRotation(container->GetLocalRotation() + RotVelocity * deltaTime);
	for (size_t i = 0; i < container->components.size(); i++)
	{
		Colliders::Collider* item = dynamic_cast<Colliders::Collider*>(container->components[i]);
		if (item)
		{
			item->AttachedBody = this;
		}
	}
	container->Refresh();
}