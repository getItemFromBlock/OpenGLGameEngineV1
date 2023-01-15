#include "Resources/ResourceLoader.hpp"

#include <filesystem>
namespace fs = std::filesystem;
#include <string>

#include "Resources/ResourceManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/ModelAnimation.hpp"
#include "Core/Debug/Log.hpp"
#include "Core/Util/ModelLoader.hpp"
#include "Core/Util/AnimatedModelLoader.hpp"
#include "Core/Util/TextHelper.hpp"

void Resources::ResourceLoader::LoadResources(Resources::ResourceManager* manager, Resources::MeshManager* meshes, Resources::MaterialManager* materials, Resources::TextureManager* textures, Resources::ShaderManager* shaders)
{
    std::vector<std::string> files;
    std::vector<std::string> exts;
    std::vector<FragmentShader*> fragments;
    std::vector<VertexShader*> vertexs;
    std::vector<std::string> shadersList;

    Texture::SetFilterType(GL_LINEAR);
    manager->SetPathAutoAppend(true);
    std::string path = "Resources/";
    for (const auto& entry : fs::recursive_directory_iterator(path))
    {
        files.push_back(entry.path().generic_string());
        exts.push_back(entry.path().extension().generic_string());
    }
    for (size_t i = 0; i < files.size(); i++)
    {
        if (!exts[i].c_str()[0]) continue;
        if (!exts[i].compare(".obj"))
        {
            LowRenderer::Model* obj = new LowRenderer::Model();
            obj->LoadMesh(files[i].c_str(), manager, materials, textures, meshes);
            meshes->AddModel(obj, files[i].c_str());
        }
        else if (!exts[i].compare(".mtl"))
        {
            std::vector<Material*> mats;
            Core::Util::LoaderData data = {
                manager,
                materials,
                textures,
                &mats,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            };
            Core::Util::ModelLoader::ParseMTL(files[i].c_str(),&data);
        }
        else if (!exts[i].compare(".smd"))
        {
            LowRenderer::SkinnedModel* obj = new LowRenderer::SkinnedModel();
            obj->LoadMesh(files[i].c_str(), manager, materials, textures, meshes);
            meshes->AddSkinnedModel(obj, files[i].c_str());
            Resources::ModelAnimation* anim = manager->Create<Resources::ModelAnimation>(files[i].c_str());
            meshes->AddAnimation(anim);
        }
        else if (!exts[i].compare(".png") || !exts[i].compare(".jpg"))
        {
            textures->CreateTexture(manager, files[i].c_str());
        }
        else if (!exts[i].compare(".frag"))
        {
            fragments.push_back(manager->Create<Resources::FragmentShader>(files[i].c_str()));
            shaders->AddShader(fragments.back());
        }
        else if (!exts[i].compare(".vert"))
        {
            vertexs.push_back(manager->Create<Resources::VertexShader>(files[i].c_str()));
            shaders->AddShader(vertexs.back());
        }
        else if (!exts[i].compare(".slist"))
        {
            shadersList.push_back(files[i]);
        }
        else
        {
            LOG("%s:%s",files[i].c_str(), exts[i].c_str());
        }
    }
    for (size_t i = 0; i < shadersList.size(); i++)
    {
        int64_t size;
        int64_t line = 1;
        int64_t pos = 0;
        std::string name = "Shader";
        FragmentShader* frag = nullptr;
        VertexShader* vert = nullptr;
        const char* data = Core::Util::ModelLoader::loadFile(shadersList[i].c_str(), size);
        while (pos < size)
        {
            if (Text::compareWord(data, pos, size, "NewShader"))
            {
                pos = Text::skipCharSafe(data, pos, size);
                name = Text::getText(data, pos, size);
                if (!name.c_str())
                {
                    LOG("Error: Expected path after \"NewShader\" at line %llu", line);
                    break;
                }

            }
            else if (Text::compareWord(data, pos, size, "EndShader"))
            {
                if (!vert || !frag)
                {
                    LOG("Error: Unexpected \"EndShader\" at line %llu", line);
                    break;
                }
                shaders->CreateShaderProgram(vert, frag, name.c_str());
                name = "Shader";
            }
            else if (Text::compareWord(data, pos, size, "Fragment"))
            {
                pos = Text::skipCharSafe(data, pos, size);
                std::string fPath = Text::getText(data, pos, size);
                if (!fPath.c_str())
                {
                    LOG("Error: Expected path after \"Fragment\" at line %llu", line);
                    break;
                }
                bool found = false;
                for (size_t i = 0; i < fragments.size(); i++)
                {
                    if (!fPath.compare(fragments[i]->GetPath()))
                    {
                        found = true;
                        frag = fragments[i];
                        break;
                    }
                }
                if (!found)
                {
                    LOG("Error: Invalid shader \"%s\" at line %llu", fPath.c_str(), line);
                    break;
                }
            }
            else if (Text::compareWord(data, pos, size, "Vertex"))
            {
                pos = Text::skipCharSafe(data, pos, size);
                std::string fPath = Text::getText(data, pos, size);
                if (!fPath.c_str())
                {
                    LOG("Error: Expected path after \"Vertex\" at line %llu", line);
                    break;
                }
                bool found = false;
                for (size_t i = 0; i < vertexs.size(); i++)
                {
                    if (!fPath.compare(vertexs[i]->GetPath()))
                    {
                        found = true;
                        vert = vertexs[i];
                        break;
                    }
                }
                if (!found)
                {
                    LOG("Error: Invalid shader \"%s\" at line %llu", fPath.c_str(), line);
                    break;
                }
            }
            pos = Text::endLine(data, pos, size);
            line++;
        }
        delete[] data;
    }
}