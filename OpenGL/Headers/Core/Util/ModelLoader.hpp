#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

#include "Core/Maths/Maths.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"
#include "LowRenderer/Model.hpp"
#include "Resources/MeshManager.hpp"

namespace Core::Util
{
    struct Triangle
    {
        unsigned int vertice[3];
        unsigned int normal[3];
        unsigned int texture[3];

        Triangle(unsigned int v[3], unsigned int n[3], unsigned int t[3]);
    };

    struct VerticeIndex
    {
        unsigned int indexes[3];
    };

    class Vertice
    {
    public:
        Core::Maths::Vec3D pos;
        Core::Maths::Vec3D norm;
        Core::Maths::Vec2D UV;
    };

    class TangentVertice
    {
    public:
        Core::Maths::Vec3D pos;
        Core::Maths::Vec3D norm;
        Core::Maths::Vec2D UV;
        Core::Maths::Vec3D tang;
    };

    struct LoaderData
    {
        Resources::ResourceManager* manager;
        Resources::MaterialManager* materialManager;
        Resources::TextureManager* textureManager;
        std::vector<Resources::Material*>* mats;
        std::vector<std::string>* matIds;
        std::vector<std::vector<Triangle>>* tris;
        std::vector<Core::Maths::Vec3D>* verts;
        std::vector<Core::Maths::Vec3D>* norms;
        std::vector<Core::Maths::Vec2D>* UVs;
        std::vector<Vertice>* verticesOut;
        std::vector<TangentVertice>* tverticesOut;
        std::vector<VerticeIndex>* indexes;
        std::vector<std::vector<unsigned int>>* indexesOut;
    };

    class ModelLoader
    {
    public:
        static const char* loadFile(const char* path, int64_t& length);
        static void ParseMTL(const char* path, LoaderData* args);
        static void LoadMesh(LowRenderer::Model& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager);
    private:
        static char IsDefaultModel(const char* path);
        static signed char Loop(const char* path, const char* data, const int64_t& size, LoaderData* args);
        static signed char GenerateDefault(char Type, LoaderData* args);
        static void MeshLoaderThread(LowRenderer::Model& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager);
        static void ReadFace(int64_t& pos, size_t& objIndex, signed char& type, const char* data, const int64_t& size, LoaderData* args);
        static inline bool CreateTangent = false;
    };
}