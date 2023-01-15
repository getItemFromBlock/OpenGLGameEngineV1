#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "IResource.hpp"
#include "Core/Debug/Assert.hpp"

namespace Resources
{
	class ResourceManager
	{
	public:

		template <typename T> T* Create(const char* path);
		template <typename T> T* Get(const char* path);
		inline void Delete(const char* path);

		inline void SetPathAutoAppend(bool value);
		inline void ClearResources();
	private:
		std::unordered_map<std::string, std::unique_ptr<IResource>> dataMap;
		bool raw = false;
	};
}

#include "ResourceManager.inl"

