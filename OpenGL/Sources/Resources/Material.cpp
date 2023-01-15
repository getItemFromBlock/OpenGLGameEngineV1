#include "Resources/Material.hpp"

#include <ImGUI/imgui.h>
#include "Core/Util/TextHelper.hpp"

char Resources::Material::searchText[64] = "";
Resources::Texture* Resources::Material::selectedTexture = nullptr;

Resources::Material::Material()
{
}

void Resources::Material::Load(const char* name)
{
    fullPath = name;
    int startIndex = 0;
    char tmp;
    for (int i = 0; i < 255; i++)
    {
        tmp = name[i];
        if (tmp == 0)
        {
            break;
        }
        else if (tmp == '\\' || tmp == '/')
        {
            startIndex = i + 1;
        }
    }
    tmp = name[startIndex];
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
        tmp = name[i];
    }
    Name[index] = 0;
}

void Resources::Material::UnLoad()
{
    if (!fullPath.empty())
        this->~Material();
}

void Resources::Material::Update(Resources::ResourceManager* manager)
{
    if (tmpTexturePath.c_str()[0])
    {
        deltaF++;
        Texture* tmp = manager->Get<Texture>(tmpTexturePath.c_str());
        if (tmp)
        {
            texture = tmp;
            tmpTexturePath = "";
        }
        else
        {
            if (deltaF > 8)
            {
                LOG("Warning, could not find texture %s", tmpTexturePath.c_str());
                tmpTexturePath = "";
            }
        }
    }
}

const char* Resources::Material::GetPath()
{
    return fullPath.c_str();
}

Resources::Material::~Material()
{
}

const char* Resources::Material::GetName()
{
	return Name;
}

void Resources::Material::SetTexture(Resources::ResourceManager* manager, Resources::TextureManager* textures, const char* path, bool raw)
{
    manager->SetPathAutoAppend(raw);
    size_t index = textures->CreateTexture(manager, path);
    texture = textures->GetTextures().at(index);
}

void Resources::Material::SetTexture(Resources::Texture* tex)
{
    texture = tex;
}

void Resources::Material::SetNormalMap(Resources::ResourceManager* manager, Resources::TextureManager* textures, const char* path, bool raw)
{
    manager->SetPathAutoAppend(raw);
    size_t index = textures->CreateTexture(manager, path);
    normalMap = textures->GetTextures().at(index);
}

void Resources::Material::SetNormalMap(Resources::Texture* tex)
{
    normalMap = tex;
}

