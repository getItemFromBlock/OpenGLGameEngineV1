#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <time.h>
#include <glad/glad.h>
#include <chrono>

#include "Core/Util/AnimatedModelLoader.hpp"
#include "Resources/ModelAnimation.hpp"
#include "Core/Util/ModelLoader.hpp"
#include "Core/Debug/Log.hpp"

#include "Core/Maths/Maths.hpp"

#include "Core/Util/TextHelper.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/Mesh.hpp"

#define MAX_SEARCH_INDEX 16ll

int64_t GetMatIndex(std::vector<std::string>* list, std::string key)
{
	for (size_t i = 0; i < list->size(); i++) if (list->at(i) == key) return i;
	return -1;
}

void Core::Util::AnimatedModelLoader::CalculateDefaultMats(std::vector<Bone>* bones)
{
	for (size_t i = 0; i < bones->size(); i++)
	{
		bones->at(i).defaultRot = AdaptRot(bones->at(i).defaultRot);
		if (bones->at(i).parent < 0)
		{
			bones->at(i).defaultMat = Core::Maths::Mat4D::CreateTransformMatrix(bones->at(i).defaultPos, bones->at(i).defaultRot, Core::Maths::Vec3D(1));
		}
		else
		{
			bones->at(i).defaultMat = bones->at(bones->at(i).parent).defaultMat * Core::Maths::Mat4D::CreateTransformMatrix(bones->at(i).defaultPos, bones->at(i).defaultRot, Core::Maths::Vec3D(1));
		}
		bones->at(i).invMat = bones->at(i).defaultMat.CreateInverseMatrix();
	}
}

void AddMaterial(std::string path, const std::string& name, const std::string& TexName, Core::Util::ALoaderData* args)
{
	Resources::Texture::SetFilterType(GL_LINEAR);
	Resources::Texture::SetWrapType(GL_REPEAT);
	path = path.append(":");
	path = path.append(name);
	size_t tmpIndex = args->materialManager->CreateMaterial(args->manager, path.c_str());
	args->mats->push_back(args->materialManager->GetMaterials().at(tmpIndex));
	args->mats->back()->SetTexture(args->manager, args->textureManager, TexName.c_str(), true);
	args->matIds->push_back(TexName);
	args->indexesOut->push_back(std::vector<unsigned int>());
}

