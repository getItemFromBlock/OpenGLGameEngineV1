#include "Resources/ModelAnimation.hpp"

#include "Core/Util/AnimatedModelLoader.hpp"

Resources::ModelAnimation::ModelAnimation()
{
}

Resources::ModelAnimation::~ModelAnimation()
{
}

void Resources::ModelAnimation::Load(const char* path)
{
	fullPath = path;
	int startIndex = 0;
	char tmp;
	for (int i = 0; i < 255; i++)
	{
		tmp = path[i];
		if (tmp == 0)
		{
			break;
		}
		else if (tmp == '\\' || tmp == '/')
		{
			startIndex = i + 1;
		}
	}
	tmp = path[startIndex];
	int index = 0;
	for (int i = startIndex + 1; i < 255 && tmp != 0 && tmp != '.'; i++)
	{
		Name[index] = tmp;
		tmp = path[i];
		index++;
	}
	Name[index] = 0;

	Core::Util::AnimatedModelLoader::LoadAnimation(*this, path);
}

void Resources::ModelAnimation::UnLoad()
{
	this->~ModelAnimation();
}

const char* Resources::ModelAnimation::GetPath()
{
	return fullPath.c_str();
}

const char* Resources::ModelAnimation::GetName()
{
	return Name;
}

std::vector<Resources::BoneInfo> Resources::ModelAnimation::GetInterpolatedBones(float time, bool looped)
{
	if (Frames == 0) return std::vector<BoneInfo>(); // GET BONED IDIOT
	time = time * 60.0f;
	int frame;
	int nextFrame;
	float delta;
	if (looped)
	{
		frame = (int)(time) % Frames;
		if (frame < 0) frame += Frames;
		nextFrame = (frame+1) % Frames;
		delta = Core::Maths::Util::mod(time, 1.0f);
	}
	else
	{
		frame = Core::Maths::Util::cutInt((int)time, 0, Frames - 2);
		nextFrame = (frame + 1) % Frames;
		delta = time < 0 ? 0.0f :(time > Frames - 2 ? 1.0f : Core::Maths::Util::mod(time, 1.0f));
	}
	if (delta < 0.0f) delta += 1.0f;
	std::vector<BoneInfo> out;
	for (unsigned int i = 0; i < Bones; i++)
	{
		BoneInfo tmp;
		tmp.id = i;
		tmp.Position = AnimData[frame * (int64_t)Bones + i].Position * (1-delta) + AnimData[nextFrame * (int64_t)Bones + i].Position * (delta);
		Core::Maths::Vec3D RotA = AnimData[frame * (int64_t)Bones + i].Rotation;
		Core::Maths::Vec3D RotB = AnimData[nextFrame * (int64_t)Bones + i].Rotation;
		for (int n = 0; n < 3; n++)
		{
			if (RotA[n] < 180.0f && RotB[n] > 180.0f && RotB[n] - RotA[n] > 180.0f) RotA[n] += 360.0f;
			if (RotB[n] < 180.0f && RotA[n] > 180.0f && RotA[n] - RotB[n] > 180.0f) RotB[n] += 360.0f;
		}
		tmp.Rotation = RotA * (1-delta) + RotB * (delta);
		out.push_back(tmp);
	}
	return out;
}

Resources::ModelAnimation& Resources::ModelAnimation::operator=(const ModelAnimation& other)
{
	if (this == &other)
		return *this;
	for (unsigned int i = 0; i < 256; i++)
	{
		Name[i] = other.Name[i];
	}
	fullPath = other.fullPath;
	AnimData = other.AnimData;
	Frames = other.Frames;
	Bones = other.Bones;
	return *this;
}
