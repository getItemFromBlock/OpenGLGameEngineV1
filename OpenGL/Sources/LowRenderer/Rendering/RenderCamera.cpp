#include "LowRenderer/Rendering/RenderCamera.hpp"

#include <ImGUI/imgui.h>

#include "Core/DataStructure/Node.hpp"
#include "Resources/TextureManager.hpp"
#include "Core/Util/TextHelper.hpp"

using namespace Core::Maths;

const char* const BufferFilterDesc[2] =
{
	"Nearest",
	"Linear",
};

const char* const BufferWrapDesc[5] =
{
	"Repeat",
	"Mirrored Repeat",
	"Mirrored Clamp",
	"Clamp To Edge",
	"Clamp To Border",
};

LowRenderer::Rendering::RenderCamera::RenderCamera()
{
	Resolution = Int2D(256, 256);
}

LowRenderer::Rendering::RenderCamera::~RenderCamera()
{
}

void LowRenderer::Rendering::RenderCamera::Update(Core::DataStructure::Node* container, std::vector<RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
{
	if (!UUID[0])
	{
		for (unsigned int i = 0; i < 16; i++)
		{
			int c = (int)(rand() * 36.0f / RAND_MAX);
			UUID[i] = c < 10 ? c + '0' : c + 'A' - 10;
		}
		UUID[16] = 0;
	}
	if (!buffer)
	{
		std::string path = GetName();
		path.append("@");
		path.append(UUID);
		buffer = resources->Get<FrameBuffer>(path.c_str());
		if (buffer)
		{
			buffer->Update(Resolution.x, Resolution.y, Filter, Wrap);
		}
		else
		{
			buffer = resources->Create<FrameBuffer>(path.c_str());
			textureManager->AddFrameBuffer(buffer);
			buffer->Init(Resolution.x, Resolution.y, Filter, Wrap);
		}
	}
	if (!Enabled) return;
	Resolution = Int2D(Util::maxI(Resolution.x,1), Util::maxI(Resolution.y, 1));
	position = container->GetGlobalMatrix()->GetPositionFromTranslation();
	Vec3D cameraRot = container->GetGlobalMatrix()->GetRotationFromTranslation(container->GetGlobalMatrix()->GetScaleFromTranslation());
	rotation = Vec3D(cameraRot.y - 180, cameraRot.x, cameraRot.z);
	Core::App::Inputs i;
	i.ScreenSize = Resolution;
	Camera::Update(i,deltaTime);
	cameras->push_back(this);
}

void LowRenderer::Rendering::RenderCamera::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new RenderCamera());
}

void LowRenderer::Rendering::RenderCamera::DeleteComponent()
{
	Camera::~Camera();
	this->~RenderCamera();
}

void LowRenderer::Rendering::RenderCamera::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
	Camera::RenderGUI();
	ImGui::DragInt2("Camera Resolution", &tmpResolution.x, 1.0f, 1, 4096);
	ImGui::Combo("Buffer Filter Mode", (int*)&Filter, BufferFilterDesc, 2, -1);
	ImGui::Combo("Buffer Wrap Mode", (int*)&Wrap, BufferWrapDesc, 5, -1);
	ImGui::Checkbox("Clear Buffer", &ClearBuffer);
	if (ClearBuffer) ImGui::ColorEdit3("Clear Color", &ClearColor.x);
	bool apply = false;
	if ((!(Resolution == tmpResolution) || Filter != buffer->FilterType || Wrap != buffer->WrapType) && ImGui::Button("Apply"))
	{
		apply = true;
		Resolution = tmpResolution;
	}
	if (apply || !(Resolution == Int2D(buffer->sizeX, buffer->sizeY)))
	{
		buffer->Update(Resolution.x,Resolution.y, Filter, Wrap);
	}
}

void LowRenderer::Rendering::RenderCamera::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	Component::Deserialize(resources, shaders, data, pos, size, line, err);
	if (err.c_str()[0]) return;
	pos = Text::endLine(data, pos, size);
	line++;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "EndComponent"))
		{
			break;
		}
		else if (Text::compareWord(data, pos, size, "UPVector"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, up))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "FOV"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			fov = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Near"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			nearPlane = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Far"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			farPlane = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Resolution"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			Resolution.x = (int)Text::getInt(data,pos,size);
			pos = Text::skipCharSafe(data, pos, size);
			Resolution.y = (int)Text::getInt(data, pos, size);
			tmpResolution = Resolution;
		}
		else if (Text::compareWord(data, pos, size, "ClearBuffer"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadBool(data, pos, size, ClearBuffer))
			{
				err = "Unrecognized argument; valids are [True/False]";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "ClearColor"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			if (!Parsing::ReadVec3D(data, pos, size, ClearColor))
			{
				err = "Malformated Vec3";
				break;
			}
		}
		else if (Text::compareWord(data, pos, size, "CameraID"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string id = Text::getText(data, pos, size);
			if (!id.c_str()[0] || id.size() > 17)
			{
				err = "Expected valid UUID after \"UUID\"";
				break;
			}
			id.copy(UUID, 17);
		}
		else if (Text::compareWord(data, pos, size, "FilterType"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode > static_cast<unsigned char>(TextureFilterType::Linear))
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"FilterType\" : valid numbers are [0-1]", mode);
				err = buff;
				break;
			}
			Filter = static_cast<TextureFilterType>(mode);
		}
		else if (Text::compareWord(data, pos, size, "WrapType"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			int mode = (int)Text::getInt(data, pos, size);
			if (mode < 0 || mode > static_cast<unsigned char>(TextureWrapType::ClampToBorder))
			{
				char buff[96];
				snprintf(buff, 96, "Invalid number %d after \"WrapType\" : valid numbers are [0-4]", mode);
				err = buff;
				break;
			}
			Wrap = static_cast<TextureWrapType>(mode);
		}
		pos = Text::endLine(data, pos, size);
		line++;
	}
}

void LowRenderer::Rendering::RenderCamera::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	Component::Serialize(fileOut, rec);
	Parsing::Shift(fileOut, rec);
	fileOut << "UPVector " << up.x << " " << up.y << " " << up.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "FOV " << fov << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Near " << nearPlane << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Far " << farPlane << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "Resolution " << Resolution.x << " " << Resolution.y << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "ClearBuffer " << (ClearBuffer ? "True" : "False") << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "ClearColor " << ClearColor.x << " " << ClearColor.y << " " << ClearColor.z << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "CameraID " << UUID << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "FilterType " << static_cast<unsigned int>(Filter) << std::endl;
	Parsing::Shift(fileOut, rec);
	fileOut << "WrapType " << static_cast<unsigned int>(Wrap) << std::endl;
	Parsing::Shift(fileOut, rec - 1);
	fileOut << "EndSubComponent" << std::endl;
}
