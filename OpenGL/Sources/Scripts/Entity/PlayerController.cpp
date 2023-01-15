#include "Scripts/Entity/PlayerController.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"

#include "Core/App/App.hpp"
#include "Core/App/Input.hpp"
#include "Physics/Colliders/Collider.hpp"
#include "Resources/ModelAnimation.hpp"

Scripts::Entity::PlayerController::PlayerController()
{
}

void Scripts::Entity::PlayerController::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources,
	Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	if (Core::App::App::GetPlayType() == Core::App::PlayType::Editor || !Enabled) return;
	EntityBase::Update(container, cameras, resources, textureManager, lightManager, DeltaTime);
	if (!body) return;
	if (!Camera) Camera = Core::App::App::GetMainCamera();
	if (!Model && ModelNode)
	{
		for (size_t i = 0; i < ModelNode->components.size(); i++)
		{
			LowRenderer::SkinnedModel* tmp = dynamic_cast<LowRenderer::SkinnedModel*>(ModelNode->components[i]);
			if (tmp)
			{
				Model = tmp;
				break;
			}
		}
		if (!Model)
		{
			LOG("Error, Unable to Get Skinned Model needed for PlayerController at %s", container->GetPath().append(":PlayerController").c_str());
			return;
		}
	}
	if (Model && !JumpAnim) JumpAnim = resources->Get<Resources::ModelAnimation>("Resources/Models/Anims/Jump.smd");
	if (Model && !WalkAnim) WalkAnim = resources->Get<Resources::ModelAnimation>("Resources/Models/Anims/Run.smd");
	if (Model && !IdleAnim) IdleAnim = resources->Get<Resources::ModelAnimation>("Resources/Models/Anims/Idle.smd");
	if (Model && !FlexAnim) FlexAnim = resources->Get<Resources::ModelAnimation>("Resources/Models/Anims/Idle2.smd");
	if (!Bindings) Bindings = Core::App::App::GetInputBindings();
	Camera->focus = container->GetGlobalMatrix()->operator*(Core::Maths::Vec3D(0,0.8f,0)).getVector();
	Core::Maths::Vec3D MovementOut = Core::Maths::Vec3D();
	Core::Maths::Mat4D InvMat = CurrentMat.CreateInverseMatrix();
	if (CameraDistanceIndex != 0) for (unsigned int i = 0; i < 4; i++)
	{
		unsigned int index = (i < 2) ? 2 : 0;
		if (ImGui::IsKeyDown(Bindings[i])) MovementOut[index] += (i % 2 == 0 ? 1.0f : -1.0f);
	}
	if (MovementOut.getLength() < 0.1f) MovementOut = Core::Maths::Vec3D();
	else if (MovementOut.getLength() > 1.0f) MovementOut = MovementOut.unitVector();
	float vy = (CurrentMat *body->Velocity).y;
	if (OnGround) vy = body->Gravity.y * DeltaTime;
	if (OnGround && Vel.unitVector().dotProduct(MovementOut.unitVector()) < -0.95f) Vel = Vel + Core::Maths::Vec3D(-Vel.z, 0, Vel.x) * 0.6f;
	Vel = Vel + MovementOut * DeltaTime * (OnGround ? (OnSlope ? Acceleration * 0.2f : Acceleration) : Acceleration * 0.2f);
	Vel.y = 0;
	if (Vel.getLength() > Speed) Vel = Vel.unitVector() * Speed;
	Core::Maths::Vec3D rVel = Core::Maths::Vec3D(Vel.x, vy, Vel.z);
	MovementDir = (Core::Maths::Mat4D::CreateYRotationMatrix(CameraRotY) * rVel).getVector();
	body->Velocity = OnSlope ? body->Velocity * 0.95f + (InvMat * MovementDir).getVector() * 0.1f : (InvMat * MovementDir).getVector();
	if (ImGui::IsKeyPressed(Bindings[static_cast<unsigned int>(Core::App::InputType::Camera)]))
	{
		CameraDistanceIndex = (CameraDistanceIndex + 1) % 3;
	}
	Camera->distance = CameraDistance[CameraDistanceIndex];
	ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
	for (unsigned int i = 0; i < 4; i++)
	{
		unsigned int index = (i < 2) ? 0 : 1;
		if (ImGui::IsKeyPressed(Bindings[i+static_cast<unsigned int>(Core::App::InputType::CameraFront)])) CameraRotDelta[index] += (i % 2 == 0 ? 22.5f : -22.5f);
		if (CameraDistanceIndex == 0 && ImGui::IsKeyDown(Bindings[i])) CameraRot[index] += (i % 2 == 0 ? 1.0f : -1.0f);
	}
	CameraRot = CameraRot + Core::Maths::Vec3D(-dt.y, dt.x, 0) * 0.1f;
	for (unsigned int i = 0; i < 3; i++)
	{
		if (CameraRotDelta[i] > 0.5f) CameraRot[i] += 2.5f;
		else if (CameraRotDelta[i] < -0.5f) CameraRot[i] += -2.5f;
		if (abs(CameraRotDelta[i]) < 0.1f)CameraRotDelta[i] = 0.0f;
		else
		{
			CameraRotDelta[i] -= (CameraRotDelta[i] > 0 ? 2.5f : -2.5f);
		}
	}
	Core::Maths::Mat4D CameraGlobalRot = InvMat * Core::Maths::Mat4D::CreateRotationMatrix(CameraRot);
	Camera->rotation = CameraGlobalRot.GetRotationFromTranslation(CameraGlobalRot.GetScaleFromTranslation());
	Camera->rotation = Core::Maths::Vec3D(Camera->rotation.y + 180, Camera->rotation.x, -Camera->rotation.z);
	if (OnGround)
	{
		if (Vel.getLength() >= Speed + 10) Vel = Vel * (1 - DeltaTime * 20.0f);
		else
		{
			if (Vel.getLength() > 0.1f)
			{
				if (Model->GetAnimation() != WalkAnim) Model->SetAnimation(WalkAnim, 1.0f, true);
				Model->SetAnimSpeed(Vel.getLength() * 0.1f);
				IdleTime = 0.0f;
			}
			else
			{
				if (Model->GetAnimation() != IdleAnim && Model->GetAnimation() != FlexAnim) Model->SetAnimation(IdleAnim, 0.5f, true);
				Model->SetAnimSpeed(0.5f);
				IdleTime += DeltaTime;
				if (IdleTime > 15.0f)
				{
					if (Model->GetAnimation() != FlexAnim) Model->SetAnimation(FlexAnim, 0.3f, true);
					Model->SetAnimSpeed(0.4f);
				}
			}
			if (MovementOut.getLength() > 0.1f)
			{
				CameraRotY = CameraRot.y;
			}
			Vel = Vel * (1 - DeltaTime * 4.8f);
			if (!OnSlope && ImGui::IsKeyDown(Bindings[static_cast<unsigned int>(Core::App::InputType::Jump)]))
			{
				OnGround = false;
				body->Velocity = body->Velocity + (InvMat * Core::Maths::Vec3D(0, JumpPower,0)).getVector();
				Model->SetAnimation(JumpAnim, 1.0f, false);
			}
		}
	}
	else
	{
		Model->SetAnimSpeed(1.0f);
	}
	Core::App::Inputs tmpInputs;
	tmpInputs.ScreenSize = Camera->GetResolution();
	Camera->Update(tmpInputs, 0);
}

