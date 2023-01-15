#pragma once

#include <stdint.h>
#include <string>
#include <fstream>
#include "Core/Maths/Maths.hpp"

namespace Text
{
    // Function for parsing .obj models.
    inline bool compareWord(const char *buff, int64_t index, const int64_t maxSize, const char *inputWord);
    inline int countSlash(const char *buff, int64_t index, const int64_t maxSize);
    inline int64_t skipCharSafe(const char *buff, int64_t index, const int64_t maxSize);
    inline int64_t skipChar(const char *buff, int64_t index, const int64_t maxSize, const char cr);
    inline int64_t endLine(const char *buff, int64_t index, const int64_t maxSize);
    inline int64_t getInt(const char* buf, int64_t index, const int64_t maxSize);
    inline float getFloat(const char* buf, int64_t index, const int64_t maxSize);
    inline std::string getText(const char* buf, int64_t index, const int64_t maxSize);
    inline std::string getTextQuoted(const char* buf, int64_t index, const int64_t maxSize);
    inline std::string DeSerialize(const std::string& in);
    inline std::string Serialize(const std::string& in);
    inline std::string UTFToASCII(const std::string& in);
}

namespace Parsing
{
    inline bool ReadBool(const char* buff, const int64_t index, const int64_t maxSize, bool& result);
    inline bool ReadVec3D(const char* buff, int64_t index, const int64_t maxSize, Core::Maths::Vec3D& result);
    inline bool ReadVec2D(const char* buff, int64_t index, const int64_t maxSize, Core::Maths::Vec2D& result);
    inline void Shift(std::ofstream& fileOut, unsigned int rec);
}

#include "Core/Util/TextHelper.inl"