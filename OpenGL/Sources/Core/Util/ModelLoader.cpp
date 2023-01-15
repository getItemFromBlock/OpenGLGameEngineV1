#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <time.h>
#include <glad/glad.h>
#include <chrono>

#include "Core/Util/ModelLoader.hpp"
#include "Core/Debug/Log.hpp"

#include "Core/Maths/Maths.hpp"

#include "Core/Util/TextHelper.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/Mesh.hpp"

#define MAX_SEARCH_INDEX 16ll

Core::Util::Triangle::Triangle(unsigned int v[3], unsigned int n[3], unsigned int t[3])
{
	for (int i = 0; i < 3; i++)
	{
		vertice[i] = v[i];
		normal[i] = n[i];
		texture[i] = t[i];
	}
}

unsigned int GenerateVertices(Core::Util::LoaderData* args, unsigned int modelIndex, signed char type)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < args->tris->at(modelIndex).size(); i++)
	{
		for (int n = 0; n < 3; n++)
		{
			unsigned int vertice = args->tris->at(modelIndex).at(i).vertice[n];
			unsigned int normal = args->tris->at(modelIndex).at(i).normal[n];
			unsigned int texture = args->tris->at(modelIndex).at(i).texture[n];
			unsigned int indexes[3] = { vertice, normal, texture };
			bool found = false;
			int64_t max = args->verticesOut->size() > MAX_SEARCH_INDEX ? args->verticesOut->size() - MAX_SEARCH_INDEX : 0;
			for (int64_t j = args->verticesOut->size() - 1llu; j >= max; j--)
			{
				bool out = false;
				for (char u = 0; u < 3; u++)
				{
					if (indexes[u] != args->indexes->at(j).indexes[u])
					{
						out = true;
						break;
					}
				}
				if (!out)
				{
					found = true;
					args->indexesOut->at(modelIndex).push_back((unsigned int)j);
					count++;
					break;
				}
			}
			if (found) continue;
			Core::Util::Vertice data;
			Core::Util::VerticeIndex ind;
			for (char u = 0; u < 3; u++)
				ind.indexes[u] = indexes[u];
			data.pos = args->verts->at(vertice);
			if (type == -1 || type == 2)
			{
				data.norm = args->norms->at(normal);
			}
			if (type == 1 || type == 2)
			{
				data.UV = args->UVs->at(texture);
			}
			args->indexesOut->at(modelIndex).push_back((unsigned int)args->verticesOut->size());
			args->verticesOut->push_back(data);
			args->indexes->push_back(ind);
			count++;
		}
	}
	return count;
}

unsigned int GenerateVerticesUnoptimized(Core::Util::LoaderData* args, unsigned int modelIndex, signed char type)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < args->tris->at(modelIndex).size(); i++)
	{
		for (int n = 0; n < 3; n++)
		{
			unsigned int vertice = args->tris->at(modelIndex).at(i).vertice[n];
			unsigned int normal = args->tris->at(modelIndex).at(i).normal[n];
			unsigned int texture = args->tris->at(modelIndex).at(i).texture[n];
			Core::Util::Vertice data;
			data.pos = args->verts->at(vertice);
			if (type == -1 || type == 2)
			{
				data.norm = args->norms->at(normal);
			}
			if (type == 1 || type == 2)
			{
				data.UV = args->UVs->at(texture);
			}
			args->indexesOut->at(modelIndex).push_back((unsigned int)args->verticesOut->size());
			args->verticesOut->push_back(data);
			count++;
		}
	}
	return count;
}

