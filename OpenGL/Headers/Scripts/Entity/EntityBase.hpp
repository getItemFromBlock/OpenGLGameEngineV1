#pragma once

#include "LowRenderer/Component.hpp"
#include "Physics/RigidBody.hpp"

namespace Scripts::Entity
{
	class EntityBase : public LowRenderer::Component
	{
	public:
		EntityBase() {}
		~EntityBase() {}

		virtual void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime);
		virtual void OnCollision(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other, Core::Maths::Vec3D HitPoint, Core::Maths::Vec3D Direction);
		virtual void PreUpdate();
		virtual void DeleteComponent();
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err);
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec);
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager);

		Core::Maths::Vec3D CurrentUp = Core::Maths::Vec3D(0, 1, 0);
		Core::Maths::Vec3D CurrentFront = Core::Maths::Vec3D(0, 0, 1);
		Core::Maths::Mat4D CurrentMat = Core::Maths::Mat4D::Identity();
		Physics::RigidBody* body = nullptr;
		bool OnGround = false;
	protected:
		Core::DataStructure::Node* ModelNode = nullptr;
		Core::Maths::Vec3D MovementDir = Core::Maths::Vec3D();
	};
}