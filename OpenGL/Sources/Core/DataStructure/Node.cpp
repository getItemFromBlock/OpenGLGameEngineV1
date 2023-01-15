#include "Core/DataStructure/Node.hpp"

#include <ImGUI/imgui.h>

#include "LowRenderer/Component.hpp"
#include "Core/DataStructure/Node.hpp"
#include "LowRenderer/Lightning/LightManager.hpp"
#include "Core/Util/TextHelper.hpp"
#include "LowRenderer/Lightning/Light.hpp"
#include "LowRenderer/Rendering/Camera.hpp"
#include "Core/App/App.hpp"
#include "LowRenderer/Lightning/ShadowMapBuffer.hpp"
#include "LowRenderer/Rendering/RenderCamera.hpp"
#include "LowRenderer/Rendering/FrameBuffer.hpp"
#include "Physics/Colliders/Collider.hpp"

Core::DataStructure::SceneNode::SceneNode() : Node(nullptr, "Scene")
{
}

Core::DataStructure::SceneNode::~SceneNode()
{
}

void Core::DataStructure::SceneNode::RenderScene(Resources::ShaderManager* shaders, LowRenderer::Lightning::LightManager* lights, Resources::TextureManager* textures, Resources::ResourceManager* resources, unsigned int& VAOCurrent,
	Physics::PhysicsHandler* physics, Resources::ShaderProgram** shaderProgramCurrent, bool ortho, float deltaTime, float globalTime)
{
	LowRenderer::Rendering::EditorCamera* MainCam = Core::App::App::GetMainCamera();
	lights->EmptyLights();
	std::vector<LowRenderer::Rendering::RenderCamera*> cameras;
	PreUpdate();
	physics->UpdatePhysics(this, deltaTime != 0.0f, deltaTime);
	Update(&cameras, resources, textures, lights, deltaTime);
	Refresh();
	LowRenderer::Rendering::Camera cam = LowRenderer::Rendering::Camera();
	bool changed = false;
	for (size_t i = 0; i < lights->ShadowMapLights.size(); i++)
	{
		glCullFace(GL_FRONT);
		LowRenderer::SpotLight* s = lights->ShadowMapLights[i];
		if (!s->shadowMap)
		{
			std::string path = s->GetName();
			path.append("@");
			char text[6] = "xxxxx";
			for (unsigned int i = 0; i < 5; i++)
			{
				int c = (int)(rand() * 36.0f / RAND_MAX);
				text[i] = c < 10 ? c + '0' : c + 'A' - 10;
			}
			path.append(text);
			s->shadowMap = resources->Create<LowRenderer::Lightning::ShadowMapBuffer>(path.c_str());
			textures->AddShadowMap(s->shadowMap);
		}
		s->shadowMap->RefreshSize();
		cam.fov = s->Spotangle * 2 * 180 / (float)M_PI;
		cam.nearPlane = s->nearPlane;
		cam.farPlane = s->farPlane;
		cam.Update(Core::Maths::Int2D(s->shadowMap->GetTextureWidth(), s->shadowMap->GetTextureHeight()), s->Position, s->GlobalDirection, s->GlobalUp);
		Core::Maths::Mat4D tmpVP = cam.GetProjectionMatrix() * cam.GetViewMatrix();
		s->shadowMap->VPLight = tmpVP;

		if (s->ShadowType == ShadowUpdateType::Realtime || s->inited == false)
		{
			changed = true;
			s->inited = true;
			s->shadowMap->BindForWriting();
			glClear(GL_DEPTH_BUFFER_BIT);
			Render(VAOCurrent, shaderProgramCurrent, tmpVP, nullptr, RenderMode::Shadow);
		}
	}
	if (lights->ShadowMapLights.size() != 0)
	{
		glCullFace(GL_BACK);
	}
	std::vector<Core::Maths::Mat4D> lvp;
	lights->UpdateShaders(shaderProgramCurrent, shaders, MainCam->position, &lvp, globalTime);
	for (size_t i = 0; i < cameras.size(); i++)
	{
		if (!cameras[i]->buffer || !cameras[i]->buffer->BindForWriting()) continue;
		changed = true;
		Core::Maths::Mat4D tmpVP = cameras[i]->GetProjectionMatrix() * cameras[i]->GetViewMatrix();
		if (cameras[i]->ClearBuffer)
		{
			glClearColor(cameras[i]->ClearColor.x, cameras[i]->ClearColor.y, cameras[i]->ClearColor.z, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		glClear(GL_DEPTH_BUFFER_BIT);
		Render(VAOCurrent, shaderProgramCurrent, tmpVP, &lvp, RenderMode::SecondaryRender);
	}
	if (changed)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		Core::Maths::Int2D res = Core::App::App::GetMainCamera()->GetResolution();
		glViewport(0, 0, res.x, res.y);
	}
	Core::Maths::Mat4D vp = (ortho ? MainCam->GetOrthoMatrix() : MainCam->GetProjectionMatrix()) * MainCam->GetViewMatrix();
	Render(VAOCurrent, shaderProgramCurrent, vp, &lvp, RenderMode::Mainrender);
	if (App::App::IsDebugMode())
	{
		if (!App::App::IsWireframe())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
		}

		std::vector<Physics::Colliders::Collider*>* colliders = physics->GetColliders();
		Resources::ShaderProgram* program = shaders->GetShaderPrograms().at(shaders->GetShaderPrograms().size() - 2);
		glUseProgram(program->GetProgramID());
		for (size_t i = 0; i < colliders->size(); i++)
		{
			colliders->at(i)->DrawCollider(resources, program, VAOCurrent, vp);
		}
		glUseProgram((*shaderProgramCurrent)->GetProgramID());
		if (!App::App::IsWireframe())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
		}

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(1);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		Render(VAOCurrent, shaderProgramCurrent, vp, &lvp, RenderMode::DebugBase);
		glStencilFunc(GL_EQUAL, 0, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilMask(0x00);
		glUseProgram(program->GetProgramID());
		glUniform3f(program->GetLocation(Resources::ShaderData::MatAmbient), 1, 1, 0);
		glUniform1f(program->GetLocation(Resources::ShaderData::MatAlpha), 1);
		glDisable(GL_DEPTH_TEST);
		Render(VAOCurrent, &program, vp, &lvp, RenderMode::DebugHalo);
		glUseProgram((*shaderProgramCurrent)->GetProgramID());
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
	}
	glDisable(GL_DEPTH_TEST);
	Core::Maths::Mat4D v = MainCam->GetOrthoMatrix();
	ImVec2 MPos = ImGui::GetMousePos();
	unsigned int MInputs = 0;
	for (unsigned int i = 0; i < static_cast<unsigned int>(MouseInput::COUNT); i++)
	{
		MInputs |= (i > 4 ? ImGui::IsMouseClicked(i%5) : ImGui::IsMouseDown(i)) << i;
	}
	for (unsigned int i = 0; i < static_cast<unsigned int>(Core::App::InputType::Crouch); i++)
	{
		MInputs |= ImGui::IsKeyPressed(Core::App::App::GetInputBindings()[i]) << (i+16);
	}
	Resources::ShaderProgram* LitShader = shaders->GetShaderProgram("ui shader");
	if (!LitShader) return;
	glUseProgram(LitShader->GetProgramID());
	glUniform1i(LitShader->GetLocation(Resources::ShaderData::Texture), Core::App::DefaultTextures::Blank);
	RenderGameUI(VAOCurrent, &LitShader, v, Core::App::App::GetMainCamera()->GetResolution(), Core::Maths::Vec2D(MPos.x, MPos.y), ImGui::GetScrollY(), MInputs);
	glUseProgram((*shaderProgramCurrent)->GetProgramID());
	glEnable(GL_DEPTH_TEST);
}

