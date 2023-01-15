#include "LowRenderer/GameUI/UIBindingButton.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

const char* const InputTypeDesc[] =
{
	"Move Forward",
	"Move Backward",
	"Move Left",
	"Move Right",
	"Jump",
	"Crouch",
	"Change Camera View",
	"Move Camera Forward",
	"Move Camera Backward",
	"Move Camera Right",
	"Move Camera Left",
};

LowRenderer::GameUI::UIBindingButton::UIBindingButton()
{
}

LowRenderer::GameUI::UIBindingButton::~UIBindingButton()
{
}

void LowRenderer::GameUI::UIBindingButton::DeleteComponent()
{
	UIButton::DeleteComponent();
	this->~UIBindingButton();
}

void LowRenderer::GameUI::UIBindingButton::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new UIBindingButton());
}

void LowRenderer::GameUI::UIBindingButton::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	UIButton::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent") || Text::compareWord(data, pos, size, "EndSubComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "Delay"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Delay = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "InputType"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode >= static_cast<unsigned char>(Core::App::InputType::All))
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"InputType\" : valid numbers are [0-10]", mode);
				err = buff;
				break;
			}
			Type = static_cast<Core::App::InputType>(mode);
		}
		else if (Text::compareWord(data, pos, size, "WaitColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, WaitColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::GameUI::UIBindingButton::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	UIButton::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "Delay " << Delay << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "InputType " << static_cast<unsigned int>(Type) << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "WaitColor " << WaitColor.x << " " << WaitColor.y << " " << WaitColor.z << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}

void LowRenderer::GameUI::UIBindingButton::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	UIButton::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	ImGui::ColorEdit3("Wait Color", &WaitColor.x);
	ImGui::DragFloat("Max Delay", &Delay, 0.1f);
	if (Delay < 0.1f)
	{
		Delay = 0.1f;
	}
	ImGui::Combo("Input Type", (int*)&Type, InputTypeDesc, 11, -1);
}

void LowRenderer::GameUI::UIBindingButton::RenderGameUI(Core::DataStructure::Node* container, unsigned int& VAOCurrent, Resources::ShaderProgram** shaderProgramCurrent, const Core::Maths::Mat4D& v, const Core::Maths::Vec2D ScreenRes, const Core::Maths::Vec2D MousePos, float ScrollValue, unsigned int Inputs)
{
	if (!Enabled || !Plane) return;
	if (Label == "Text") UpdateKeyText();
	UIButton::RenderGameUI(container, VAOCurrent, shaderProgramCurrent, v, ScreenRes, MousePos, ScrollValue, Inputs);
}

void LowRenderer::GameUI::UIBindingButton::Update(Core::DataStructure::Node* container, std::vector<LowRenderer::Rendering::RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float DeltaTime)
{
	UIButton::Update(container, cameras, resources, textureManager, lightManager, DeltaTime);
	if (Counter >= 0.0f)
	{
		int key = Core::App::App::GetLastKeyPressed();
		if (key)
		{
			Core::App::App::GetInputBindings()[static_cast<unsigned int>(Type)] = key;
			Counter = Delay;
		}
		if (Counter >= Delay)
		{
			BaseColor = TmpColor[0];
			HoverColor = TmpColor[1];
			ClickColor = TmpColor[2];
			Counter = -1.0f;
			UpdateKeyText();
			return;
		}
		Counter += DeltaTime;
	}
}

void LowRenderer::GameUI::UIBindingButton::OnClick()
{
	if (Counter >= 0.0f) return;
	TmpColor[0] = BaseColor;
	TmpColor[1] = HoverColor;
	TmpColor[2] = ClickColor;
	BaseColor = WaitColor;
	HoverColor = WaitColor;
	ClickColor = WaitColor;
	Core::App::App::ClearLastKeyPressed();
	Counter = 0.0f;
	Label.clear();
	Label = "...";
	UIText::UpdateText();
}

void LowRenderer::GameUI::UIBindingButton::UpdateKeyText()
{
	Label.clear();
	Label = ImGui::GetKeyName(Core::App::App::GetInputBindings()[static_cast<unsigned int>(Type)]);
	UIText::UpdateText();
}
