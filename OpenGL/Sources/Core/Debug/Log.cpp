#include "Core/Debug/Log.hpp"

#include <iostream>
#include <stdarg.h>
#include <string>

namespace Core::Debug
{
	std::ofstream logFile;

	bool rawmode = false;

	void Log::OpenFile(const char* path)
	{
		std::string name = path;
		name.append("@");
		time_t timeLocal;
		struct tm dateTime;
		char text[32];
		time(&timeLocal);
		localtime_s(&dateTime, &timeLocal);
		strftime(text, 32, "%Y_%m_%d", &dateTime);
		name.append(text);
		name.append(".log");
		if (logFile.is_open()) CloseFile();
		logFile.open(name.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		if (!logFile.is_open())
		{
			std::cerr << "Error: Could not open output file " << path << "\nError Code :" << errno << std::endl;
		}
	}

	void Log::SetRaw()
	{
		rawmode = true;
	}

	void Log::CloseFile()
	{
		if (!logFile.is_open()) return;
		logFile.close();
	}

	void Log::Print(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		if (!rawmode) putc('\n', stdout);
		va_end(args);
		
		if (logFile.is_open())
		{
			char buffer[512];
			va_start(args, format);
			vsnprintf(buffer, 512, format, args);
			int index = 0;
			while (index < 512 && buffer[index])
			{
				logFile.put(buffer[index]);
				index++;
			}
			if (!rawmode) logFile.put('\n');
			logFile.flush();
			va_end(args);
		}
		rawmode = false;
	}

	void Log::glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
	}
}