unsigned int GenerateTangentVertices(Core::Util::LoaderData* args, unsigned int modelIndex, signed char type)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < args->tris->at(modelIndex).size(); i++)
	{
		Core::Util::TangentVertice data[3];
		for (int n = 0; n < 3; n++)
		{
			data[n].pos = args->verts->at(args->tris->at(modelIndex).at(i).vertice[n]);
			if (type == -1 || type == 2)
			{
				data[n].norm = args->norms->at(args->tris->at(modelIndex).at(i).normal[n]);
			}
			if (type == 1 || type == 2)
			{
				data[n].UV = args->UVs->at(args->tris->at(modelIndex).at(i).texture[n]);
			}
		}
		Core::Maths::Vec3D A = data[1].pos - data[0].pos;
		Core::Maths::Vec3D B = data[2].pos - data[0].pos;
		Core::Maths::Vec2D DeltaA = data[1].UV - data[0].UV;
		Core::Maths::Vec2D DeltaB = data[2].UV - data[0].UV;
		Core::Maths::Vec3D tangent;
		float f = 1.0f / (DeltaA.x * DeltaB.y - DeltaB.x * DeltaA.y);
		tangent.x = f * (DeltaB.y * A.x - DeltaA.y * B.x);
		tangent.y = f * (DeltaB.y * A.y - DeltaA.y * B.y);
		tangent.z = f * (DeltaB.y * A.z - DeltaA.y * B.z);
		tangent = tangent.unitVector();
		for (int n = 0; n < 3; n++)
		{
			data[n].tang = tangent;
			args->indexesOut->at(modelIndex).push_back((unsigned int)args->tverticesOut->size());
			args->tverticesOut->push_back(data[n]);
			count++;
		}
	}
	return count;
}

const char* Core::Util::ModelLoader::loadFile(const char* path, int64_t& length)
{
	std::ifstream in;
	in.open(path, std::ios::binary | std::ios::in | std::ios::ate);
	if (!in.is_open())
	{
		return nullptr;
	}
	LOG("Loading File %s", path);
	char* data;
	length = in.tellg();
	in.seekg(0, in.beg);

	data = new char[length + 1];
	in.read(data, length);
	in.close();
	data[length] = 0;
	return data;
}

