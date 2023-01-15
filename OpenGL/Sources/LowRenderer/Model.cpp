#include "LowRenderer/Model.hpp"

#include <glad/glad.h>
#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "LowRenderer/Lightning/LightManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Core/Util/ModelLoader.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

const char* const RenderModeDesc[8] =
{
	"None",
	"Main Render only",
	"Secondary Render only",
	"No Shadow",
	"Shadow only",
	"Shadow and Main",
	"Shadow and Second",
	"All",
};

void LowRenderer::Model::MaterialPopUp(Resources::MaterialManager* materialManager, size_t i)
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	ImGui::InputText("Search", searchText, 64);
	std::vector<Resources::Material*> searchResult = materialManager->GetMaterials(searchText);
	if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
	{
		if (i >= materials.size())
		{
			if (searchResult.size() == 0)
			{
				ImGui::CloseCurrentPopup();
				ImGui::PopStyleColor();
				ImGui::EndPopup();
				return;
			}
			else if (selectedMaterial == nullptr) selectedMaterial = searchResult[0];
			for (size_t n = materials.size(); n < i; n++)
			{
				materials.push_back(materials.back());
			}
			materials.push_back(*selectedMaterial);
		}
		materials[i].UnLoad();
		materials[i] = *selectedMaterial;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel") || ImGui::IsKeyDown(ImGuiKey_Escape))
	{
		ImGui::CloseCurrentPopup();
	}
	for (unsigned int j = 0; j < searchResult.size(); j++)
	{
		ImGui::Image((ImTextureID)(size_t)searchResult[j]->GetTexture()->GetTextureID(), ImVec2(32, 32));
		bool clicked = ImGui::IsItemClicked();
		ImGui::SameLine();
		if (ImGui::Selectable(searchResult[j]->GetName(), selectedMaterial == searchResult[j], ImGuiSelectableFlags_DontClosePopups) || clicked)
		{
			selectedMaterial = searchResult[j];
		}
	}
	ImGui::PopStyleColor();
	ImGui::EndPopup();
}

void LowRenderer::Model::MeshPopUp(Resources::MeshManager* meshManager, size_t i)
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	ImGui::InputText("Search", searchText, 64);
	std::vector<Resources::Mesh*> searchResult = meshManager->GetMeshs(searchText);
	if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
	{
		if (i < models.size())
		{
			if (models[i]->GetPath() == selectedMesh->GetPath() && searchResult.size() > 0) selectedMesh = searchResult[0];
			models[i] = selectedMesh;
		}
		else
		{
			if (selectedMesh == nullptr && searchResult.size() > 0) selectedMesh = searchResult[0];
			if (selectedMesh != nullptr)
			{
				models.push_back(selectedMesh);
				modelVisible.push_back(true);
			}
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
		if (ImGui::Selectable(searchResult[j]->GetName(), selectedMesh == searchResult[j], ImGuiSelectableFlags_DontClosePopups))
		{
			selectedMesh = searchResult[j];
		}
	}
	ImGui::PopStyleColor();
	ImGui::EndPopup();
}

void LowRenderer::Model::ShaderPopUp(Resources::ShaderManager* shaderManager)
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	ImGui::InputText("Search", searchText, 64);
	std::vector<Resources::ShaderProgram*> searchResult = shaderManager->GetShaderPrograms(searchText);
	if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
	{
		if (shaderProgram == selectedProgram && searchResult.size() > 0) selectedProgram = searchResult[0];
		shaderProgram = selectedProgram;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel") || ImGui::IsKeyDown(ImGuiKey_Escape))
	{
		ImGui::CloseCurrentPopup();
	}
	for (unsigned int i = 0; i < searchResult.size(); i++)
	{
		if (ImGui::Selectable(searchResult[i]->GetName(), selectedProgram == searchResult[i], ImGuiSelectableFlags_DontClosePopups))
		{
			selectedProgram = searchResult[i];
		}
	}
	ImGui::PopStyleColor();
	ImGui::EndPopup();
}