void Scripts::Entity::PlayerController::OnCollision(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other, Core::Maths::Vec3D HitPoint, Core::Maths::Vec3D Direction)
{
	EntityBase::OnCollision(collider, other, HitPoint, Direction);
	if (other->LayerMask & 0x2) OnSlope = true;
	else OnSlope = false;
}

void Scripts::Entity::PlayerController::OnTrigger(Physics::Colliders::Collider* collider, Physics::Colliders::Collider* other)
{
	EntityBase::OnTrigger(collider, other);
}

void Scripts::Entity::PlayerController::PreUpdate()
{
	EntityBase::PreUpdate();
}

void Scripts::Entity::PlayerController::DeleteComponent()
{
	EntityBase::DeleteComponent();
	this->~PlayerController();
}

void Scripts::Entity::PlayerController::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new PlayerController());
}

void Scripts::Entity::PlayerController::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	EntityBase::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Speed"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Speed = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Acceleration"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Acceleration = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "JumpPower"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			JumpPower = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void Scripts::Entity::PlayerController::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	EntityBase::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Speed " << Speed << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Acceleration " << Acceleration << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "JumpPower " << JumpPower << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void Scripts::Entity::PlayerController::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	EntityBase::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat("Speed", &Speed, 0.1f);
	ImGui::DragFloat("Acceleration", &Acceleration, 0.1f);
	ImGui::DragFloat("Jump Power", &JumpPower, 0.1f);
	ImGui::DragFloat3("Camera Distances", &CameraDistance.x, 0.1f);
	ImGui::DragFloat3("Camera Rotation", &CameraRot.x, 0.1f);
}
