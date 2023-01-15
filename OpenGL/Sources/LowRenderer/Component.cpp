#include "LowRenderer/Component.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"

void LowRenderer::Component::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Enabled"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, Enabled))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::Component::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Parsing::Shift(fileOut, rec);
	fileOut << "Enabled " << (Enabled ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::Component::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	ImGui::Checkbox("Enabled", &Enabled);
}