void LowRenderer::Model::ModelPopUp(Resources::MeshManager* meshManager)
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	ImGui::InputText("Search", searchText, 64);
	std::vector<Resources::ModelHolder*> searchResult = meshManager->GetModels(searchText);
	if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
	{
		if (selectedModel == nullptr && searchResult.size() > 0) selectedModel = searchResult[0];
		if (selectedModel != nullptr)
		{
			this->CreateFrom(selectedModel->model);
		}
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel") || ImGui::IsKeyDown(ImGuiKey_Escape))
	{
		ImGui::CloseCurrentPopup();
	}
	for (unsigned int i = 0; i < searchResult.size(); i++)
	{
		if (ImGui::Selectable(searchResult[i]->GetName(), selectedModel == searchResult[i], ImGuiSelectableFlags_DontClosePopups))
		{
			selectedModel = searchResult[i];
		}
	}
	ImGui::PopStyleColor();
	ImGui::EndPopup();
}

LowRenderer::Model::Model()
{
}

LowRenderer::Model::Model(Resources::ShaderProgram* Shader) : shaderProgram(Shader)
{
}

LowRenderer::Model::~Model() {}

void LowRenderer::Model::Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode mode, bool isSelected)
{
	if (!Enabled) return;

	if (mode == RenderMode::DebugHalo && isSelected)
	{
		Core::Maths::Mat4D mvp = vp * (modelMat * Core::Maths::Mat4D::CreateScaleMatrix(Core::Maths::Vec3D(1.1f)));
		glUniformMatrix4fv((*shaderProgramCurrent)->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
		for (size_t i = 0; i < models.size(); i++)
		{
			models[i]->Draw(VAOCurrent);
		}
		return;
	}
	if (!shaderProgram || models.size() == 0 || materials.size() == 0) return;
	if (!(mode == RenderMode::DebugBase && isSelected) && (!((unsigned char)mode & (unsigned char)renderMode))) return;

	// Set matrix of model.
	Core::Maths::Mat4D mvp = vp * modelMat;
	if (*shaderProgramCurrent != shaderProgram)
	{
		glUseProgram(shaderProgram->GetProgramID());
		*shaderProgramCurrent = shaderProgram;
	}
	currentTexture = materials[0].GetTexture();
	currentNormalMap = materials[0].GetNormalMap();
	glUniformMatrix4fv(shaderProgram->GetLocation(Resources::ShaderData::MVP), 1, GL_FALSE, mvp.content);
	glUniformMatrix4fv(shaderProgram->GetLocation(Resources::ShaderData::M), 1, GL_FALSE, modelMat.content);
	glUniform1i(shaderProgram->GetLocation(Resources::ShaderData::Texture), currentTexture->GetTextureID());
	if (currentNormalMap) glUniform1i(shaderProgram->GetLocation(Resources::ShaderData::NTexture), currentNormalMap->GetTextureID());
	if (lvp && lvp->size() > 0)
	{
		glUniformMatrix4fv(shaderProgram->GetLocation(Resources::ShaderData::LightMVP), (int)lvp->size(), GL_FALSE, lvp->data()->content);
	}
	// Render all meshs.
	for (size_t i = 0; i < models.size(); i++)
	{
		if (!modelVisible[i]) continue;
		size_t matIndex = i < materials.size() ? i : materials.size() - 1;
		glUniform3f(shaderProgram->GetLocation(Resources::ShaderData::MatAmbient), materials[matIndex].AmbientColor.x, materials[matIndex].AmbientColor.y, materials[matIndex].AmbientColor.z);
		glUniform3f(shaderProgram->GetLocation(Resources::ShaderData::MatDiffuse), materials[matIndex].DiffuseColor.x, materials[matIndex].DiffuseColor.y, materials[matIndex].DiffuseColor.z);
		glUniform3f(shaderProgram->GetLocation(Resources::ShaderData::MatSpecular), materials[matIndex].SpecularColor.x, materials[matIndex].SpecularColor.y, materials[matIndex].SpecularColor.z);
		glUniform3f(shaderProgram->GetLocation(Resources::ShaderData::MatEmissive), materials[matIndex].EmissiveColor.x, materials[matIndex].EmissiveColor.y, materials[matIndex].EmissiveColor.z);
		glUniform1f(shaderProgram->GetLocation(Resources::ShaderData::MatAlpha), materials[matIndex].Alpha);
		glUniform1f(shaderProgram->GetLocation(Resources::ShaderData::MatSmoothness), materials[matIndex].Smoothness);
		glUniform1f(shaderProgram->GetLocation(Resources::ShaderData::MatShininess), materials[matIndex].Shininess);
		glUniform1f(shaderProgram->GetLocation(Resources::ShaderData::MatAbsorbtion), materials[matIndex].Absorbtion);
		if (materials[matIndex].GetTexture() != currentTexture)
		{
			currentTexture = materials[matIndex].GetTexture();
			glUniform1i(shaderProgram->GetLocation(Resources::ShaderData::Texture), currentTexture->GetTextureID());
		}
		if (materials[matIndex].GetNormalMap() != currentNormalMap)
		{
			currentNormalMap = materials[matIndex].GetNormalMap();
			if (currentNormalMap) glUniform1i(shaderProgram->GetLocation(Resources::ShaderData::NTexture), currentNormalMap->GetTextureID());
		}
		models[i]->Draw(VAOCurrent);
	}
}

int LowRenderer::Model::GetNumberOfTriangles()
{
	int x = 0;
	for (size_t i = 0; i < models.size(); i++)
	{
		x += models[i]->GetVerticesCount() / 3;
	}
	return x;
}

void LowRenderer::Model::LoadMesh(const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* meshManager)
{
	Core::Util::ModelLoader::LoadMesh(*this, path, manager, materialManager, textureManager, meshManager);
	currentTexture = materials.at(0).GetTexture();
}

void LowRenderer::Model::AddMesh(Resources::Mesh* modelIn, Resources::ResourceManager* manager, Resources::Material* modelMaterial)
{
	models.push_back(modelIn);
	modelVisible.push_back(true);
	if (modelMaterial->GetTexture() == nullptr) modelMaterial->SetTexture(manager->Get<Resources::Texture>("DefaultResources/Textures/Blank.png"));
	if (modelMaterial->GetNormalMap() == nullptr) modelMaterial->SetNormalMap(manager->Get<Resources::Texture>("DefaultResources/Textures/normal.png"));
	//materials.push_back(modelMaterial);
}

void LowRenderer::Model::CreateFrom(const Model* other)
{
	for (size_t i = 0; i < other->models.size(); i++)
	{
		models.push_back(other->models[i]);
		modelVisible.push_back(true);
	}
	for (size_t i = 0; i < other->materials.size(); i++)
	{
		materials.push_back(other->materials[i]);
	}
}

void LowRenderer::Model::DeleteComponent()
{
	for (size_t i = 0; i < materials.size(); i++) materials[i].UnLoad();
	materials.clear();
	this->~Model();
}

void LowRenderer::Model::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
	modelMat = *container->GetGlobalMatrix();
	for (size_t i = 0; i < materials.size(); i++)
	{
		materials[i].Update(resources);
	}
}

