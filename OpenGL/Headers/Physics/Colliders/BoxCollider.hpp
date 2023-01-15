#pragma once

#include "Physics/Colliders/Collider.hpp"

namespace Physics::Colliders
{
	class BoxCollider : public Collider
	{
		friend class Physics::PhysicsHandler;
	public:
		BoxCollider();
		~BoxCollider();
		void Create(Core::DataStructure::Node* container);
		virtual const char* GetName() override { return "Box Collider"; }
		void DeleteComponent() override;
		virtual void PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime) override;
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;
		virtual void DrawCollider(Resources::ResourceManager* resources, Resources::ShaderProgram* program, unsigned int& VAOCurrent, const Core::Maths::Mat4D& vp) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
	protected:
		Core::Maths::Vec3D Size = Core::Maths::Vec3D(2, 2, 2);
		Core::Maths::Vec3D Rotation = Core::Maths::Vec3D(0, 0, 0);
		Core::Maths::Mat4D LocalMatrix = Core::Maths::Mat4D();
	};
}