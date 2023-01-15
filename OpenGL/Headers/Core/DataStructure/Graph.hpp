#pragma once

#include "Core/DataStructure/Node.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "LowRenderer/Component.hpp"

class Resources::TextureManager;

namespace Physics
{
	class PhysicsHandler;
}

namespace Core::DataStructure
{
	class Graph
	{
	public:
		Graph();
		~Graph();

		void RenderGUI(Resources::ResourceManager* resources, Physics::PhysicsHandler* handler, Resources::ShaderManager* shaderManager,
			Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager, SceneNode* node);
		void EmptySelected() { selected = nullptr; selectedComponent = 0; }
		void AddComponent(LowRenderer::Component* in);
		const std::vector<LowRenderer::Component*> GetComponents();
		void DeleteComponents();
	private:
		Core::DataStructure::Node* selected = nullptr;
		Core::DataStructure::Node* hovered = nullptr;
		unsigned int selectedComponent = UINT32_MAX;
		unsigned int selectedComponentDragIndex = UINT32_MAX;
		std::vector<LowRenderer::Component*> components;
		static char searchText[64];
		static LowRenderer::Component* selectedElement;
		bool dragging = false;
		bool NodeDragging = false;
		void RenderInspector(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager, SceneNode* node);
		void SwapNodes(Core::DataStructure::Node* a, Core::DataStructure::Node* b);
		void HandleFocusActions(Core::DataStructure::SceneNode* node, Physics::PhysicsHandler* handler, Resources::ResourceManager* resources, Resources::ShaderManager* shaders);
		void CopyNode();
		void PasteNode(Resources::ResourceManager* resources, Resources::ShaderManager* shaders);
		void DeleteSelectedNode(Core::DataStructure::SceneNode* node);
		void ComponentPopUp();
	};
}