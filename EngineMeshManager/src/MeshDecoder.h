#pragma once

#include "Mesh.h"

#include <vector>

namespace MeshManagement
{
	class __declspec(dllexport) MeshDecoder
	{
	public:
		static std::vector<Mesh> ReadAsciiStl(const char* path);
		static Mesh ReadObj(const char* path);
	};
}