Core::DataStructure::Node::Node()
{
}

Core::DataStructure::Node::Node(Node* pr)
{
	parent = pr;
}

Core::DataStructure::Node::Node(Node* pr, const char* name)
{
	parent = pr;
	for (int i = 0; i < 63; i++)
	{
		if (name[i] == 0)
		{
			Name[i] = 0;
			break;
		}
		Name[i] = name[i];
	}
	Name[63] = 0;
}

Core::DataStructure::Node::~Node()
{
}

void Core::DataStructure::Node::SetLocalPosition(const Core::Maths::Vec3D& value)
{
	transform.position = value;
}

void Core::DataStructure::Node::SetLocalRotation(const Core::Maths::Vec3D& value)
{
	transform.rotation = value;
}

void Core::DataStructure::Node::SetLocalScale(const Core::Maths::Vec3D& value)
{
	transform.scale = value;
}

const Core::Maths::Mat4D* Core::DataStructure::Node::GetLocalMatrix() const
{
	return &transform.local;
}

const Core::Maths::Mat4D* Core::DataStructure::Node::GetGlobalMatrix() const
{
	return &transform.global;
}

Core::Maths::Vec3D Core::DataStructure::Node::GetLocalPosition() const
{
	return transform.position;
}

Core::Maths::Vec3D Core::DataStructure::Node::GetLocalRotation() const
{
	return transform.rotation;
}

