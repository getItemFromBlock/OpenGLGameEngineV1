#pragma once

#include "Resources/Mesh.hpp"
#include "Core/Maths/Maths.hpp"
#include "LowRenderer/Component.hpp"
#include "Resources/ShaderProgram.hpp"
#include "Resources/Material.hpp"

#include <vector>

namespace Resources
{
    class ModelHolder;
}

namespace Core::Util
{
    class ModelLoader;
}

namespace LowRenderer
{
    class Model : public Component
    {
    private:
        std::vector<Resources::Mesh*> models = std::vector<Resources::Mesh*>();
        std::vector<bool> modelVisible = std::vector<bool>();
        std::vector<Resources::Material> materials = std::vector<Resources::Material>();
        Resources::Texture* currentTexture = nullptr;
        Resources::Texture* currentNormalMap = nullptr;
        Core::Maths::Mat4D modelMat = Core::Maths::Mat4D::Identity();
        static inline char searchText[64] = "";
        static inline unsigned int selectedMeshIndex = UINT32_MAX;
        static inline unsigned int selectedMeshDragIndex = UINT32_MAX;
        static inline bool dragging = false;
        static inline Resources::ShaderProgram* selectedProgram = nullptr;
        static inline Resources::Material* selectedMaterial = nullptr;
        static inline Resources::Mesh* selectedMesh = nullptr;
        static inline Resources::ModelHolder* selectedModel = nullptr;

        void MaterialPopUp(Resources::MaterialManager* materialManager, size_t index);
        void MeshPopUp(Resources::MeshManager* meshManager, size_t index);
        void ShaderPopUp(Resources::ShaderManager* shaderManager);
        void ModelPopUp(Resources::MeshManager* meshManager);
        void DeserializeElements(Resources::ResourceManager* resources, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err);
        void RenderMeshes(Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* meshManager);
    public:
        Resources::ShaderProgram* shaderProgram = nullptr;
        RenderMode renderMode = RenderMode::All;

        Model();
        Model(Resources::ShaderProgram* Shader);
        ~Model();

        void Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode mode, bool isSelected) override;

        void AddMesh(Resources::Mesh* modelIn, Resources::ResourceManager* manager, Resources::Material* modelMaterial);

        void CreateFrom(const Model* other);

        void DeleteComponent() override;

        void Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime) override;

        void Create(Core::DataStructure::Node* container) override;

        void Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err) override;

        void Serialize(std::ofstream& fileOut, unsigned int rec) override;

        void RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager) override;

        const char* GetName() override { return "Model"; }

        int GetNumberOfTriangles();

        void LoadMesh(const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager);

        friend class Core::Util::ModelLoader;
    };
}