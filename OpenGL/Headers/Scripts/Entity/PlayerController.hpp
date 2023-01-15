#pragma once

#include "LowRenderer/Component.hpp"
#include "Scripts/Entity/EntityBase.hpp"
#include "LowRenderer/Rendering/EditorCamera.hpp"
#include "LowRenderer/SkinnedModel.hpp"

namespace Scripts::Entity
{
	class PlayerController : public EntityBase
	{
	public:
		PlayerController();
		~PlayerController() {}

		void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) override;
		void OnCollision(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other, Core::Maths::Vec3D HitPoint, Core::Maths::Vec3D Direction) override;
		void OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other) override;
		void PreUpdate() override;
		const char* GetName() override { return "Player Controller"; }
		void DeleteComponent() override;
		void Create(Core::DataStructure::Node* container) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;

		float Speed = 20.0f;
		float Acceleration = 20.0f;
		float JumpPower = 20.0f;
		float CameraRotY = 0;
		int CameraDistanceIndex = 1;
		Core::Maths::Vec3D CameraDistance = Core::Maths::Vec3D(2.1f, 12.0f, 24.0f);
		Core::Maths::Vec3D Vel = Core::Maths::Vec3D();
		Core::Maths::Vec3D CameraRot = Core::Maths::Vec3D(22.5f, 0, 0);
		Core::Maths::Vec3D CameraRotDelta = Core::Maths::Vec3D(0,0,0);
		bool OnSlope = false;
		LowRenderer::SkinnedModel* Model = nullptr;
		Resources::ModelAnimation* JumpAnim = nullptr;
		Resources::ModelAnimation* WalkAnim = nullptr;
		Resources::ModelAnimation* IdleAnim = nullptr;
		Resources::ModelAnimation* FlexAnim = nullptr;
		float IdleTime = 0.0f;

	private:
		LowRenderer::Rendering::EditorCamera* Camera = nullptr;
		int* Bindings = nullptr;
	};
}