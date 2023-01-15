#pragma once

#include "LowRenderer/Component.hpp"
#include "Core/Maths/Maths.hpp"

namespace Resources
{
	class ShaderProgram;
}

namespace Physics
{
	class RigidBody;
	class PhysicsHandler;
	namespace Colliders
	{
		enum class ColliderType : unsigned char
		{
			None = 0,
			Ball = 1,
			Box = 2,
			Capsule = 3,
		};

		class Collider : public LowRenderer::Component
		{
			friend class Physics::RigidBody;
			friend class Physics::PhysicsHandler;
		public:
			Collider() {}
			~Collider() {}

			virtual void PreUpdate() override;
			virtual void PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime);
			void Create(Core::DataStructure::Node* container) = 0;
			virtual const char* GetName() override = 0;
			virtual void DrawCollider(Resources::ResourceManager* resources, Resources::ShaderProgram* program, unsigned int& VAOCurrent, const Core::Maths::Mat4D& vp) = 0;
			virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
			void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
			void Serialize(std::ofstream& fileOut, unsigned int rec) override;

			static inline void SetPhysicHandler(PhysicsHandler* in) { Handler = in; }

			bool IsTrigger = false;
			unsigned int LayerMask = 0xffffffff;
			Physics::RigidBody* AttachedBody = nullptr;
		protected:
			void LayerPopUp();

			bool Entered = false;
			bool NotifyChilds = true;
			Core::DataStructure::Node* parent = nullptr;
			ColliderType Type = ColliderType::None;
			Core::Maths::Mat4D ModelMatrix = Core::Maths::Mat4D();
			Core::Maths::Vec3D Position = Core::Maths::Vec3D(0, 0, 0);
			float Range = 1.0f;
			static inline PhysicsHandler* Handler;
		};
	}
}