Core::Maths::Vec3D Core::DataStructure::Node::GetLocalScale() const
{
	return transform.scale;
}

void Core::DataStructure::Node::Refresh()
{
	transform.Update();
	transform.local = Core::Maths::Mat4D::CreateTransformMatrix(transform.position, transform.rotation, transform.scale);
	transform.global = transform.local;
	Node* local = parent;
	while (local)
	{
		transform.global = local->transform.local * transform.global;
		local = local->parent;
	}
}

void Core::DataStructure::Node::RefreshChilds()
{
	for (unsigned int i = 0; i < childs.size(); i++)
	{
		childs[i]->Refresh();
		childs[i]->RefreshChilds();
	}
}

void Core::DataStructure::Node::Render(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& vp, const std::vector<Core::Maths::Mat4D>* lvp, RenderMode renderMode)
{
	for (unsigned int i = 0; i < components.size(); i++)
	{
		components[i]->Render(VAOCurrent, shaderProgramCurrent, vp, lvp, renderMode, NodeSelected);
	}
	for (unsigned int i = 0; i < childs.size(); i++)
	{
		childs[i]->Render(VAOCurrent, shaderProgramCurrent, vp, lvp, renderMode);
	}
	if (!App::App::IsDebugMode() || renderMode == RenderMode::DebugHalo) NodeSelected = false;
}

void Core::DataStructure::Node::Update(std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* manager, float deltaTime)
{
	Refresh();
	for (unsigned int i = 0; i < components.size(); i++)
	{
		components[i]->Update(this, cameras, resources, textureManager, manager, deltaTime);
	}
	for (unsigned int i = 0; i < childs.size(); i++)
	{
		childs[i]->Update(cameras, resources, textureManager, manager, deltaTime);
	}
}

std::string Core::DataStructure::Node::GetPath()
{
	std::string out;
	for (Node* tmp = this; tmp->parent; tmp = tmp->parent)
	{
		if (tmp != this) out = "/" + out;
		out = tmp->Name + out;
	}
	return out;
}

void Core::DataStructure::Node::Destroy()
{
	for (size_t i = 0; i < childs.size(); i++)
	{
		childs[i]->Destroy();
		delete childs[i];
	}
	childs.clear();
	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->DeleteComponent();
		delete components[i];
	}
	components.clear();
}

