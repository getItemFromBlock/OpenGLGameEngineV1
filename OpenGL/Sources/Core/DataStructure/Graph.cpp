#include "Core/DataStructure/Graph.hpp"

#include <ImGUI/imgui.h>
#include <iomanip>

#include "LowRenderer/Component.hpp"
#include "Core/DataStructure/NameSearcher.hpp"
#include "Core/Util/ModelLoader.hpp"
#include "Core/Util/SceneLoader.hpp"
#include "Core/Util/TextHelper.hpp"

char Core::DataStructure::Graph::searchText[64] = "";
LowRenderer::Component* Core::DataStructure::Graph::selectedElement = nullptr;

Core::DataStructure::Graph::Graph()
{
}

Core::DataStructure::Graph::~Graph()
{
}

void Core::DataStructure::Graph::RenderGUI(Resources::ResourceManager* resources, Physics::PhysicsHandler* handler, Resources::ShaderManager* shaderManager,
	Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager, SceneNode* node)
{
	if (ImGui::Begin("Scene Graph"))
	{
		bool focused = ImGui::IsWindowFocused();
		node->RenderGUI(&selected, &hovered);
		if (selected != nullptr)
		{
			if (ImGui::Button("Add Node"))
			{
				selected->childs.push_back(new Node(selected, "Node"));
				selected->childs[selected->childs.size() - 1llu]->Refresh();
				selected = selected->childs[selected->childs.size() - 1llu];
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Node"))
			{
				DeleteSelectedNode(node);
			}
			if (focused)
			{
				HandleFocusActions(node, handler, resources, shaderManager);
			}
			if (ImGui::Button("Deselect"))
			{
				selected = nullptr;
			}
		}
	}
	bool drag = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
	if (!drag && !NodeDragging) hovered = nullptr;
	if (hovered && drag && !NodeDragging)
	{
		NodeDragging = true;
		selected = hovered;
	}
	if (NodeDragging && !drag)
	{
		if (selected != hovered) SwapNodes(selected, hovered);
		NodeDragging = false;
	}
	ImGui::End();
	RenderInspector(resources, shaderManager, materialManager, textureManager, modelManager, node);
}

void Core::DataStructure::Graph::RenderInspector(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager, SceneNode* node)
{
	if (ImGui::Begin("Element Inspector") && selected && selected != node)
	{
		ImGui::Text(selected->Name);
		selected->RenderTransform();
		for (unsigned int i = 0; i < selected->components.size(); i++)
		{
			ImGui::PushID(i);
			bool colored = false;
			if (dragging && selectedComponentDragIndex == i)
			{
				colored = true;
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 127, 127, 255));
			}
			else if (selectedComponent == i)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
				colored = true;
			}
			bool open = ImGui::CollapsingHeader(selected->components[i]->GetName(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				selectedComponent = i;
			}
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
				{
					if (dragging)
					{
						if (selectedComponentDragIndex != i) selectedComponentDragIndex = i;
					}
					else if (selectedComponent == i)
					{
						dragging = true;
					}
				}
			}
			if (open)
			{
				selected->components[i]->RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
			}
			if (colored) ImGui::PopStyleColor();
			ImGui::PopID();
		}
		if (dragging && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (selectedComponentDragIndex < selected->components.size() && selectedComponent < selected->components.size())
			{
				LowRenderer::Component* tmp = selected->components[selectedComponentDragIndex];
				selected->components[selectedComponentDragIndex] = selected->components[selectedComponent];
				selected->components[selectedComponent] = tmp;
			}
			dragging = false;
		}
		if (ImGui::Button("Add Component"))
		{
			selectedElement = nullptr;
			searchText[0] = 0;
			ImGui::OpenPopup("Component Selection");
			ImGui::SetNextWindowSize(ImVec2(700, 400));
		}
		if (ImGui::BeginPopupModal("Component Selection")) ComponentPopUp();
		if (selectedComponent < selected->components.size())
		{
			ImGui::SameLine();
			if (ImGui::Button("Delete Component"))
			{
				delete selected->components[selectedComponent];
				for (unsigned int i = selectedComponent; i < selected->components.size() - 1; i++)
				{
					selected->components[i] = selected->components[i + 1llu];
				}
				selected->components.pop_back();
			}
		}
	}
	ImGui::End();
}