void LowRenderer::Model::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new LowRenderer::Model());
}

void LowRenderer::Model::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Component::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "ShaderProgram"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string name = Text::getText(data, pos, size);
			if (!name.c_str())
			{
				err = "Expected name after \"ShaderProgram\"";
				break;
			}
			shaderProgram = shaders->GetShaderProgram(name.c_str());
			if (!shaderProgram)
			{
				LOG("Warning : invalid shader name %s", name.c_str());
			}
		}
		else if (Text::compareWord(data, pos, size, "RenderMode"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode > (int)RenderMode::All)
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"RenderMode\" : valid numbers are [0-7]", mode);
				err = buff;
				break;
			}
			renderMode = (RenderMode)mode;
		}
		else if (Text::compareWord(data, pos, size, "Elements"))
		{
			pos = Text::endLine(data, pos, size);
			line++;
			DeserializeElements(resources, data, pos, size, line, err);
			if (err.c_str()[0]) break;
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::Model::DeserializeElements(Resources::ResourceManager* resources, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Resources::Mesh* currentMod = nullptr;
	bool visible = true;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndElements"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "NewMesh"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string path = Text::getText(data, pos, size);
			if (!path.c_str())
			{
				err = "Expected path after \"NewMesh\"";
				break;
			}
			models.push_back(resources->Get<Resources::Mesh>(path.c_str()));
			modelVisible.push_back(true);
			currentMod = models.back();
		}
		else if (Text::compareWord(data, pos, size, "NewMaterial"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string path = Text::getText(data, pos, size);
			if (!path.c_str())
			{
				err = "Expected path after \"NewMaterial\"";
				break;
			}
			Resources::Material* mat = resources->Get<Resources::Material>(path.c_str());
			if (mat) materials.push_back(*mat);
			else
			{
				LOG("Warning : Unknown Material %s at line %ld", path.c_str(), line);
				materials.push_back(Resources::Material());
			}
			pos = Text::endLine(data, pos, size);
			line++;
			materials.back().Deserialize(resources, data, pos, size, line, err);
			if (err.c_str()[0]) return;
		}
		else if (Text::compareWord(data, pos, size, "Visible"))
		{
			if (!currentMod)
			{
				err = "\"Visible\" called before model declaration";
				break;
			}
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, visible))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
			modelVisible[modelVisible.size() - 1llu] = visible;
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::Model::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* meshManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, meshManager);
	const char* text = (shaderProgram == nullptr ? "Shader" : shaderProgram->GetName());
	if (ImGui::Button(text))
	{
		selectedProgram = shaderProgram;
		ImGui::OpenPopup("Shader Selection");
		ImGui::SetNextWindowSize(ImVec2(700, 400));
	}
	if (ImGui::BeginPopupModal("Shader Selection")) ShaderPopUp(shaderManager);
	ImGui::Combo("Render Mode", (int*)&renderMode, RenderModeDesc, 8, -1);
	ImGui::Text("Triangles : %d", GetNumberOfTriangles());
	if (ImGui::CollapsingHeader("Meshes"))
	{
		RenderMeshes(textureManager, materialManager, meshManager);
		ImGui::Button("Add Mesh");
		if (ImGui::IsItemClicked())
		{
			selectedMesh = nullptr;
			ImGui::OpenPopup("Mesh Selection");
			ImGui::SetNextWindowSize(ImVec2(700, 400));
		}
		if (ImGui::BeginPopupModal("Mesh Selection")) MeshPopUp(meshManager, models.size());
		if (models.size() > 0 && selectedMeshIndex < models.size())
		{
			ImGui::SameLine();
			ImGui::Button("Delete Mesh");
			if (ImGui::IsItemClicked())
			{
				materials[selectedMeshIndex].UnLoad();
				for (unsigned int n = selectedMeshIndex; n < models.size() - 1; n++)
				{
					models[n] = models[n + 1llu];
					modelVisible[n] = modelVisible[n + 1llu];
				}
				for (unsigned int n = selectedMeshIndex + 1; n < materials.size(); n++)
				{
					materials[n - 1llu] = materials[n];
				}
				if (materials.size() >= models.size()) materials.pop_back();
				models.pop_back();
				modelVisible.pop_back();
			}
		}
		if (models.size() == 0)
		{
			ImGui::SameLine();
			ImGui::Button("Create from Model");
			if (ImGui::IsItemClicked())
			{
				selectedMesh = nullptr;
				ImGui::OpenPopup("Model Selection");
				ImGui::SetNextWindowSize(ImVec2(700, 400));
			}
			if (ImGui::BeginPopupModal("Model Selection")) ModelPopUp(meshManager);
		}
	}
	if (dragging && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		if (selectedMeshDragIndex < models.size() && selectedMeshIndex < models.size())
		{
			Resources::Mesh* tmp = models[selectedMeshIndex];
			bool tmp2 = modelVisible[selectedMeshIndex];
			models[selectedMeshIndex] = models[selectedMeshDragIndex];
			modelVisible[selectedMeshIndex] = modelVisible[selectedMeshDragIndex];
			models[selectedMeshDragIndex] = tmp;
			modelVisible[selectedMeshDragIndex] = tmp2;
		}
		if (selectedMeshDragIndex < materials.size() && selectedMeshIndex < materials.size())
		{
			Resources::Material tmp = materials[selectedMeshIndex];
			materials[selectedMeshIndex] = materials[selectedMeshDragIndex];
			materials[selectedMeshDragIndex] = tmp;
		}
		dragging = false;
	}
}

