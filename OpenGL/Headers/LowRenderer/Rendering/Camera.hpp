#pragma once

#include "Core/Maths/Maths.hpp"

#include "Core/App/Input.hpp"

namespace LowRenderer::Rendering
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		Core::Maths::Vec3D position;
		Core::Maths::Vec3D focus;
		Core::Maths::Vec3D up;
		Core::Maths::Vec3D rotation;
		float fov = 60;
		float nearPlane = 0.1f;
		float farPlane = 600.0f;

		virtual void RenderGUI();
		virtual void Update(const Core::App::Inputs& inputs, const float deltaTime);
		virtual void Update(Core::Maths::Int2D resolution, Core::Maths::Vec3D position, Core::Maths::Vec3D forward, Core::Maths::Vec3D up);
		Core::Maths::Mat4D GetViewMatrix();
		Core::Maths::Mat4D GetProjectionMatrix();
		Core::Maths::Mat4D GetOrthoMatrix();

		Core::Maths::Int2D GetResolution() const { return Resolution; }

	protected:
		Core::Maths::Int2D Resolution;
		Core::Maths::Vec3D deltaUp;
		float aspect_ratio;
	};
}