void Core::DataStructure::Graph::SwapNodes(Node* a, Node* b)
{
	if (a == nullptr || b == nullptr || a->parent == nullptr || b->parent == nullptr) return;
	Node* ParentA = a->parent;
	unsigned int indexA = 0;
	for (unsigned int i = 0; i < ParentA->childs.size(); i++)
	{
		if (ParentA->childs[i] == a)
		{
			indexA = i;
			break;
		}
	}
	Node* ParentB = b->parent;
	unsigned int indexB = 0;
	for (unsigned int i = 0; i < ParentB->childs.size(); i++)
	{
		if (ParentB->childs[i] == b)
		{
			indexB = i;
			break;
		}
	}
	std::vector<Core::Maths::Mat4D> matA;
	for (Node* local = ParentB; local->parent; local = local->parent)
	{
		matA.push_back(local->GetLocalMatrix()->CreateInverseMatrix());
		if (local == b || local == a) return;
	}
	std::vector<Core::Maths::Mat4D> matB;
	for (Node* local = ParentA; local->parent; local = local->parent)
	{
		matB.push_back(local->GetLocalMatrix()->CreateInverseMatrix());
		if (local == a || local == b) return;
	}
	Core::Maths::Mat4D dest = *a->GetGlobalMatrix();
	for (size_t i = matA.size(); i > 0; i--)
	{
		dest = matA[i - 1llu] * dest;
	}
	Core::Maths::Vec3D scale = dest.GetScaleFromTranslation();
	a->SetLocalPosition(dest.GetPositionFromTranslation());
	a->SetLocalRotation(dest.GetRotationFromTranslation(scale));
	a->SetLocalScale(scale);
	dest = *b->GetGlobalMatrix();
	for (size_t i = matB.size(); i > 0; i--)
	{
		dest = matB[i - 1llu] * dest;
	}
	scale = dest.GetScaleFromTranslation();
	b->SetLocalPosition(dest.GetPositionFromTranslation());
	b->SetLocalRotation(dest.GetRotationFromTranslation(scale));
	b->SetLocalScale(scale);
	a->parent = ParentB;
	b->parent = ParentA;
	Node* tmp = a;
	ParentA->childs[indexA] = b;
	ParentB->childs[indexB] = tmp;
}

void Core::DataStructure::Graph::HandleFocusActions(Core::DataStructure::SceneNode* node, Physics::PhysicsHandler* handler, Resources::ResourceManager* resources, Resources::ShaderManager* shaders)
{
	if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_C))
	{
		CopyNode();
	}
	if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_X))
	{
		CopyNode();
		DeleteSelectedNode(node);
	}
	if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) && ImGui::IsKeyPressed(ImGuiKey_V))
	{
		Core::Util::SceneLoader::LoadScene("clipboard.txt", selected, this, handler, resources, shaders);
	}
}

void Core::DataStructure::Graph::CopyNode()
{
	std::ofstream clipboard;
	clipboard.open("clipboard.txt", std::ios::out | std::ios::binary | std::ios::trunc);
	char errCode[128];
	if (!clipboard.is_open() || clipboard.fail())
	{
		LOG("Error, cannot open clipboard : %s", strerror_s(errCode, 128, errno));
	}
	else
	{
		clipboard << std::fixed << std::setprecision(6);
		clipboard << "NewNode " << selected->Name << std::endl;
		Parsing::Shift(clipboard, 1);
		clipboard << "Position " << selected->transform.position.x << " " << selected->transform.position.y << " " << selected->transform.position.z << std::endl;
		Parsing::Shift(clipboard, 1);
		clipboard << "Rotation " << selected->transform.rotation.x << " " << selected->transform.rotation.y << " " << selected->transform.rotation.z << std::endl;
		Parsing::Shift(clipboard, 1);
		clipboard << "Scale " << selected->transform.scale.x << " " << selected->transform.scale.y << " " << selected->transform.scale.z << std::endl;
		selected->Serialize(clipboard, 1);
		clipboard << "EndNode " << std::endl;
		clipboard << "EndScene " << std::endl;
		clipboard.close();
		int64_t size;
		const char* data = Core::Util::ModelLoader::loadFile("clipboard.txt", size);
		if (data)
		{
			ImGui::LogToClipboard();
			ImGui::LogText(data);
			ImGui::LogFinish();
			delete[] data;
		}
	}
}

void Core::DataStructure::Graph::PasteNode(Resources::ResourceManager* resources, Resources::ShaderManager* shaders)
{

}

void Core::DataStructure::Graph::DeleteSelectedNode(SceneNode* node)
{
	selected->Destroy();
	Node* tmp = selected->parent;
	if (selected != node)
	{
		delete selected;
		unsigned int index = 0;
		for (unsigned int i = 0; i < tmp->childs.size(); i++)
		{
			if (tmp->childs[i] == selected)
			{
				index = i;
				break;
			}
		}
		for (unsigned int i = index; i < tmp->childs.size() - 1; i++)
		{
			tmp->childs[i] = tmp->childs[i + 1llu];
		}
		tmp->childs.pop_back();
	}
	selected = tmp;
}

void Core::DataStructure::Graph::AddComponent(LowRenderer::Component* in)
{
	components.push_back(in);
}

const std::vector<LowRenderer::Component*> Core::DataStructure::Graph::GetComponents()
{
	return components;
}

void Core::DataStructure::Graph::DeleteComponents()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->DeleteComponent();
		delete components[i];
	}
	components.clear();
}

void Core::DataStructure::Graph::ComponentPopUp()
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	ImGui::InputText("Search", searchText, 64);
	std::vector<LowRenderer::Component*> searchResult;
	if (searchText[0] != 0)
	{
		searchResult = Core::DataStructure::NameSearcher::FindElementsPtr<LowRenderer::Component>(components, searchText);
	}
	else
	{
		for (unsigned int i = 0; i < components.size(); i++)
		{
			searchResult.push_back(components[i]);
		}
	}
	if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
	{
		if (selectedElement == nullptr && searchResult.size() > 0) selectedElement = searchResult[0];
		if (selectedElement != nullptr)
		{
			selectedElement->Create(selected);
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
		if (ImGui::Selectable(searchResult[i]->GetName(), selectedElement == searchResult[i], ImGuiSelectableFlags_DontClosePopups))
		{
			selectedElement = searchResult[i];
		}
	}
	ImGui::PopStyleColor();
	ImGui::EndPopup();
}