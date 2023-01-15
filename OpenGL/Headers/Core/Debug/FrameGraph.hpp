#pragma once

#include "Resources/Texture.hpp"

#include "Core/Maths/Maths.hpp"

namespace Core::Debug
{
	class FrameGraph
	{
	public:
		FrameGraph(Resources::Texture* output, const Core::Maths::Int2D size, float maxDelta = 0.2f);
		FrameGraph();
		~FrameGraph();

		void Update(float deltaTime);
		void Destroy();

		void Print();
	private:
		Resources::Texture* graph = nullptr;
		unsigned int width = 0;
		unsigned int height = 0;
		Core::Maths::UChar4D* data = nullptr;
		char pixel = 0;
		float delta = 0;

		void UpdateGraph();
	};
}