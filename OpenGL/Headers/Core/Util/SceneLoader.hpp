#pragma once

#include <string>

namespace Core::DataStructure
{
	class SceneNode;
	class Node;
	class Graph;
}
namespace Resources
{
	class ResourceManager;
	class ShaderManager;
}

namespace Physics
{
	class PhysicsHandler;
}

namespace Core::Util::SceneLoader
{
	bool SaveScene(const char* path, const Core::DataStructure::SceneNode* scene, Physics::PhysicsHandler* handler, Core::DataStructure::Graph* provider);
	bool LoadScene(const char* path, Core::DataStructure::Node* scene, Core::DataStructure::Graph* provider, Physics::PhysicsHandler* handler, Resources::ResourceManager* resources, Resources::ShaderManager* shaders);
	bool CreateComponentType(std::string type, Core::DataStructure::Graph* provider, Core::DataStructure::Node* parent);
}