bool Core::Util::AnimatedModelLoader::Loop(const char* path, const char* data, const int64_t& size, Core::Util::ALoaderData* args)
{
	int64_t currentMat = -1;
	unsigned int vIndex = 0;
	std::vector<Bone>* bones = args->bones;
	std::vector<AttachedVertice>* verticesOut = args->verticesOut;
	std::vector<Resources::Material*>* mats = args->mats;
	std::vector<std::string>* matIds = args->matIds;
	std::vector<std::vector<unsigned int>>* indexes = args->indexesOut;
	int64_t pos = 0;
	int64_t line = 1;
	if (Text::compareWord(data, pos, size, "version"))
	{
		pos = Text::skipCharSafe(data, pos, size);
		LOG("SMD File Version : %d", Text::getInt(data, pos, size));
		pos = Text::endLine(data, pos, size);
		line++;
	}
	if (!Text::compareWord(data, pos, size, "nodes"))
	{
		LOG("Error, incorrect file formatting at line %ld", line);
		return false;
	}
	pos = Text::endLine(data, pos, size);
	line++;
	while (!Text::compareWord(data, pos, size, "end") && pos < size)
	{
		Bone tmp = Bone();
		tmp.id = (int)Text::getInt(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		tmp.name = Text::getTextQuoted(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		tmp.parent = (int)Text::getInt(data, pos, size);
		bones->push_back(tmp);
		pos = Text::endLine(data, pos, size);
		line++;
	}
	pos = Text::endLine(data, pos, size);
	line++;
	if (!Text::compareWord(data, pos, size, "skeleton"))
	{
		LOG("Error, incorrect file formatting at line %ld", line);
		return false;
	}
	pos = Text::endLine(data, pos, size);
	line++;
	while (!Text::compareWord(data, pos, size, "end") && pos < size)
	{
		int id = (int)Text::getInt(data, pos, size);
		if (id < 0 || id >= bones->size())
		{
			LOG("Error, invalid bone id at line %ld", line);
			return false;
		}
		pos = Text::skipCharSafe(data, pos, size);
		Parsing::ReadVec3D(data, pos, size, bones->at(id).defaultPos);
		pos = Text::skipCharSafe(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		Parsing::ReadVec3D(data, pos, size, bones->at(id).defaultRot);
		pos = Text::endLine(data, pos, size);
		line++;
		if (Text::compareWord(data, pos, size, "time")) // We skip the animation for now
		{
			while (!Text::compareWord(data, pos, size, "end") && pos < size)
			{
				pos = Text::endLine(data, pos, size);
				line++;
			}
		}
	}
	pos = Text::endLine(data, pos, size);
	line++;
	CalculateDefaultMats(bones);
	if (!Text::compareWord(data, pos, size, "triangles"))
	{
		LOG("Error, incorrect file formatting at line %ld", line);
		return false;
	}
	pos = Text::endLine(data, pos, size);
	line++;
	while (!Text::compareWord(data, pos, size, "end") && pos < size) // Now things become real
	{
		std::string tex = "Resources\\Textures\\";
		std::string name = Text::getText(data, pos, size);
		tex.append(name);
		if (currentMat == -1)
		{
			AddMaterial(path, name, tex, args);
			currentMat = 0;
		}
		else if (matIds->at(currentMat).compare(tex))
		{
			currentMat = GetMatIndex(matIds, tex);
			if (currentMat == -1)
			{
				AddMaterial(path, name, tex, args);
				currentMat = mats->size() - 1;
			}
		}
		pos = Text::endLine(data, pos, size);
		line++;
		for (unsigned int i = 0; i < 3 && pos < size; i++)
		{
			int boneID = (int)Text::getInt(data, pos, size);
			if (boneID < 0 || boneID >= bones->size())
			{
				LOG("Error, invalid bone id at line %ld", line);
				return false;
			}
			AttachedVertice tmp = {0};
			pos = Text::skipCharSafe(data, pos, size);
			Parsing::ReadVec3D(data, pos, size, tmp.pos);
			pos = Text::skipCharSafe(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			Parsing::ReadVec3D(data, pos, size, tmp.norm);
			pos = Text::skipCharSafe(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			Parsing::ReadVec2D(data, pos, size, tmp.UV);
			pos = Text::skipCharSafe(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			int AttCount = (int)Text::getInt(data, pos, size);
			if (AttCount < 1)
			{
				LOG("Error, invalid bone attachment count at line %ld", line);
				return false;
			}
			else if (AttCount > 6)
			{
				LOG("Error, too much attachments (max 6) at line %ld", line);
				return false;
			}
			for (int a = 0; a < AttCount && pos < size; a++)
			{
				pos = Text::skipCharSafe(data, pos, size);
				tmp.BoneIDs[a] = (int)Text::getInt(data, pos, size);
				pos = Text::skipCharSafe(data, pos, size);
				tmp.Weights[a] = Text::getFloat(data, pos, size);
			}
			verticesOut->push_back(tmp);
			indexes->at(currentMat).push_back(vIndex);
			vIndex++;
			pos = Text::endLine(data, pos, size);
			line++;
		}
		Core::Maths::Vec3D A = verticesOut->at(verticesOut->size() - 2).pos - verticesOut->at(verticesOut->size() - 1).pos;
		Core::Maths::Vec3D B = verticesOut->at(verticesOut->size() - 3).pos - verticesOut->at(verticesOut->size() - 1).pos;
		Core::Maths::Vec2D DeltaA = verticesOut->at(verticesOut->size() - 2).UV - verticesOut->at(verticesOut->size() - 1).UV;
		Core::Maths::Vec2D DeltaB = verticesOut->at(verticesOut->size() - 3).UV - verticesOut->at(verticesOut->size() - 1).UV;
		Core::Maths::Vec3D tangent;
		float f = 1.0f / (DeltaA.x * DeltaB.y - DeltaB.x * DeltaA.y);
		tangent.x = f * (DeltaB.y * A.x - DeltaA.y * B.x);
		tangent.y = f * (DeltaB.y * A.y - DeltaA.y * B.y);
		tangent.z = f * (DeltaB.y * A.z - DeltaA.y * B.z);
		tangent = tangent.unitVector();
		for (int n = 0; n < 3; n++)
		{
			verticesOut->at(verticesOut->size() - 1 - n).tang = tangent;
		}
	}
	return true;
}

bool Core::Util::AnimatedModelLoader::AnimLoop(const char* path, const char* data, const int64_t& size, Resources::ModelAnimation& in)
{
	std::vector<Resources::BoneInfo>* AnimData = &in.AnimData;
	int64_t pos = 0;
	int64_t line = 1;
	if (Text::compareWord(data, pos, size, "version"))
	{
		pos = Text::skipCharSafe(data, pos, size);
		LOG("SMD File Version : %d", Text::getInt(data, pos, size));
		pos = Text::endLine(data, pos, size);
		line++;
	}
	if (!Text::compareWord(data, pos, size, "nodes"))
	{
		LOG("Error, incorrect file formatting at line %ld", line);
		return false;
	}
	pos = Text::endLine(data, pos, size);
	line++;
	std::vector<Bone> bones;
	while (!Text::compareWord(data, pos, size, "end") && pos < size)
	{
		Bone tmp = Bone();
		tmp.id = (int)Text::getInt(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		tmp.name = Text::getTextQuoted(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		tmp.parent = (int)Text::getInt(data, pos, size);
		bones.push_back(tmp);
		pos = Text::endLine(data, pos, size);
		line++;
	}
	in.Bones = (unsigned int)bones.size();
	pos = Text::endLine(data, pos, size);
	line++;
	if (!Text::compareWord(data, pos, size, "skeleton"))
	{
		LOG("Error, incorrect file formatting at line %ld", line);
		return false;
	}
	pos = Text::endLine(data, pos, size);
	line++;
	pos = Text::endLine(data, pos, size);
	line++;
	while (!Text::compareWord(data, pos, size, "time") &&! Text::compareWord(data, pos, size, "end") && pos < size)
	{
		pos = Text::endLine(data, pos, size);
		line++;
	}
	bool first = false;
	int timer = -1;
	while (!Text::compareWord(data, pos, size, "end") && pos < size)
	{
		if (Text::compareWord(data, pos, size, "time"))
		{
			pos = Text::skipCharSafe(data, pos, size);
			timer = (int)Text::getInt(data, pos, size);
			pos = Text::endLine(data, pos, size);
			line++;
			AnimData->resize((timer) * bones.size());
			if (timer > 1)
			{
				for (size_t j = 0; j < bones.size(); j++)
				{
					AnimData->at((timer-1ll) * bones.size() + j) = AnimData->at((timer - 2ll) * bones.size() + j);
				}
			}
		}
		int id = (int)Text::getInt(data, pos, size);
		if (id < 0 || id >= bones.size())
		{
			LOG("Error, invalid bone id at line %ld", line);
			return false;
		}
		pos = Text::skipCharSafe(data, pos, size);
		Parsing::ReadVec3D(data, pos, size, AnimData->at((timer-1ll) * bones.size() + id).Position);
		pos = Text::skipCharSafe(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		pos = Text::skipCharSafe(data, pos, size);
		Parsing::ReadVec3D(data, pos, size, AnimData->at((timer-1ll) * bones.size() + id).Rotation);
		AnimData->at((timer-1ll) * bones.size() + id).Rotation = AdaptRot(AnimData->at((timer-1ll) * bones.size() + id).Rotation);

		pos = Text::endLine(data, pos, size);
		line++;
	}
	in.Frames = (unsigned int)in.AnimData.size() / in.Bones;
	if (in.Frames > 0) in.Frames = in.Frames-1;
	return true;
}

Core::Maths::Vec3D Core::Util::AnimatedModelLoader::AdaptRot(Core::Maths::Vec3D in)
{
	// XZY
	// ZXY
	Core::Maths::Mat4D tmp = Core::Maths::Mat4D::CreateZRotationMatrix(Core::Maths::Util::toDegrees(in.z)) * Core::Maths::Mat4D::CreateYRotationMatrix(Core::Maths::Util::toDegrees(in.y)) * Core::Maths::Mat4D::CreateXRotationMatrix(Core::Maths::Util::toDegrees(in.x));
	Core::Maths::Vec3D res = tmp.GetRotationFromTranslation(tmp.GetScaleFromTranslation());
	res.x = Core::Maths::Util::mod(res.x, 360.0f);
	res.y = Core::Maths::Util::mod(res.y, 360.0f);
	res.z = Core::Maths::Util::mod(res.z, 360.0f);
	return res;
}

void Core::Util::AnimatedModelLoader::MeshLoaderThread(LowRenderer::SkinnedModel& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager)
{
	if (!path) return;
	std::vector<std::string> matIds;
	std::vector<Resources::Material*> tmats;
	std::vector<AttachedVertice> verticesOut;
	std::vector<Bone> bones;
	std::vector<std::vector<unsigned int>> indexesOut;
	int64_t size = 0;
	const char* data = ModelLoader::loadFile(path, size);
	if (data == nullptr)
	{
		LOG("Error, cannot open file %s", path);
		return;
	}

	std::string newPath;
	unsigned int index = 0;
	for (unsigned int i = 0; path[i] != 0; i++) if (path[i] == '/' || path[i] == '\\') index = i + 1;
	for (unsigned int i = 0; i < index; i++) newPath += path[i];
	ALoaderData args = {
		manager,
		materialManager,
		textureManager,
		&tmats,
		&matIds,
		&bones,
		&verticesOut,
		&indexesOut
	};
	if (!Loop(newPath.c_str(), data, size, &args))
	{
		delete[] data;
		return;
	}

	unsigned int VBO = 0;
	for (unsigned int i = 0; i < matIds.size(); i++)
	{
		std::string fullPath = path;
		fullPath.append(":");
		fullPath.append(args.matIds->at(i));
		fullPath.erase(fullPath.find_last_of("."));

		Resources::Mesh* current = manager->Create<Resources::Mesh>(fullPath.c_str());
		current->loaded = false;
		in.AddMesh(current, manager, args.mats->at(i));
		modelManager->AddMesh(current);
		if (i == 0)
		{
			glGenBuffers(1, &current->VBO);
			VBO = current->VBO;
		}
		else
		{
			current->VBO = VBO;
		}
		glGenBuffers(1, &current->EBO);
		glGenVertexArrays(1, &current->VAO);

		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(current->VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexesOut[i].size() * sizeof(unsigned int), indexesOut[i].data(), GL_STATIC_DRAW);
		current->indexCount = (unsigned int)indexesOut[i].size();
		glBindBuffer(GL_ARRAY_BUFFER, current->VBO);

		if (i == 0) glBufferData(GL_ARRAY_BUFFER, verticesOut.size() * sizeof(AttachedVertice), verticesOut.data(), GL_STATIC_DRAW);
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AttachedVertice), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AttachedVertice), (void*)(sizeof(Core::Maths::Vec3D)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AttachedVertice), (void*)(2 * sizeof(Core::Maths::Vec3D)));
		glEnableVertexAttribArray(2);
		//Tangent data
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AttachedVertice), (void*)(2 * sizeof(Core::Maths::Vec3D) + sizeof(Core::Maths::Vec2D)));
		glEnableVertexAttribArray(3);
		//Bone ids attribute
		glVertexAttribIPointer(4, 3, GL_INT, sizeof(AttachedVertice), (void*)(3 * sizeof(Core::Maths::Vec3D) + sizeof(Core::Maths::Vec2D)));
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(5, 3, GL_INT, sizeof(AttachedVertice), (void*)(3 * sizeof(Core::Maths::Vec3D) + sizeof(Core::Maths::Vec2D) + 3 * sizeof(int)));
		glEnableVertexAttribArray(5);
		//Bone weights attribute
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(AttachedVertice), (void*)(3 * sizeof(Core::Maths::Vec3D) + sizeof(Core::Maths::Vec2D) + 6 * sizeof(int)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(AttachedVertice), (void*)(4 * sizeof(Core::Maths::Vec3D) + sizeof(Core::Maths::Vec2D) + 6 * sizeof(int)));
		glEnableVertexAttribArray(7);

		current->loaded = true;
	}
	for (size_t i = 0; i < tmats.size(); i++)
	{
		in.materials.push_back(*tmats[i]);
	}
	for (size_t i = 0; i < bones.size(); i++)
	{
		LowRenderer::BoneData tmp;
		tmp.ParentIndex = bones[i].parent;
		tmp.name = bones[i].name;
		tmp.defaultPos = bones[i].defaultPos;
		tmp.defaultRot = bones[i].defaultRot;
		tmp.BoneMatrix = Core::Maths::Mat4D::CreateTransformMatrix(tmp.defaultPos, tmp.defaultRot, Core::Maths::Vec3D(1));
		tmp.DefaultInvMat = bones[i].invMat;
		tmp.Parent = nullptr;
		if (i != bones[i].id)
		{
			if (i >= in.Bones.size())
			{
				while (i > in.Bones.size()) in.Bones.push_back(LowRenderer::BoneData());
				in.Bones.push_back(tmp);
			}
			else
			{
				in.Bones[i] = tmp;
			}
		}
		else
		{
			if (in.Bones.size() == i)
				in.Bones.push_back(tmp);
			else
				in.Bones[i] = tmp;
		}
	}
	LOG("Successfully loaded animated model %s", path);
	delete[] data;
	return;
}

void Core::Util::AnimatedModelLoader::AnimLoaderThread(Resources::ModelAnimation& in, const char* path)
{
	if (!path) return;
	int64_t size = 0;
	const char* data = ModelLoader::loadFile(path, size);
	if (data == nullptr)
	{
		LOG("Error, cannot open file %s", path);
		return;
	}

	if (AnimLoop(path, data, size, in))
	{
		LOG("Successfully loaded animation %s", path);
	}
	delete[] data;
	return;
}

void Core::Util::AnimatedModelLoader::LoadAnimatedMesh(LowRenderer::SkinnedModel& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager)
{
	auto begin = std::chrono::high_resolution_clock::now();

	MeshLoaderThread(in, path, manager, materialManager, textureManager, modelManager);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	long unsigned int total = 0;
	LOG("Time measured: %.6f seconds (%.3f miliseconds)", elapsed.count() * 1e-9, elapsed.count() * 1e-6);
}

void Core::Util::AnimatedModelLoader::LoadAnimation(Resources::ModelAnimation& in, const char* path)
{
	auto begin = std::chrono::high_resolution_clock::now();

	AnimLoaderThread(in, path);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	long unsigned int total = 0;
	LOG("Time measured: %.6f seconds (%.3f miliseconds)", elapsed.count() * 1e-9, elapsed.count() * 1e-6);
}