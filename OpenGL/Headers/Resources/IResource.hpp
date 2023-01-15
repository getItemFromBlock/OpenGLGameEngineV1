#pragma once

namespace Resources
{
	__interface IResource
	{
		void Load(const char* path);
		void UnLoad();
		const char* GetPath();
	};
}