#include "Physics/Colliders/Collider.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"
#include "Core/DataStructure/Node.hpp"
#include "Physics/PhysicsHandler.hpp"

void Physics::Colliders::Collider::PreUpdate()
{
	AttachedBody = nullptr;
	Entered = false;
}

void Physics::Colliders::Collider::PhysicUpdate(Core::DataStructure::Node* container, std::vector<Physics::Colliders::Collider*>* colliders, float DeltaTime)
{
	parent = container;
}

void Physics::Colliders::Collider::LayerPopUp()
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	std::string label = "Layer XX";
	for (char i = 0; i < 32; i++)
	{
		label.data()[6] = i / 10 + '0';
		label.data()[7] = i % 10 + '0';
		ImGui::TextUnformatted(Handler->CollisionLayerNames[i].c_str());
		ImGui::SameLine();
		ImGui::SetCursorPosX(Handler->CollisionLayerNames[i].size() + 4.0f);
		unsigned int ind = (0x1 << i);
		bool selected = LayerMask & ind;
		ImGui::Checkbox(label.c_str(), &selected);
		if (selected != (bool)(LayerMask & ind))
		{
			LayerMask ^= ind;
		}
	}
	if (ImGui::Button("Select All"))
	{
		LayerMask = 0xffffffff;
	}
	ImGui::SameLine();
	if (ImGui::Button("Deselect All"))
	{
		LayerMask = 0x0;
	}
	ImGui::SameLine();
	if (ImGui::Button("Invert"))
	{
		LayerMask = ~LayerMask;
	}
	if (ImGui::Button("Close") || ImGui::IsKeyDown(ImGuiKey_Escape))
	{
		ImGui::CloseCurrentPopup();
	}
	ImGui::PopStyleColor();
	ImGui::EndPopup();
}

void Physics::Colliders::Collider::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::Checkbox("Should Notify Childs On Collision ?", &NotifyChilds);
	ImGui::Checkbox("Is Trigger", &IsTrigger);
	ImGui::Text("Touched : %s", Entered ? "True" : "False");
	ImGui::Text("Collision Mask : %08X", LayerMask);
	if (Handler)
	{
		ImGui::SameLine();
		ImGui::Button("Change Layer");
		if (ImGui::IsItemClicked())
		{
			ImGui::OpenPopup("Layer Edit");
			ImGui::SetNextWindowSize(ImVec2(700, 400));
		}
		if (ImGui::BeginPopupModal("Layer Edit")) LayerPopUp();
	}
}

void Physics::Colliders::Collider::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "NotifyChilds"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, NotifyChilds))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "IsTrigger"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, IsTrigger))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "LayerMask"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			LayerMask = (unsigned int)Text::getInt(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Physics::Colliders::Collider::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "LayerMask " << LayerMask << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "NotifyChilds " << (NotifyChilds ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "IsTrigger " << (IsTrigger ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "EndSubComponent" << std::endl;
}