void Core::DataStructure::Node::Serialize(std::ofstream& fileOut, unsigned int rec) const
{
	for (size_t i = 0; i < components.size(); i++)
	{
		Parsing::Shift(fileOut, rec);
		fileOut << "NewComponent " << components[i]->GetName() << std::endl;
		components[i]->Serialize(fileOut, rec+1);
		Parsing::Shift(fileOut, rec);
		fileOut << "EndComponent " << std::endl;
	}
	for (size_t i = 0; i < childs.size(); i++)
	{
		Parsing::Shift(fileOut, rec);
		fileOut << "NewNode " << childs[i]->Name << std::endl;
		Parsing::Shift(fileOut, rec + 1);
		fileOut << "Position " << childs[i]->transform.position.x << " " << childs[i]->transform.position.y << " " << childs[i]->transform.position.z << std::endl;
		Parsing::Shift(fileOut, rec + 1);
		fileOut << "Rotation " << childs[i]->transform.rotation.x << " " << childs[i]->transform.rotation.y << " " << childs[i]->transform.rotation.z << std::endl;
		Parsing::Shift(fileOut, rec + 1);
		fileOut << "Scale " << childs[i]->transform.scale.x << " " << childs[i]->transform.scale.y << " " << childs[i]->transform.scale.z << std::endl;
		childs[i]->Serialize(fileOut, rec+1);
		Parsing::Shift(fileOut, rec);
		fileOut << "EndNode " << std::endl;
	}
}

void Core::DataStructure::Node::RenderGUI(Node** selectedNode, Node** hoveredNode, bool IsSelected, int rec)
{
	selected = (*selectedNode == this);
	if (selected) IsSelected = true;
	NodeSelected = IsSelected;
	bool color = false;
	bool hover = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
	if (*hoveredNode == this && hover)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 127, 127, 255));
		color = true;
	}
	else if (selected)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		color = true;
	}
	rec++;
	if (ImGui::ArrowButton("SelectArrowButton", opened ? ImGuiDir_Down : ImGuiDir_Right))
	{
		opened = !opened;
		*selectedNode = this;
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && hover)
	{
		*hoveredNode = this;
		opened = true;
	}
	ImGui::SameLine();
	if (ImGui::Selectable(Name, &selected))
	{
		*selectedNode = this;
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
	{
		if (hover) *hoveredNode = this;
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			for (int i = 0; i < 64; i++)
			{
				newName[i] = Name[i];
			}
			ImGui::OpenPopup("Rename");
		}
	}
	if (ImGui::BeginPopupModal("Rename"))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
		ImGui::InputText("Rename", newName, 64, ImGuiInputTextFlags_EnterReturnsTrue);
		if (ImGui::Button("Valid") || ImGui::IsKeyDown(ImGuiKey_Enter))
		{
			for (int i = 0; i < 64; i++)
			{
				Name[i] = newName[i];
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel") || ImGui::IsKeyDown(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();
		ImGui::EndPopup();
	}
	if (opened)
	{
		for (unsigned int i = 0; i < childs.size(); i++)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX()+15*rec);
			ImGui::PushID(i);
			childs[i]->RenderGUI(selectedNode, hoveredNode, selected, rec);
			ImGui::PopID();
		}
	}
	if (color) ImGui::PopStyleColor();
}

void Core::DataStructure::Node::RenderTransform()
{
	transform.DrawGUI();
}

void Core::DataStructure::SceneNode::UpdatePhysics(std::vector<Physics::Colliders::Collider*>* colliders, float deltaTime)
{
	PhysicUpdate(colliders, deltaTime);
}

void Core::DataStructure::Node::PhysicUpdate(std::vector<Physics::Colliders::Collider*>* colliders, float deltaTime)
{
	for (unsigned int i = 0; i < components.size(); i++)
	{
		components[i]->PhysicUpdate(this, colliders, deltaTime);
	}
	for (unsigned int i = 0; i < childs.size(); i++)
	{
		childs[i]->PhysicUpdate(colliders, deltaTime);
	}
}

void Core::DataStructure::Node::PreUpdate()
{
	for (unsigned int i = 0; i < components.size(); i++)
	{
		components[i]->PreUpdate();
	}
	for (unsigned int i = 0; i < childs.size(); i++)
	{
		childs[i]->PreUpdate();
	}
}

void Core::DataStructure::Node::RenderGameUI(unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs)
{
	for (unsigned int i = 0; i < components.size(); i++)
	{
		components[i]->RenderGameUI(this, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, MouseInputs);
	}
	for (unsigned int i = 0; i < childs.size(); i++)
	{
		childs[i]->RenderGameUI(VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, MouseInputs);
	}
}
