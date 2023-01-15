#pragma once

#include <string>
#include <vector>

#include "Resources/IResource.hpp"
#include "Core/DataStructure/INameable.hpp"
#include "Core/Maths/Maths.hpp"

namespace Core::Util
{
	class AnimatedModelLoader;
}

namespace Resources
{
	struct BoneInfo
	{
		int id = -1;
		Core::Maths::Vec3D Position;
		Core::Maths::Vec3D Rotation;
	};
	class ModelAnimation : public IResource, public Core::DataStructure::INameable
	{
	public:

		ModelAnimation();
		~ModelAnimation();
		void Load(const char* path) override;
		void UnLoad() override;
		const char* GetPath() override;
		const char* GetName() override;
		std::vector<BoneInfo> GetInterpolatedBones(float time, bool looped = true);

		ModelAnimation& operator=(const ModelAnimation& other);
	private:
		char Name[256] = "";
		std::string fullPath = "";
		std::vector<BoneInfo> AnimData;
		unsigned int Frames = 0;
		unsigned int Bones = 0;

		friend class Core::Util::AnimatedModelLoader;
	};
}