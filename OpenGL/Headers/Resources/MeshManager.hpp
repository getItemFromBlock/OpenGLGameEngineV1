#pragma once

#include <vector>
#include "Resources/ResourceManager.hpp"
#include "Core/DataStructure/INameable.hpp"

namespace Resources
{
	class Mesh;
	class ModelAnimation;
}
namespace LowRenderer
{
	class Model;
	class SkinnedModel;
}

namespace Resources
{
	class ModelHolder : public Core::DataStructure::INameable
	{
	public:
		ModelHolder();
		ModelHolder(LowRenderer::Model* in, const char* path);
		~ModelHolder();
		const char* GetName() override;
		LowRenderer::Model* model = nullptr;
	private:
		std::string fullPath = "";
	};

	class SkinnedModelHolder : public Core::DataStructure::INameable
	{
	public:
		SkinnedModelHolder();
		SkinnedModelHolder(LowRenderer::SkinnedModel* in, const char* path);
		~SkinnedModelHolder();
		const char* GetName() override;
		LowRenderer::SkinnedModel* model = nullptr;
	private:
		std::string fullPath = "";
	};

	class MeshManager
	{
	public:
		MeshManager();
		~MeshManager();
		size_t AddMesh(Mesh* in);
		std::vector<Mesh*> GetMeshs();
		std::vector<Mesh*> GetMeshs(const char* filter);
		size_t AddModel(LowRenderer::Model* in, const char* path);
		std::vector<ModelHolder*> GetModels();
		std::vector<ModelHolder*> GetModels(const char* filter);
		size_t AddSkinnedModel(LowRenderer::SkinnedModel* in, const char* path);
		std::vector<SkinnedModelHolder*> GetSkinnedModels();
		std::vector<SkinnedModelHolder*> GetSkinnedModels(const char* filter);
		size_t AddAnimation(ModelAnimation* in);
		std::vector<ModelAnimation*> GetAnimations();
		std::vector<ModelAnimation*> GetAnimations(const char* filter);
		void ClearModels();
	private:
		std::vector<Mesh*> meshs;
		std::vector<ModelHolder*> models;
		std::vector<SkinnedModelHolder*> skinnedModels;
		std::vector<ModelAnimation*> anims;
	};
}