#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>

#define DEBUG_LOG(x, ...) printf("[%s:%d] ",__FILE__,__LINE__); printf(x, __VA_ARGS__);

#define LOG(x, ...) Core::Debug::Log::Print(x, __VA_ARGS__);
#define LOGRAW(x, ...) Core::Debug::Log::SetRaw(); Core::Debug::Log::Print(x, __VA_ARGS__);

namespace Core::Debug::Log
{
		void OpenFile(const char* path);
		void CloseFile();
		void Print(const char* format, ...);
		void SetRaw();

	void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
}