void Core::Util::ModelLoader::ReadFace(int64_t& pos, size_t& objIndex, signed char& type, const char* data, const int64_t& size, Core::Util::LoaderData* args)
{
	std::vector<std::vector<Core::Util::Triangle>>* faces = args->tris;
	if (type == -10)
	{
		type = Text::countSlash(data, pos, size);
	}
	unsigned int v[3] = { 0 };
	unsigned int n[3] = { 0 };
	unsigned int t[3] = { 0 };
	if (type == -1)
	{
		v[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		pos += 1;
		n[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		pos += 1;
		n[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		pos += 1;
		n[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
	}
	else if (type == 0)
	{
		v[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
	}
	else if (type == 1)
	{
		v[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		t[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		t[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		t[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
	}
	else if (type == 2)
	{
		v[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		t[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		n[0] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		t[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		n[1] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		v[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		t[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
		pos = Text::skipCharSafe(data, pos, size);
		n[2] = (unsigned int)Text::getInt(data, pos, size) - 1;
	}
	faces->at(objIndex).push_back(Triangle(v, n, t));
}

signed char Core::Util::ModelLoader::Loop(const char* path, const char* data, const int64_t& size, Core::Util::LoaderData* args)
{
	std::vector<std::vector<Triangle>>* faces = args->tris;
	std::vector<Core::Maths::Vec3D>* vertices = args->verts;
	std::vector<Core::Maths::Vec3D>* normals = args->norms;
	std::vector<Core::Maths::Vec2D>* tCoord = args->UVs;
	size_t objIndex = 0;
	int64_t pos = 0;
	signed char type = -10;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "# GenerateTangent"))
		{
			CreateTangent = true;
		}
		else if (Text::compareWord(data, pos, size, "mtllib "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string path2 = path;
			path2 = path2.append(Text::getText(data, pos, size));
			path2 = path2.append("");
			ParseMTL(path2.c_str(), args);
		}
		else if (Text::compareWord(data, pos, size, "usemtl "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string name = Text::getText(data, pos, size);
			size_t nameL = name.length();
			for (size_t i = 0; i < args->matIds->size(); i++)
			{
				if (Text::compareWord(args->matIds->at(i).c_str(), 0, nameL, name.c_str()))
				{
					objIndex = i;
					break;
				}
			}
		}
		else if (Text::compareWord(data, pos, size, "v "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			vertices->push_back(Core::Maths::Vec3D(a, b, c));
		}
		else if (Text::compareWord(data, pos, size, "vn "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			normals->push_back(Core::Maths::Vec3D(a, b, c));
		}
		else if (Text::compareWord(data, pos, size, "vt "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			tCoord->push_back(Core::Maths::Vec2D(a, b));
		}
		else if (Text::compareWord(data, pos, size, "f "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			ReadFace(pos, objIndex, type, data, size, args);
		}
		pos = Text::endLine(data, pos, size);
	}
	return type;
}
void Core::Util::ModelLoader::MeshLoaderThread(LowRenderer::Model& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager)
{
	if (!path) return;
	std::vector<std::string> matIds;
	std::vector<std::vector<Triangle>> tris;
	std::vector<Core::Maths::Vec3D> verts;
	std::vector<Core::Maths::Vec3D> norms;
	std::vector<Core::Maths::Vec2D> UVs;
	std::vector<Resources::Material*> tmats;
	std::vector<Vertice> verticesOut;
	std::vector<TangentVertice> tverticesOut;
	std::vector<std::vector<unsigned int>> indexesOut;
	std::vector<VerticeIndex> indexes;
	int64_t size = 0;
	const char* data = loadFile(path, size);
	char DefaultType = -1;
	if (data == nullptr)
	{
		DefaultType = IsDefaultModel(path);
		if (DefaultType == -1)
		{
			LOG("Error, cannot open file %s", path);
			return;
		}
	}
	tris.push_back(std::vector<Triangle>());

	std::string newPath;
	unsigned int index = 0;
	for (unsigned int i = 0; path[i] != 0; i++) if (path[i] == '/' || path[i] == '\\') index = i + 1;
	for (unsigned int i = 0; i < index; i++) newPath += path[i];
	LoaderData args = {
		manager,
		materialManager,
		textureManager,
		&tmats,
		&matIds,
		&tris,
		&verts,
		&norms,
		&UVs,
		&verticesOut,
		&tverticesOut,
		&indexes,
		&indexesOut
	};
	signed char type;
	if (data) type = Loop(newPath.c_str(), data, size, &args);
	else type = GenerateDefault(DefaultType, &args);

	unsigned int VBO = 0;
	for (unsigned int i = 0; i < tris.size(); i++)
	{
		std::string fullPath = path;
		if (args.mats->size() > i)
		{
			fullPath.append(":");
			fullPath.append(args.matIds->at(i));
		}
		else
		{
			char number[16];
			snprintf(number, 16, "%u", i);
			std::string path2 = path;
			path2 = path2.append(":");
			path2 = path2.append(number);
			size_t tmpIndex = args.materialManager->CreateMaterial(args.manager, path2.c_str());
			args.mats->push_back(args.materialManager->GetMaterials().at(tmpIndex));
		}
		Resources::Mesh* current = manager->Create<Resources::Mesh>(fullPath.c_str());
		current->loaded = false;
		in.AddMesh(current, manager, args.mats->at(i));
		modelManager->AddMesh(current);
		indexesOut.push_back(std::vector<unsigned int>());
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
		if (size < 20000000llu)
		{
			if (CreateTangent || !data)
			{
				current->indexCount = GenerateTangentVertices(&args, i, type);
			}
			else
			{
				current->indexCount = GenerateVertices(&args, i, type);
			}
		}
		else
		{
			current->indexCount = GenerateVerticesUnoptimized(&args, i, type);
		}
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(current->VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexesOut[i].size() * sizeof(unsigned int), indexesOut[i].data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, current->VBO);
		if (CreateTangent || !data)
		{
			if (i == tris.size() - 1llu) glBufferData(GL_ARRAY_BUFFER, tverticesOut.size() * sizeof(TangentVertice), tverticesOut.data(), GL_STATIC_DRAW);
			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertice), (void*)0);
			glEnableVertexAttribArray(0);
			// normal attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertice), (void*)(sizeof(Core::Maths::Vec3D)));
			glEnableVertexAttribArray(1);
			// texture coord attribute
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TangentVertice), (void*)(2 * sizeof(Core::Maths::Vec3D)));
			glEnableVertexAttribArray(2);
			// tangent attribute
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertice), (void*)(2 * sizeof(Core::Maths::Vec3D) + sizeof(Core::Maths::Vec2D)));
			glEnableVertexAttribArray(3);
		}
		else
		{
			if (i == tris.size() - 1llu) glBufferData(GL_ARRAY_BUFFER, verticesOut.size() * sizeof(Vertice), verticesOut.data(), GL_STATIC_DRAW);
			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);
			glEnableVertexAttribArray(0);
			// normal attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(sizeof(Core::Maths::Vec3D)));
			glEnableVertexAttribArray(1);
			// texture coord attribute
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(2 * sizeof(Core::Maths::Vec3D)));
			glEnableVertexAttribArray(2);
		}
		current->loaded = true;
	}
	for (unsigned int i = 0; i < tmats.size(); i++)
	{
		in.materials.push_back(*tmats[i]);
	}
	LOG("Successfully loaded model %s", path);
	if (data) delete[] data;
	return;
}

void Core::Util::ModelLoader::ParseMTL(const char* path, Core::Util::LoaderData* args)
{
	args->manager->SetPathAutoAppend(true);
	int64_t size;
	const char* data = loadFile(path, size);
	if (!data)
	{
		return;
	}
	int64_t pos = 0;
	int64_t matIndex = -1;
	while (pos < size)
	{
		if (Text::compareWord(data, pos, size, "newmtl "))
		{
			Resources::Texture::SetFilterType(GL_LINEAR);
			Resources::Texture::SetWrapType(GL_REPEAT);
			pos = Text::skipCharSafe(data, pos, size);
			std::string path2 = path;
			std::string name = Text::getText(data, pos, size);
			path2 = path2.append(":");
			path2 = path2.append(name);
			size_t tmpIndex = args->materialManager->CreateMaterial(args->manager, path2.c_str());
			args->mats->push_back(args->materialManager->GetMaterials().at(tmpIndex));
			if (args->matIds)
			{
				args->matIds->push_back(name);
				if (matIndex != -1)
				{
					args->tris->push_back(std::vector<Triangle>());
				}
			}
			matIndex++;
		}
		else if (Text::compareWord(data, pos, size, "Ns "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			args->mats->at(matIndex)->Smoothness = Text::getFloat(data, pos, size);
		}
		else if (Text::compareWord(data, pos, size, "Ka "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			args->mats->at(matIndex)->AmbientColor = Core::Maths::Vec3D(a, b, c);
		}
		else if (Text::compareWord(data, pos, size, "Kd "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			args->mats->at(matIndex)->DiffuseColor = Core::Maths::Vec3D(a, b, c);
		}
		else if (Text::compareWord(data, pos, size, "Ks "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			args->mats->at(matIndex)->SpecularColor = Core::Maths::Vec3D(a, b, c);
		}
		else if (Text::compareWord(data, pos, size, "Ke "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			float a = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float b = Text::getFloat(data, pos, size);
			pos = Text::skipCharSafe(data, pos, size);
			float c = Text::getFloat(data, pos, size);
			args->mats->at(matIndex)->EmissiveColor = Core::Maths::Vec3D(a, b, c);
		}
		else if (Text::compareWord(data, pos, size, "d "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			args->mats->at(matIndex)->Alpha = Core::Maths::Util::cut(Text::getFloat(data, pos, size), 0.0f, 1.0f);
		}
		else if (Text::compareWord(data, pos, size, "Tr "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			args->mats->at(matIndex)->Alpha = Core::Maths::Util::cut(1.0f - Text::getFloat(data, pos, size), 0.0f, 1.0f);
		}
		else if (Text::compareWord(data, pos, size, "map_Kd_Nearest"))
		{
			Resources::Texture::SetFilterType(GL_NEAREST);
		}
		else if (Text::compareWord(data, pos, size, "map_Kd_Edge"))
		{
			Resources::Texture::SetWrapType(GL_CLAMP_TO_EDGE);
		}
		else if (Text::compareWord(data, pos, size, "map_Ka ") || Text::compareWord(data, pos, size, "map_Kd "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string texPath = Text::getText(data, pos, size);
			args->mats->at(matIndex)->SetTexture(args->manager, args->textureManager, texPath.c_str(), true);
		}
		else if (Text::compareWord(data, pos, size, "map_Kn "))
		{
			pos = Text::skipCharSafe(data, pos, size);
			std::string texPath = Text::getText(data, pos, size);
			args->mats->at(matIndex)->SetNormalMap(args->manager, args->textureManager, texPath.c_str(), true);
		}
		pos = Text::endLine(data, pos, size);
	}
	delete[] data;
}

void Core::Util::ModelLoader::LoadMesh(LowRenderer::Model& in, const char* path, Resources::ResourceManager* manager, Resources::MaterialManager* materialManager, Resources::TextureManager* textureManager, Resources::MeshManager* modelManager)
{
	CreateTangent = false;
	auto begin = std::chrono::high_resolution_clock::now();

	MeshLoaderThread(in, path, manager, materialManager, textureManager, modelManager);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	long unsigned int total = 0;
	LOG("Time measured: %.6f seconds (%.3f miliseconds)", elapsed.count() * 1e-9, elapsed.count() * 1e-6);
}

char Core::Util::ModelLoader::IsDefaultModel(const char* path)
{
	if (Text::compareWord("DebugSphere", 0, 12, path))
	{
		return 1;
	}
	else if (Text::compareWord("DebugCube", 0, 10, path))
	{
		return 2;
	}
	else if (Text::compareWord("DebugCapsuleA", 0, 14, path))
	{
		return 3;
	}
	else if (Text::compareWord("DebugCapsuleB", 0, 14, path))
	{
		return 4;
	}
	else if (Text::compareWord("DebugCapsuleC", 0, 14, path))
	{
		return 5;
	}
	else if (Text::compareWord("DebugPlane", 0, 11, path))
	{
		return 6;
	}
	return -1;
}

signed char Core::Util::ModelLoader::GenerateDefault(char Type, LoaderData* args)
{
	
	switch (Type)
	{
	case 1:
		Maths::Util::GenerateSphere(32,32,args->verts, args->norms, args->UVs);
		break;
	case 2:
		Maths::Util::GenerateCube(args->verts, args->norms, args->UVs);
		break;
	case 3:
		Maths::Util::GenerateDome(32, 32, false, args->verts, args->norms, args->UVs);
		break;
	case 4:
		Maths::Util::GenerateDome(32, 32, true, args->verts, args->norms, args->UVs);
		break;
	case 5:
		Maths::Util::GenerateCylinder(32, 16, args->verts, args->norms, args->UVs);
		break;
	case 6:
		Maths::Util::GeneratePlane(args->verts, args->norms, args->UVs);
		break;
	default:
		break;
	}
	unsigned int t[3] = { 0,0,0 };
	for (size_t i = 0; i < args->verts->size() / 3; i++)
	{
		t[0] = (unsigned int)(i * 3);
		t[1] = (unsigned int)(i * 3 + 1);
		t[2] = (unsigned int)(i * 3 + 2);
		args->tris->at(0).push_back(Triangle(t,t,t));
	}
	return 2;
}