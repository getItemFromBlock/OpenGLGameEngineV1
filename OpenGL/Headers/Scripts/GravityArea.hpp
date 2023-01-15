#pragma once

#include "LowRenderer/Component.hpp"
#include "Core/Maths/Maths.hpp"

namespace Resources
{
	class Mesh;
}

namespace Scripts
{
	enum class FieldType
	{
		Box = 0,
		Sphere = 1,
		InverseSphere = 2,
	};
	class GravityArea : public LowRenderer::Component
	{
	public:
		GravityArea() {};
		~GravityArea() {};

		void Create(Core::DataStructure::Node* container) override;
		void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) override;
		void Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode mode, bool isSelected) override;
		const char* GetName() override { return "Gravity Area"; }
		void DeleteComponent() override;
		void OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other) override;
		void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;
		void Serialize(std::ofstream& fileOut, unsigned int rec) override;
		void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;

		Core::Maths::Vec3D GravityField = Core::Maths::Vec3D(0,-30,0);
		FieldType Type = FieldType::Box;
		float MaxPlane = 1.0f;
	private:
		Core::DataStructure::Node* parent = nullptr;
		Resources::Mesh* SphereMesh = nullptr;
		Core::Maths::Mat4D ModelMatrix = Core::Maths::Mat4D::Identity();
	};
}