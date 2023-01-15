#pragma once

#include "LowRenderer/Component.hpp"
#include "Core/Maths/Maths.hpp"

namespace Physics
{
	class PhysicsHandler;
	class RigidBody : public LowRenderer::Component
	{
		friend class PhysicsHandler;
	public:
		RigidBody();
		~RigidBody();

		void PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float deltaTime) override;
		void Create(Core::DataStructure::Node* container) override;
		const char* GetName() override { return "RigidBody"; }
		void DeleteComponent() override;
		void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		Core::DataStructure::Node* GetParent() { return parent; }

		Core::Maths::Vec3D Gravity = Core::Maths::Vec3D(0,-2.0f,0);
		float Mass = 1.0f;
		float Bounciness = 0.5f;
		float DynamicFriction = 0.5f;
		float StaticFriction = 0.03f;

		Core::Maths::Vec3D Velocity = Core::Maths::Vec3D();
		Core::Maths::Vec3D RotVelocity = Core::Maths::Vec3D();
	private:
		Core::DataStructure::Node* parent = nullptr;
	};
}