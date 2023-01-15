#include "Core/DataStructure/NameSearcher.hpp"

size_t Core::DataStructure::NameSearcher::FormatFilter(const char* input, char** output)
{
	size_t length = strlen(input);
	*output = new char[length+1llu];
	size_t index = 0;
	for (unsigned int i = 0; i < length; i++)
	{
		char t = ToLower(input[i]);
		if (t != 0 && t != ' ' && t != '_' && t != '-')
		{
			(*output)[index] = t;
			index++;
		}
	}
	(*output)[index] = 0;

	return index;
}

char Core::DataStructure::NameSearcher::ToLower(char in)
{
	return (in >= 'A' && in <= 'Z') ? in + 32 : in;
}