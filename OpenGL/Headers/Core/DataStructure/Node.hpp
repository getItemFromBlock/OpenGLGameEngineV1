#pragma once

#include <vector>
#include <fstream>
#include <string>

#include "Physics/Transform.hpp"
#include "Resources/ShaderManager.hpp"
#include "LowRenderer/Component.hpp"
enum class RenderMode : unsigned char;

namespace Physics
{
	class PhysicsHandler;
}

namespace Core::DataStructure
{
	enum class MouseInput : unsigned int
	{
		Left_Down = 0,
		Right_Down,
		Middle_Down,
		Forward_Down,
		Back_Down,
		Left_Press,
		Right_Press,
		Middle_Press,
		Forward_Press,
		Back_Press,
		COUNT,
		ALL_DOWN = 0b0000000000011111,
		ALL_PRESS = 0b0000001111100000,
		ALL = 0b0000001111111111,
	};
	class Node
	{
	public:
		Node();
		Node(Node* parent);
		Node(Node* parent, const char* name);
		~Node();

		char Name[64] = "Object";
		char newName[64];
		void RenderGUI(Node** selectedNode, Node** hoveredNode, bool IsSelected = false, int rec = 0);
		void RenderTransform();
		bool selected = false;
		bool opened = false;

		void SetLocalPosition(const Core::Maths::Vec3D& value);
		void SetLocalRotation(const Core::Maths::Vec3D& value);
		void SetLocalScale(const Core::Maths::Vec3D& value);
		const Core::Maths::Mat4D* GetLocalMatrix() const;
		Core::Maths::Vec3D GetLocalPosition() const;
		Core::Maths::Vec3D GetLocalRotation() const;
		Core::Maths::Vec3D GetLocalScale() const;
		const Core::Maths::Mat4D* GetGlobalMatrix() const;
		void RefreshChilds();
		void Refresh();
		std::vector<Node*> childs;
		std::vector<LowRenderer::Component*> components;
		std::string GetPath();
		void Destroy();
		Node* GetParent() { return parent; }
		void Serialize(std::ofstream& fileOut, unsigned int rec) const;
		friend class Graph;
	protected:
		Node* parent = nullptr;
		Physics::Transform transform;
		void Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode renderMode);
		void Update(std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* manager, float deltaTime);
		void PhysicUpdate(std::vector<Physics::Colliders::Collider*>* colliders, float deltaTime);
		void PreUpdate();
		void RenderGameUI(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs);
		bool NodeSelected = false;
	};

	class SceneNode : public Node
	{
	public:
		SceneNode();
		~SceneNode();

		void RenderScene(Resources::ShaderManager* shaders, LowRenderer::Lightning::LightManager* lights, Resources::TextureManager* textures, Resources::ResourceManager* resources, unsigned int& VAOCurrent,
			Physics::PhysicsHandler* physics, Resources::ShaderProgram** shaderProgramCurrent, bool ortho, float deltaTime, float globalTime);
		void UpdatePhysics(std::vector<Physics::Colliders::Collider*>* colliders, float deltaTime);
	private:
	};
}