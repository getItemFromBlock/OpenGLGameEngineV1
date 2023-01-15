#include "Core/Debug/FrameGraph.hpp"

#include <ImGUI/imgui.h>

Core::Debug::FrameGraph::FrameGraph(Resources::Texture* output, const Core::Maths::Int2D size, float maxDelta)
{
	graph = output;
	width = size.x;
	height = size.y;
	delta = maxDelta;
	data = new Core::Maths::UChar4D[1llu * width * height];
	UpdateGraph();
}

Core::Debug::FrameGraph::FrameGraph()
{
}

Core::Debug::FrameGraph::~FrameGraph()
{
}

void Core::Debug::FrameGraph::Destroy()
{
	delete[] data;
}

void Core::Debug::FrameGraph::Update(float deltaTime)
{
	for (unsigned int i = 0; i < height; i++)
	{
		for (unsigned int j = 1; j < width; j++)
		{
			data[i * width + j - 1] = data[i * width + j];
		}
	}
	float value = Core::Maths::Util::minF(delta, deltaTime);
	Core::Maths::UChar4D color = Core::Maths::UChar4D(Core::Maths::Vec4D((value-1.0f/60)/delta, 1 - (value - 1.0f / 60) / delta, 0.0f));
	for (unsigned int i = 0; i < height; i++)
	{
		if (i < Core::Maths::Util::cut(1 - value / delta, 0, 1) * height)
			data[(i + 1) * width - 1] = Core::Maths::UChar4D();
		else
			data[(i + 1) * width - 1] = color;
	}
	if (!pixel)
	{
		data[((int)((1 - (1.0f / 60) / delta) * height) + 1) * width - 1] = Core::Maths::UChar4D(63, 63, 255);
		data[((int)((1 - (1.0f / 30) / delta) * height) + 1) * width - 1] = Core::Maths::UChar4D(255, 255, 63);
	}
	pixel = (pixel+1)%2;
	UpdateGraph();
}

void Core::Debug::FrameGraph::UpdateGraph()
{
	graph->Overwrite(&data[0].r, width, height);
}

void Core::Debug::FrameGraph::Print()
{
	ImGui::Image((ImTextureID)(size_t)graph->GetTextureID(), {(float)graph->GetTextureWidth(), (float)graph->GetTextureHeight()});
}