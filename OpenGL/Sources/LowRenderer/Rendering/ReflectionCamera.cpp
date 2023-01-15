#include "..\..\..\Headers\LowRenderer\Rendering\ReflectionCamera.hpp"

#include "Core/DataStructure/Node.hpp"
#include "Resources/TextureManager.hpp"
#include "Core/Util/TextHelper.hpp"
#include "Core/App/App.hpp"

LowRenderer::Rendering::ReflectionCamera::ReflectionCamera()
{
	PlaneNormal = Core::Maths::Vec3D(0, 0, 1);
	PlaneRight = Core::Maths::Vec3D(1, 0, 0);
	PlaneUp = Core::Maths::Vec3D(0, 1, 0);
}

LowRenderer::Rendering::ReflectionCamera::~ReflectionCamera()
{
}

void LowRenderer::Rendering::ReflectionCamera::Update(Core::DataStructure::Node* container, std::vector<RenderCamera*>* cameras, Resources::ResourceManager* resources, Resources::TextureManager* textureManager, LowRenderer::Lightning::LightManager* lightManager, float deltaTime)
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
	PlaneNormal = container->GetGlobalMatrix()->operator*(Core::Maths::Vec4D(0, 0, 1, 0)).getVector().unitVector();
	PlaneRight = container->GetGlobalMatrix()->operator*(Core::Maths::Vec4D(1, 0, 0, 0)).getVector().unitVector();
	PlaneUp = container->GetGlobalMatrix()->operator*(Core::Maths::Vec4D(0, 1, 0, 0)).getVector().unitVector();
	const LowRenderer::Rendering::EditorCamera* mCam = Core::App::App::GetMainCamera();
	if (!(Resolution == mCam->GetResolution()))
	{
		Resolution = mCam->GetResolution();
		aspect_ratio = Resolution.x * 1.0f / Resolution.y;
		buffer->Update(Resolution.x, Resolution.y, Filter, Wrap);
	}
	fov = mCam->fov;
	nearPlane = (mCam->position - container->GetGlobalMatrix()->GetPositionFromTranslation()).getLength();
	float dtX = (mCam->position - container->GetGlobalMatrix()->GetPositionFromTranslation()).dotProduct(PlaneRight);
	float dtY = (mCam->position - container->GetGlobalMatrix()->GetPositionFromTranslation()).dotProduct(PlaneUp);
	Core::Maths::Vec3D refPos = PlaneRight * dtX + PlaneUp * dtY + container->GetGlobalMatrix()->GetPositionFromTranslation();
	position = mCam->position + (refPos - mCam->position) * 2;
	Core::Maths::Vec3D dir = (mCam->focus - mCam->position).unitVector();
	dtX = dir.dotProduct(PlaneRight);
	dtY = dir.dotProduct(PlaneUp);
	Core::Maths::Vec3D refDir = PlaneRight * dtX + PlaneUp * dtY;
	focus = dir + (refDir - dir) * 2 + position;
	Core::Maths::Vec3D wUp = mCam->up;
	dtX = wUp.dotProduct(PlaneRight);
	dtY = wUp.dotProduct(PlaneUp);
	Core::Maths::Vec3D refUp = PlaneRight * dtX + PlaneUp * dtY;
	deltaUp = wUp + (refUp - wUp) * 2;
	cameras->push_back(this);
}

void LowRenderer::Rendering::ReflectionCamera::Create(Core::DataStructure::Node* container)
{
	container->components.push_back(new ReflectionCamera());
}

void LowRenderer::Rendering::ReflectionCamera::DeleteComponent()
{
	Camera::~Camera();
	this->~ReflectionCamera();
}

void LowRenderer::Rendering::ReflectionCamera::RenderGUI(Resources::ResourceManager* resources, Resources::ShaderManager* shaderManager, Resources::TextureManager* textureManager, Resources::MaterialManager* materialManager, Resources::MeshManager* modelManager)
{
	Component::RenderGUI(resources, shaderManager, textureManager, materialManager, modelManager);
}

void LowRenderer::Rendering::ReflectionCamera::Deserialize(Resources::ResourceManager* resources, Resources::ShaderManager* shaders, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	RenderCamera::Deserialize(resources, shaders, data, pos, size, line, err);
	Resolution = Core::Maths::Int2D(1200, 900);
}

void LowRenderer::Rendering::ReflectionCamera::Serialize(std::ofstream& fileOut, unsigned int rec)
{
	RenderCamera::Serialize(fileOut, rec);
}
