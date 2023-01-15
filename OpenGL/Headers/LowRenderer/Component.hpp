#pragma once

#include <stdint.h>
#include <string>
#include <fstream>
#include <vector>

#include "Core/Maths/Maths.hpp"
#include "Core/DataStructure/INameable.hpp"

namespace LowRenderer
{
	namespace Lightning
	{
		class LightManager;
	}
	namespace Rendering
	{
		class RenderCamera;
	}
}
namespace Core::DataStructure
{
	class Node;
}
namespace Resources
{
	class ShaderManager;
	class ShaderProgram;
	class MaterialManager;
	class MeshManager;
	class TextureManager;
	class ResourceManager;
}
namespace Physics::Colliders
{
	class Collider;
}

enum class RenderMode : unsigned char
{
	None = 0u,
	Mainrender = 1u,
	SecondaryRender = 2u,
	NoShadow = 3u,
	Shadow = 4u,
	ShadowMain = 5u,
	ShadowSecond = 6u,
	All = 7u,
	DebugBase = 8u,
	DebugHalo = 16u,
};

namespace LowRenderer
{
	class Component : public Core::DataStructure::INameable
	{
	public:
		virtual void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime) {};
		virtual void PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime) {}
		virtual void OnCollision(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other, Core::Maths::Vec3D HitPoint, Core::Maths::Vec3D Direction) {}
		virtual void OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other) {}
		virtual void PreUpdate() {}
		virtual const char* GetName() override = 0;
		virtual void DeleteComponent() = 0;
		virtual void Create(Core::DataStructure::Node* container) = 0;
		virtual void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err);
		virtual void Serialize(std::ofstream& fileOut, unsigned int rec);
		virtual bool IsEnabled() { return Enabled; }
		virtual void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager);
		virtual void Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode renderMode, bool isSelected) {};
		virtual void RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs) {};

		bool Enabled = true;
	};
}