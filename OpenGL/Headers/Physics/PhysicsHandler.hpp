#pragma once

#include <vector>
#include <string>
#include "Core/Maths/Maths.hpp"

namespace Core::DataStructure
{
	class SceneNode;
	class Node;
}

namespace Core::App
{
	class App;
}

namespace Physics
{
	namespace Colliders
	{
		class Collider;
	}

	class PhysicsHandler
	{
	friend class Colliders::Collider;
	public:
		PhysicsHandler();
		~PhysicsHandler();

		void UpdatePhysics(Core::DataStructure::SceneNode* scene, bool PlayMode, float deltaTime);
		std::vector<Colliders::Collider*>* GetColliders() { return &colliders; }

		void RenderGUI();
		void Serialize(std::ofstream& fileOut);
		void Deserialize(const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err);
	private:
		bool CollideObjects(Colliders::Collider* a, Colliders::Collider* b, float deltaTime);
		std::vector<Colliders::Collider*> colliders;
		bool HandleSphereSphere(Colliders::Collider* a, Colliders::Collider* b, float deltaTime);
		bool HandleSphereCube(Colliders::Collider* a, Colliders::Collider* b, float deltaTime);
		Core::Maths::Mat4D GetLocalMatrix(Core::DataStructure::Node* parent);
		void ApplyReaction(Colliders::Collider* a, Colliders::Collider* other, Core::Maths::Vec3D HitPoint, Core::Maths::Vec3D Dir, Core::Maths::Mat4D ParentMat, float deltaTime);
		void NotifyTriggers(Colliders::Collider* a, Colliders::Collider* b);
		void ApplyReactionDouble(Colliders::Collider* a, Colliders::Collider* b, Core::Maths::Vec3D HitPointA, Core::Maths::Vec3D HitPointB, Core::Maths::Vec3D DirA, Core::Maths::Vec3D DirB, Core::Maths::Mat4D ParentMatA, Core::Maths::Mat4D ParentMatB, float deltaTime);
		std::string CollisionLayerNames[32];
	};
}