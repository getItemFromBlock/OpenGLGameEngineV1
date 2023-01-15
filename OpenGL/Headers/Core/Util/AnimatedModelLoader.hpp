#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

#include "Core/Maths/Maths.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"
#include "LowRenderer/SkinnedModel.hpp"
#include "Resources/MeshManager.hpp"

namespace Resources
{
    struct BoneInfo;
    class ModelAnimation;
}

namespace Core::Util
{
    struct Bone
    {
        int id = -1;
        int parent = -1;
        std::string name;
        Core::Maths::Vec3D defaultPos; // Relative to parent
        Core::Maths::Vec3D defaultRot; // Relative to parent
        Core::Maths::Mat4D defaultMat;
        Core::Maths::Mat4D invMat;
    };

    class AttachedVertice
    {
    public:
        Core::Maths::Vec3D pos;
        Core::Maths::Vec3D norm;
        Core::Maths::Vec2D UV;
        Core::Maths::Vec3D tang;
        int BoneIDs[6];
        float Weights[6];
    };

    struct ALoaderData
    {
        Resources::ResourceManager* manager;
        Resources::MaterialManager* materialManager;
        Resources::TextureManager* textureManager;
        std::vector<Resources::Material*>* mats;
        std::vector<std::string>* matIds;
        std::vector<Bone>* bones;
        std::vector<AttachedVertice>* verticesOut;
        std::vector<std::vector<unsigned int>>* indexesOut;
    };

    class AnimatedModelLoader
    {
    public:
        static void LoadAnimatedMesh(LowRenderer::SkinnedModel& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager);
        static void LoadAnimation(Resources::ModelAnimation& in, const char* path);
    private:
        static bool Loop(const char* path, const char* data, const int64_t& size, ALoaderData* args);
        static bool AnimLoop(const char* path, const char* data, const int64_t& size, Resources::ModelAnimation& in);
        static void MeshLoaderThread(LowRenderer::SkinnedModel& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager);
        static void AnimLoaderThread(Resources::ModelAnimation& in, const char* path);
        static void CalculateDefaultMats(std::vector<Bone>* bones);
        static Core::Maths::Vec3D AdaptRot(Core::Maths::Vec3D in);
    };
}