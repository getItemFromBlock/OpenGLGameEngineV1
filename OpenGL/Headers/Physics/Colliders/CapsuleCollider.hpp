#pragma once

#include "Physics/Colliders/Collider.hpp"

namespace Physics::Colliders
{
	class CapsuleCollider : public Collider
	{
		friend class Physics::PhysicsHandler;
	public:
		CapsuleCollider();
		~CapsuleCollider();
		void Create(Core::DataStructure::Node* container);
		virtual const char* GetName() override { return "Capsule Collider"; }
		void DeleteComponent() override;
		virtual void PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime) override;
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void DrawCollider(Resources::ResourceManager* resources, Resources::ShaderProgram* program, unsigned int& VAOCurrent, const Core::Maths::Mat4D& vp) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
	protected:
		Core::Maths::Mat4D ModelMatrixUp = Core::Maths::Mat4D();
		Core::Maths::Mat4D ModelMatrixDown = Core::Maths::Mat4D();
		Core::Maths::Vec3D Rotation = Core::Maths::Vec3D(0, 0, 0);
		float Length = 2;
		float Rayon = 1;
	};
}