void Resources::Material::RenderGUI(Resources::TextureManager* textureManager)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::ColorEdit3("Material Ambient", &AmbientColor.x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::ColorEdit3("Material Diffuse", &DiffuseColor.x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::ColorEdit3("Material Specular", &SpecularColor.x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::ColorEdit3("Material Emissive", &EmissiveColor.x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::DragFloat("Alpha", &Alpha, 0.01f, 0.0f, 1.0f, "%.3f");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::DragFloat("Smoothness", &Smoothness, 1.0f, 0.0f, 0.0f, "%.1f");
    if (Smoothness <= 0.0f) Smoothness = 0.0001f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::DragFloat("Shininess", &Shininess, 0.1f, 0.0f, 0.0f, "%.2f");
    if (Shininess < 0.0f) Shininess = 0.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    ImGui::DragFloat("Absorbtion", &Absorbtion, 0.05f, 0.0f, 1.0f, "%.3f");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    if (texture)
    {
        ImGui::Image((ImTextureID)(size_t)texture->GetTextureID(), ImVec2(32, 32));
        ImGui::SameLine();
        ImGui::Text(texture->GetName());
    }
    else
    {
        ImGui::Image((ImTextureID)1llu, ImVec2(32, 32));
        ImGui::SameLine();
        ImGui::Text("Texture");
    }
    ImGui::SameLine();
    ImGui::Button("Change Texture");
    if (ImGui::IsItemClicked())
    {
        SetSearchData(texture);
        ImGui::OpenPopup("Texture Selection");
        ImGui::SetNextWindowSize(ImVec2(700, 400));
    }
    if (ImGui::BeginPopupModal("Texture Selection")) TexturePopUp(textureManager, false);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 45);
    if (normalMap)
    {
        ImGui::Image((ImTextureID)(size_t)normalMap->GetTextureID(), ImVec2(32, 32));
        ImGui::SameLine();
        ImGui::Text(normalMap->GetName());
    }
    else
    {
        ImGui::Image((ImTextureID)1llu, ImVec2(32, 32));
        ImGui::SameLine();
        ImGui::Text("Normal Map");
    }
    ImGui::SameLine();
    ImGui::Button("Change Normal Map");
    if (ImGui::IsItemClicked())
    {
        SetSearchData(normalMap);
        ImGui::OpenPopup("Normal Map Selection");
        ImGui::SetNextWindowSize(ImVec2(700, 400));
    }
    if (ImGui::BeginPopupModal("Normal Map Selection")) TexturePopUp(textureManager, true);
}

void Resources::Material::Deserialize(Resources::ResourceManager* resources, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
    if (err.c_str()[0]) return;
    while (pos < size)
    {
        if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndMaterial"))
        {
            break;
        }
        else if (Text::compareWord(data, pos, size, "AmbientColor"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            if (!Parsing::ReadVec3D(data, pos, size, AmbientColor))
            {
                err = "Malformated Vec3";
                break;
            }
        }
        else if (Text::compareWord(data, pos, size, "DiffuseColor"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            if (!Parsing::ReadVec3D(data, pos, size, DiffuseColor))
            {
                err = "Malformated Vec3";
                break;
            }
        }
        else if (Text::compareWord(data, pos, size, "SpecularColor"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            if (!Parsing::ReadVec3D(data, pos, size, SpecularColor))
            {
                err = "Malformated Vec3";
                break;
            }
        }
        else if (Text::compareWord(data, pos, size, "EmissiveColor"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            if (!Parsing::ReadVec3D(data, pos, size, EmissiveColor))
            {
                err = "Malformated Vec3";
                break;
            }
        }
        else if (Text::compareWord(data, pos, size, "Alpha"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            Alpha = Text::getFloat(data, pos, size);
        }
        else if (Text::compareWord(data, pos, size, "Smoothness"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            Smoothness = Text::getFloat(data, pos, size);
        }
        else if (Text::compareWord(data, pos, size, "Shininess"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            Shininess = Text::getFloat(data, pos, size);
        }
        else if (Text::compareWord(data, pos, size, "Absorbtion"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            Absorbtion = Text::getFloat(data, pos, size);
        }
        else if (Text::compareWord(data, pos, size, "Texture"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            std::string path = Text::getText(data, pos, size);
            if (!path.c_str())
            {
                err = "Expected path after \"Texture\"";
                break;
            }
            texture = resources->Get<Texture>(path.c_str());
            if (!texture)
            {
                tmpTexturePath = path;
                texture = resources->Get<Texture>("DefaultResources/Textures/Blank.png");
            }
        }
        else if (Text::compareWord(data, pos, size, "NormalMap"))
        {
            pos = Text::skipCharSafe(data, pos, size);
            std::string path = Text::getText(data, pos, size);
            if (!path.c_str())
            {
                err = "Expected path after \"NormalMap\"";
                break;
            }
            normalMap = resources->Get<Texture>(path.c_str());
            if (!normalMap)
            {
                texture = resources->Get<Texture>("DefaultResources/Textures/normal.png");
                LOG("Warning, could not find normal map %s", path.c_str());
            }
        }
        pos = Text::endLine(data, pos, size);
        line++;
    }
}

void Resources::Material::Serialize(std::ofstream& fileOut, unsigned int rec)
{
    Parsing::Shift(fileOut, rec);
    fileOut << "NewMaterial " << fullPath << std::endl;
    Parsing::Shift(fileOut, rec+1);
    fileOut << "AmbientColor " << AmbientColor.x << " " << AmbientColor.y << " " << AmbientColor.z << std::endl;
    Parsing::Shift(fileOut, rec+1);
    fileOut << "DiffuseColor " << DiffuseColor.x << " " << DiffuseColor.y << " " << DiffuseColor.z << std::endl;
    Parsing::Shift(fileOut, rec+1);
    fileOut << "SpecularColor " << SpecularColor.x << " " << SpecularColor.y << " " << SpecularColor.z << std::endl;
    Parsing::Shift(fileOut, rec+1);
    fileOut << "EmissiveColor " << EmissiveColor.x << " " << EmissiveColor.y << " " << EmissiveColor.z << std::endl;
    Parsing::Shift(fileOut, rec + 1);
    fileOut << "Alpha " << Alpha << std::endl;
    Parsing::Shift(fileOut, rec + 1);
    fileOut << "Smoothness " << Smoothness << std::endl;
    Parsing::Shift(fileOut, rec + 1);
    fileOut << "Shininess " << Shininess << std::endl;
    Parsing::Shift(fileOut, rec + 1);
    fileOut << "Absorbtion " << Absorbtion << std::endl;
    Parsing::Shift(fileOut, rec + 1);
    fileOut << "AmbientColor " << AmbientColor.x << " " << AmbientColor.y << " " << AmbientColor.z << std::endl;
    if (texture)
    {
        Parsing::Shift(fileOut, rec + 1);
        fileOut << "Texture " << texture->GetPath() << std::endl;
    }
    if (normalMap)
    {
        Parsing::Shift(fileOut, rec + 1);
        fileOut << "NormalMap " << normalMap->GetPath() << std::endl;
    }
    Parsing::Shift(fileOut, rec);
    fileOut << "EndMaterial" << std::endl;
}

void Resources::Material::TexturePopUp(Resources::TextureManager* textureManager, bool normal)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::InputText("Search", searchText, 64);
    std::vector<Texture*> searchResult = textureManager->GetTextures(searchText);
    if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
    {
        if (normal)
        {
            if (normalMap == selectedTexture && searchResult.size() > 0) selectedTexture = searchResult[0];
            normalMap = selectedTexture;
        }
        else
        {
            if (texture == selectedTexture && searchResult.size() > 0) selectedTexture = searchResult[0];
            texture = selectedTexture;
        }
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel") || ImGui::IsKeyDown(ImGuiKey_Escape))
    {
        ImGui::CloseCurrentPopup();
    }
    for (unsigned int j = 0; j < searchResult.size(); j++)
    {
        ImGui::Image((ImTextureID)(size_t)searchResult[j]->GetTextureID(), ImVec2(32, 32));
        bool clicked = ImGui::IsItemClicked();
        ImGui::SameLine();
        if (ImGui::Selectable(searchResult[j]->GetName(), selectedTexture == searchResult[j], ImGuiSelectableFlags_DontClosePopups) || clicked)
        {
            selectedTexture = searchResult[j];
        }
    }
    ImGui::PopStyleColor();
    ImGui::EndPopup();
}

void Resources::Material::SetSearchData(Texture* searchIndex)
{
    selectedTexture = searchIndex;
    searchText[0] = 0;
}

Resources::Material& Resources::Material::operator=(const Resources::Material& other)
{
    if (this == &other)
        return *this;
    for (unsigned int i = 0; i < 256; i++)
    {
        Name[i] = other.Name[i];
    }
    fullPath = other.fullPath;
    texture = other.texture;
    normalMap = other.normalMap;
    AmbientColor = other.AmbientColor;
    DiffuseColor = other.DiffuseColor;
    SpecularColor = other.SpecularColor;
    EmissiveColor = other.EmissiveColor;
    Alpha = other.Alpha;
    Smoothness = other.Smoothness;
    Shininess = other.Shininess;
    Absorbtion = other.Absorbtion;
    return *this;
}