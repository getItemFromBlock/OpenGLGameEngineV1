#pragma once

#include "LowRenderer/Rendering/Camera.hpp"

namespace LowRenderer::Rendering
{
	class EditorCamera : public Camera
	{
	public:
		float distance = 1.0f;

		float MovementSpeed = 40.0f;
		float RotationSpeed = 0.12f;

		void RenderGUI() override;
		void Update(const Core::App::Inputs& inputs, const float deltaTime) override;
	private:

	};
}