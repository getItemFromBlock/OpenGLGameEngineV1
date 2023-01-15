#pragma once

#include <vector>

#include <cstring>

#include "Core/DataStructure/INameable.hpp"

template <typename T>
struct SearchResult
{
	T* key;
	size_t weight;
};

namespace Core::DataStructure::NameSearcher
{
	template <typename T> std::vector<T*> FindElements(std::vector<T>& Provider, const char* filter);
	template <typename T> std::vector<T*> FindElementsPtr(std::vector<T*>& Provider, const char* filter);
	template <typename T> std::vector<T*> SortList(std::vector<SearchResult<T>>& rawVector);
	size_t FormatFilter(const char* input, char** output);
	char ToLower(char in);
}

#include "NameSearcher.inl"