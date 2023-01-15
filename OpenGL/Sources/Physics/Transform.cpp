#include "Physics/Transform.hpp"

#include <ImGUI/imgui.h>

Physics::Transform::Transform()
{
}

Physics::Transform::~Transform()
{
}

void Physics::Transform::DrawGUI()
{
	ImGui::DragFloat3("Position", &position.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
	ImGui::DragFloat3("Scale", &scale.x, 0.1f);
}

void Physics::Transform::Update()
{
	rotation = Core::Maths::Vec3D(Core::Maths::Util::mod(rotation.x, 360.0f), Core::Maths::Util::mod(rotation.y, 360.0f), Core::Maths::Util::mod(rotation.z, 360.0f));
}
