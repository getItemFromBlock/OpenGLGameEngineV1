template<typename T>
std::vector<T*> Core::DataStructure::NameSearcher::SortList(std::vector<SearchResult<T>>& rawVector)
{
	std::vector<T*> result;
	for (unsigned int i = 0; i < rawVector.size(); i++)
	{
		for (unsigned int j = i + 1; j < rawVector.size(); j++)
		{
			if (rawVector[i].weight < rawVector[j].weight)
			{
				SearchResult<T> swap = rawVector[i];
				rawVector[i] = rawVector[j];
				rawVector[j] = swap;
			}
		}
		result.push_back(rawVector[i].key);
	}
	return result;
}

template<typename T>
std::vector<T*> Core::DataStructure::NameSearcher::FindElements(std::vector<T>& Provider, const char* filter)
{
	char* key;
	size_t index = FormatFilter(filter, &key);
	std::vector<SearchResult<T>> rawVector;
	for (unsigned int i = 0; i < Provider.size(); i++)
	{
		SearchResult<T> element;
		element.key = &Provider[i];
		element.weight = 0;
		const char* nameB = ((INameable*)&Provider[i])->GetName();
		size_t kLength = strlen(nameB);
		index = 0;
		bool found = false;
		for (unsigned int n = 0; n < kLength; n++)
		{
			if (key[index] == 0)
			{
				found = true;
				index = 0;
			}
			if (key[index] == ToLower(nameB[n]))
			{
				index++;
				element.weight += (kLength - n) * (index+1);
			}
			else if (index != 0)
			{
				if (!found) element.weight = 0;
				index = 0;
			}
		}
		if (element.weight != 0)
		{
			rawVector.push_back(element);
		}
	}
	delete[] key;
	return SortList(rawVector);
}

template<typename T>
std::vector<T*> Core::DataStructure::NameSearcher::FindElementsPtr(std::vector<T*>& Provider, const char* filter)
{
	char* key;
	size_t index = FormatFilter(filter, &key);
	std::vector<SearchResult<T>> rawVector;
	for (unsigned int i = 0; i < Provider.size(); i++)
	{
		SearchResult<T> element;
		element.key = Provider[i];
		element.weight = 0;
		const char* nameB = ((INameable*)Provider[i])->GetName();
		size_t kLength = strlen(nameB);
		index = 0;
		bool found = false;
		for (unsigned int n = 0; n < kLength; n++)
		{
			if (key[index] == 0)
			{
				found = true;
				index = 0;
			}
			if (key[index] == ToLower(nameB[n]))
			{
				index++;
				element.weight += (kLength - n) * (index+1);
			}
			else if (index != 0)
			{
				if (!found) element.weight = 0;
				index = 0;
			}
		}
		if (element.weight != 0)
		{
			rawVector.push_back(element);
		}
	}
	delete[] key;
	return SortList(rawVector);
}