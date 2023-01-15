#include "Resources/Mesh.hpp"

#include <glad/glad.h>

Resources::Mesh::Mesh(){}

Resources::Mesh::~Mesh(){}

void Resources::Mesh::Draw(unsigned int& VAOCurrent)
{
    if (!loaded)
        return;

    if (VAOCurrent != VAO)
    {
        glBindVertexArray(VAO);
        VAOCurrent = VAO;
    }
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void Resources::Mesh::Load(const char* path)
{
    fullPath = path;
    int startIndex = 0;
    char tmp;
    for (int i = 0; i < 63; i++)
    {
        tmp = path[i];
        if (tmp == 0)
        {
            break;
        }
        else if (tmp == '\\' || tmp == '/')
        {
            startIndex = i + 1;
        }
    }
    tmp = path[startIndex];
    int index = 0;
    bool ext = false;
    for (int i = startIndex + 1; i < 255 && tmp != 0; i++)
    {
        if (tmp == '.') ext = true;
        if (!ext)
        {
            Name[index] = tmp;
            index++;
        }
        else if (tmp == ':')
        {
            ext = false;
            Name[index] = tmp;
            index++;
        }
        tmp = path[i];
    }
    Name[index] = 0;
}

void Resources::Mesh::UnLoad()
{
    if (!loaded) return;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    indexCount = 0;
    loaded = false;
    this->~Mesh();
}

const char* Resources::Mesh::GetPath()
{
    return fullPath.c_str();
}

Resources::Mesh& Resources::Mesh::operator=(const Mesh& other)
{
    if (this == &other)
        return *this;
    for (unsigned int i = 0; i < 64; i++)
    {
        Name[i] = other.Name[i];
    }
    fullPath = other.fullPath;
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    indexCount = other.indexCount;
    loaded = other.loaded;
    return *this;
}