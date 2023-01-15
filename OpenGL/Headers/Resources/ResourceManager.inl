#pragma once

#include <string>

#include "Core/Debug/Log.hpp"

template<typename T>
T* Resources::ResourceManager::Create(const char* path)
{
	T* out = new T();
	IResource* res = dynamic_cast<IResource*>(out);
	Assert(res != nullptr);
	delete out;
	std::string fullPath;
	if (!raw)
	{
		fullPath = "Resources/";
		fullPath.append(path);
	}
	else
	{
		fullPath = path;
	}
	while (true)
	{
		size_t start_pos = fullPath.find("\\\\");
		if (start_pos == std::string::npos)
			break;
		fullPath.replace(start_pos, 2, "/");
	}
	auto result = dataMap.find(fullPath.c_str());
	if (result != dataMap.end() && result->second)
	{
		result->second.get()->UnLoad();
		result->second.reset();
	}
	std::unique_ptr<IResource> ptr = std::make_unique<T>();
	ptr->Load(fullPath.c_str());
	dataMap.emplace(fullPath,std::move(ptr));
	return dynamic_cast<T*>(dataMap.find(fullPath.c_str())->second.get());
}

template<typename T>
T* Resources::ResourceManager::Get(const char* path)
{
	std::string fullPath = path;
	while (true)
	{
		size_t start_pos = fullPath.find("\\\\");
		if (start_pos == std::string::npos)
			break;
		fullPath.replace(start_pos, 2, "/");
	}
	auto result = dataMap.find(fullPath.c_str());
	if (result == dataMap.end() || !result->second) return nullptr;
	T* res = dynamic_cast<T*>(result->second.get());
	Assert(res != nullptr);
	return res;
}


inline void Resources::ResourceManager::Delete(const char* path)
{
	std::string fullPath = path;
	while (true)
	{
		size_t start_pos = fullPath.find("\\\\");
		if (start_pos == std::string::npos)
			break;
		fullPath.replace(start_pos, 2, "/");
	}
	auto result = dataMap.find(fullPath.c_str());
	if (result == dataMap.end() || !result->second) return;
	result->second.get()->UnLoad();
	result->second.reset();
	dataMap.erase(fullPath.c_str());
}

inline void Resources::ResourceManager::SetPathAutoAppend(bool value)
{
	raw = value;
}

inline void Resources::ResourceManager::ClearResources()
{
	std::unordered_map<std::string, std::unique_ptr<IResource>>::iterator it = dataMap.begin();
	while (it != dataMap.end())
	{
		if (!it->second || !it->second.get()) return;
		it->second.get()->UnLoad();
		it->second.reset();
		it++;
	}
	dataMap.clear();
}
