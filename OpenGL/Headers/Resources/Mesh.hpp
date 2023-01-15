#pragma once

#include <string>
#include <vector>

#include "Resources/IResource.hpp"
#include "Core/DataStructure/INameable.hpp"

namespace Core::Util
{
    class ModelLoader;
    class AnimatedModelLoader;
}

namespace Resources
{
    class Mesh : public IResource, public Core::DataStructure::INameable
    {
    private:
        bool loaded = true;

        unsigned int indexCount = 0;

        char Name[64] = "Mesh";

        std::string fullPath = "";

        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;

    public:
        Mesh();
        ~Mesh();

        void Draw(unsigned int& VAOCurrent);

        void Load(const char* path) override;
        void UnLoad() override;
        const char* GetPath() override;
        const char* GetName() override { return Name; }
        unsigned int GetVerticesCount() { return indexCount; }
        Mesh& operator=(const Mesh& other);

        friend class Core::Util::ModelLoader;
        friend class Core::Util::AnimatedModelLoader;
    };
}