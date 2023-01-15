#include "LowRenderer/GameUI/UIElement.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"
#include "Core/DataStructure/Node.hpp"

const char* const YAnchorDesc[3] =
{
	"Centered",
	"Align Up",
	"Align Down",
};

const char* const XAnchorDesc[3] =
{
	"Centered",
	"Align Right",
	"Align Left",
};

LowRenderer::GameUI::UIElement::UIElement()
{
}

LowRenderer::GameUI::UIElement::~UIElement()
{
}

inline void LowRenderer::GameUI::UIElement::PreUpdate()
{
	if (Clicked && !Hovered)
	{
		Clicked = false;
	}
}

void LowRenderer::GameUI::UIElement::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
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
		else if (Text::compareWord(data, pos, size, "ElementPos"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, ElementPos))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "ElementSize"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec2D(data, pos, size, ElementSize))
			{
				err = "Malformated Vec2";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "ElementRot"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			ElementRot = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "AdaptToScreen"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, AdaptToScreen))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "Anchor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode > static_cast<unsigned int>(AnchorType::DownLeft) || mode == 0x3 || mode == 0x7)
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"Anchor\" : valid numbers are [0,1,2,4,5,6,8,9,10]", mode);
				err = buff;
				break;
			}
			Anchor = static_cast<AnchorType>(mode);
		}
		else if (Text::compareWord(data, pos, size, "Alpha"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Alpha = Text::getFloat(data, pos, size);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UIElement::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "ElementPos " << ElementPos.x << " " << ElementPos.y << " " << ElementPos.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "ElementSize " << ElementSize.x << " " << ElementSize.y << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "ElementRot " << ElementRot << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "AdaptToScreen " << (AdaptToScreen ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Anchor " << static_cast<unsigned int>(Anchor) << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Alpha " << Alpha << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UIElement::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::DragFloat3("Element Position", &ElementPos.x, 0.1f);
	ImGui::DragFloat2("Element Scale", &ElementSize.x, 0.1f);
	ImGui::DragFloat("Element Rotation", &ElementRot, 0.1f);
	ImGui::Checkbox("Adapt to Screen Size", &AdaptToScreen);
	int type = static_cast<unsigned int>(Anchor);
	int AY = type & 0x3;
	int AX = (type & 0xC) >> 2;
	ImGui::Combo("X Axis Anchor Type", &AX, XAnchorDesc, 3, -1);
	ImGui::Combo("Y Axis Anchor Type", &AY, YAnchorDesc, 3, -1);
	type = (AY & 0x3) | ((AX & 0x3) << 2);
	Anchor = static_cast<AnchorType>(type);
	ImGui::DragFloat("Alpha", &Alpha, 0.05f, 0.0f, 1.0f);
}

void LowRenderer::GameUI::UIElement::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int MouseInputs)
{
	if (!Enabled) return;
	Core::Maths::Vec2D MinC;
	float Ratio = ScreenRes.x / ScreenRes.y;
	Core::Maths::Vec2D Sz = ElementSize * Core::Maths::Vec2D(AdaptToScreen ? ScreenRes.x : ScreenRes.y, ScreenRes.y) * 0.5f;
	MinC.x = (ElementPos.x + (CompareAnchor(AnchorType::Left) ? 0 : (CompareAnchor(AnchorType::Right) ? 2 : 1))) * ScreenRes.x * 0.5f - Sz.x;
	MinC.y = (ElementPos.y + (CompareAnchor(AnchorType::Up) ? 0 : (CompareAnchor(AnchorType::Down) ? 2 : 1))) * ScreenRes.y * 0.5f - Sz.y;
	Core::Maths::Vec2D MaxC;
	MaxC.x = (ElementPos.x + (CompareAnchor(AnchorType::Left) ? 0 : (CompareAnchor(AnchorType::Right) ? 2 : 1))) * ScreenRes.x * 0.5f + Sz.x;
	MaxC.y = (ElementPos.y + (CompareAnchor(AnchorType::Up) ? 0 : (CompareAnchor(AnchorType::Down) ? 2 : 1))) * ScreenRes.y * 0.5f + Sz.y;
	ElementMat = Core::Maths::Mat4D::CreateTransformMatrix(Core::Maths::Vec3D(((MinC.x + Sz.x) / ScreenRes.x * 2 - 1) * Ratio, (1 - (MinC.y + Sz.y) / ScreenRes.y) * 2 - 1, ElementPos.z * 2 - 1), Core::Maths::Vec3D(0, 0, ElementRot), Core::Maths::Vec3D(Sz.x / ScreenRes.y * 2.0f, Sz.y / ScreenRes.y * 2.0f, 1));
	Hovered = MousePos.x >= MinC.x && MousePos.x <= MaxC.x && MousePos.y >= MinC.y && MousePos.y <= MaxC.y;
	if (Clicked && !(MouseInputs & static_cast<unsigned int>(Core::DataStructure::MouseInput::ALL_DOWN))) Clicked = false;
	if (!Clicked && Hovered && (MouseInputs & static_cast<unsigned int>(Core::DataStructure::MouseInput::ALL_PRESS)))
	{
		Clicked = true;
		OnClick();
	}
}

bool LowRenderer::GameUI::UIElement::CompareAnchor(AnchorType other)
{
	return (static_cast<unsigned int>(Anchor) & static_cast<unsigned int>(other));
}
