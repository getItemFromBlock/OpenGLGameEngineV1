#pragma once

#include "Core/Maths/Maths.hpp"

namespace Physics
{
	class Transform
	{
	public:
		Transform();
		~Transform();

		Core::Maths::Mat4D local = Core::Maths::Mat4D::Identity();
		Core::Maths::Mat4D global = Core::Maths::Mat4D::Identity();

		Core::Maths::Vec3D position = Core::Maths::Vec3D();
		Core::Maths::Vec3D rotation = Core::Maths::Vec3D();
		Core::Maths::Vec3D scale = Core::Maths::Vec3D(1);

		void DrawGUI();

		void Update();
	private:

	};
}