void LowRenderer::Model::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	if (shaderProgram)
	{
		Parsing::Shift(fileOut, rec);
		fileOut << "ShaderProgram " << shaderProgram->GetName() << std::endl;
	}
	Parsing::Shift(fileOut, rec);
	fileOut << "RenderMode " << (unsigned int)renderMode << std::endl;
	Parsing::Shift(fileOut, rec+1);
	fileOut << "Elements" << std::endl;
	for (size_t i = 0; i < materials.size(); i++)
	{
		materials[i].Serialize(fileOut, rec+2);
	}
	for (size_t i = 0; i < models.size(); i++)
	{
		Parsing::Shift(fileOut, rec+2);
		fileOut << "NewMesh " << models[i]->GetPath() << std::endl;
		Parsing::Shift(fileOut, rec+3);
		fileOut << "Visible " << (modelVisible[i] ? "True" : "False") << std::endl;
	}
	Parsing::Shift(fileOut, rec+1);
	fileOut << "EndElements" << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::Model::RenderMeshes(Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* meshManager)
{
	for (unsigned int i = 0; i < models.size(); i++)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 15);
		ImGui::PushID(i);
		bool colored = false;
		if (dragging && selectedMeshDragIndex == i)
		{
			colored = true;
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 127, 127, 255));
		}
		else if (selectedMeshIndex == i)
		{
			colored = true;
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
		}
		bool opened = ImGui::CollapsingHeader(models[i]->GetName(), ImGuiTreeNodeFlags_OpenOnArrow);
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			selectedMeshIndex = i;
		}
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
			{
				if (dragging)
				{
					if (selectedMeshDragIndex != i) selectedMeshDragIndex = i;
				}
				else if (selectedMeshIndex == i)
				{
					dragging = true;
				}
			}
		}
		if (opened)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
			bool tmp = modelVisible[i];
			ImGui::Checkbox("Visible", &tmp);
			modelVisible[i] = tmp;
			ImGui::SameLine();
			ImGui::Button("Change Mesh");
			if (ImGui::IsItemClicked())
			{
				selectedMesh = models[i];
				ImGui::OpenPopup("Mesh Selection");
				ImGui::SetNextWindowSize(ImVec2(700, 400));
			}
			if (ImGui::BeginPopupModal("Mesh Selection")) MeshPopUp(meshManager, i);
			if (i >= materials.size() || materials[i].GetTexture() == nullptr || materials[i].GetNormalMap() == nullptr)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
				ImGui::Image((ImTextureID)1llu, ImVec2(32, 32));
				ImGui::SameLine();
				ImGui::Text("Material");
			}
			else
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
				ImGui::Image((ImTextureID)(size_t)materials[i].GetTexture()->GetTextureID(), ImVec2(32, 32));
				ImGui::SameLine();
				ImGui::Text(materials[i].GetName());
			}
			ImGui::SameLine();
			ImGui::Button("Change Material");
			if (ImGui::IsItemClicked())
			{
				if (i < materials.size()) selectedMaterial = &materials[i];
				ImGui::OpenPopup("Material Selection");
				ImGui::SetNextWindowSize(ImVec2(700, 400));
			}
			if (ImGui::BeginPopupModal("Material Selection")) MaterialPopUp(materialManager, i);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
			if (ImGui::CollapsingHeader("Material Settings") && materials.size() > i)
			{
				materials[i].RenderGUI(textureManager);
			}
		}
		if (colored) ImGui::PopStyleColor();
		ImGui::PopID();
	}
}