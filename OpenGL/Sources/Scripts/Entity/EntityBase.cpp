#include "Scripts/Entity/EntityBase.hpp"

#include <ImGUI/imgui.h>

#include "Core/Debug/Log.hpp"
#include "Core/Util/TextHelper.hpp"

#include "Core/DataStructure/Node.hpp"
#include "LowRenderer/SkinnedModel.hpp"

void Scripts::Entity::EntityBase::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (DeltaTime == 0.0f || !Enabled) return;
	if (!body)
	{
		for (size_t i = 0; i < container->components.size(); i++)
		{
			Physics::RigidBody* tmp = dynamic_cast<Physics::RigidBody*>(container->components[i]);
			if (tmp)
			{
				body = tmp;
				break;
			}
		}
		if (!body)
		{
			LOG("Error, RigidBody needed for EntityBase at %s", container->GetPath().append(":EntityBase").c_str());
			return;
		}
	}
	if (!ModelNode)
	{
		for (size_t n = 0; n < container->childs.size(); n++)
		{
			for (size_t i = 0; i < container->childs[n]->components.size(); i++)
			{
				LowRenderer::SkinnedModel* tmp = dynamic_cast<LowRenderer::SkinnedModel*>(container->childs[n]->components[i]);
				if (tmp)
				{
					ModelNode = container->childs[n];
					break;
				}
			}
		}
		if (!ModelNode)
		{
			LOG("Error, Child node with Model needed for EntityBase at %s", container->GetPath().append(":EntityBase").c_str());
			return;
		}
	}
	body->RotVelocity = Core::Maths::Vec3D();
	if (Core::Maths::Vec3D(MovementDir.x, 0, MovementDir.z).lengthSquared() > 0.0f)
	{
		ModelNode->SetLocalRotation(Core::Maths::Vec3D(0,Core::Maths::Vec2D(-MovementDir.x, MovementDir.z).GetAngle() - 90.0f,0));
		ModelNode->Refresh();
	}

	if (CurrentFront.lengthSquared() == 0) CurrentFront = Core::Maths::Vec3D(0, 0, 1);
	CurrentFront = CurrentFront.unitVector();
	Core::Maths::Vec3D Right = CurrentUp.crossProduct(CurrentFront);
	if (Right.lengthSquared() == 0) Right = Core::Maths::Vec3D(-CurrentFront.y, CurrentFront.z, CurrentFront.x);
	Right = Right.unitVector();
	CurrentFront = Right.crossProduct(CurrentUp).unitVector();

	Core::Maths::Vec3D z = CurrentFront;
	Core::Maths::Vec3D x = CurrentUp.crossProduct(z);
	Core::Maths::Vec3D y = z.crossProduct(x);
	CurrentMat = Core::Maths::Mat4D();
	Core::Maths::Vec3D delta = Core::Maths::Vec3D();
	for (int i = 0; i < 3; i++)
	{
		CurrentMat.at(i, 0) = x[i];
		CurrentMat.at(i, 1) = y[i];
		CurrentMat.at(i, 2) = z[i];
		CurrentMat.at(3, i) = delta[i];
	}
	CurrentMat.at(3, 3) = 1;
	//Core::Maths::Vec3D targetRot = temp.GetRotationFromTranslation(Core::Maths::Vec3D(1));
	Core::Maths::Mat4D Inv = CurrentMat.CreateInverseMatrix();
	Core::Maths::Vec3D CurrentRot = Inv.GetRotationFromTranslation(Inv.GetScaleFromTranslation());
	//Core::Maths::Vec3D grav = (CurrentMat.CreateInverseMatrix() * body->Gravity).getVector();
	//float factor = Core::Maths::Util::cut(2 * DeltaTime,0,1);
	//CurrentRot = CurrentRot * (1 - factor) + targetRot * factor;
	//CurrentMat = Core::Maths::Mat4D::CreateRotationMatrix(CurrentRot);
	//body->Gravity = (CurrentMat * grav).getVector();
	container->SetLocalRotation(CurrentRot);
	container->Refresh();

}

void Scripts::Entity::EntityBase::OnCollision(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other, Core::Maths::Vec3D HitPoint, Core::Maths::Vec3D Direction)
{
	if (HitPoint.y < -0.3f) OnGround = true;
}

void Scripts::Entity::EntityBase::PreUpdate()
{
	OnGround = false;
}

void Scripts::Entity::EntityBase::DeleteComponent()
{
	this->~EntityBase();
}

void Scripts::Entity::EntityBase::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "CurrentUp"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, CurrentUp))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "CurrentFront"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, CurrentFront))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "OnGround"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, OnGround))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Scripts::Entity::EntityBase::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "CurrentUp " << CurrentUp.x << " " << CurrentUp.y << " " << CurrentUp.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "CurrentFront " << CurrentFront.x << " " << CurrentFront.y << " " << CurrentFront.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "OnGround " << (OnGround ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void Scripts::Entity::EntityBase::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	LowRenderer::Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Up Vector", &CurrentUp.x, 0.1f);
	ImGui::DragFloat3("Front Vector", &CurrentFront.x, 0.1f);
	ImGui::Checkbox("On Ground", &OnGround);
}
