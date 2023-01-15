#pragma once

#include "Resources/Mesh.hpp"
#include "Core/Maths/Maths.hpp"
#include "LowRenderer/Component.hpp"
#include "Resources/ShaderProgram.hpp"
#include "Resources/Material.hpp"

#include <vector>

namespace Resources
{
    class SkinnedModelHolder;
    class ModelAnimation;
}

namespace Core::Util
{
    class AnimatedModelLoader;
}

namespace LowRenderer
{
    struct BoneData
    {
        int ParentIndex = -1;
        std::string name;
        Core::Maths::Vec3D defaultPos; // Relative to parent
        Core::Maths::Vec3D defaultRot; // Relative to parent
        Core::Maths::Mat4D BoneMatrix;
        Core::Maths::Mat4D DefaultInvMat;
        Core::DataStructure::Node* Parent = nullptr;
    };
    class SkinnedModel : public Component
    {
    private:
        Resources::ModelAnimation* Anim = nullptr;
        float AnimationTime = 0.0f;
        float AnimationSpeed = 1.0f;
        bool AnimationLooped = true;
        std::vector<Resources::Mesh*> meshes = std::vector<Resources::Mesh*>();
        std::vector<bool> modelVisible = std::vector<bool>();
        std::vector<Resources::Material> materials = std::vector<Resources::Material>();
        std::vector<BoneData> Bones = std::vector<BoneData>();
        Resources::Texture* currentTexture = nullptr;
        Resources::Texture* currentNormalMap = nullptr;
        Core::DataStructure::Node* ParentNode = nullptr;
        static inline char searchText[64] = "";
        static inline unsigned int selectedMeshIndex = UINT32_MAX;
        static inline unsigned int selectedMeshDragIndex = UINT32_MAX;
        static inline bool dragging = false;
        static inline Resources::ShaderProgram* selectedProgram = nullptr;
        static inline Resources::Material* selectedMaterial = nullptr;
        static inline Resources::Mesh* selectedMesh = nullptr;
        static inline Resources::SkinnedModelHolder* selectedModel = nullptr;
        static inline Resources::ModelAnimation* selectedAnim = nullptr;

        void DetectBones();
        Core::DataStructure::Node* FindChildInNode(Core::DataStructure::Node* node, std::string name);
        void MaterialPopUp(Resources::MaterialManager* materialManager, size_t index);
        void MeshPopUp(Resources::MeshManager* meshManager, size_t index);
        void ShaderPopUp(Resources::ShaderManager* shaderManager);
        void ModelPopUp(Resources::MeshManager* meshManager);
        void AnimPopUp(Resources::MeshManager* meshManager);
        void DeserializeElements(Resources::ResourceManager* resources, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err);
        void RenderMeshes(Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* meshManager);
        void CalculateDefaultMats();
        void RefreshAnim();
    public:
        Resources::ShaderProgram* shaderProgram = nullptr;
        RenderMode renderMode = RenderMode::All;

        SkinnedModel();
        SkinnedModel(Resources::ShaderProgram* Shader);
        ~SkinnedModel();

        void Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode mode, bool isSelected) override;

        void AddMesh(Resources::Mesh* modelIn, Resources::ResourceManager* manager, Resources::Material* modelMaterial);

        void CreateFrom(Core::DataStructure::Node* parent, const SkinnedModel* other);

        void DeleteComponent() override;

        void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime) override;

        void Create(Core::DataStructure::Node* container) override;

        void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;

        void Serialize(std::ofstream& fileOut, unsigned int rec) override;

        void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;

        const char* GetName() override { return "Skinned Model"; }

        int GetNumberOfTriangles();

        void LoadMesh(const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager);
        void SetAnimation(Resources::ModelAnimation* NewAnim, float AnimSpeed = 1.0f, bool looped = true);
        Resources::ModelAnimation* GetAnimation();
        void SetAnimSpeed(float NewSpeed) { AnimationSpeed = NewSpeed; }

        friend class Core::Util::AnimatedModelLoader;
    };
}