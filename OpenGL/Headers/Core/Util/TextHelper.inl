#include "Core/Util/TextHelper.hpp"

inline bool Text::compareWord(const char *buff, int64_t index, const int64_t maxSize, const char *inputWord)
{
    for (index; index < maxSize && (buff[index] == ' ' || buff[index] == '\t'); index++) {}
    for (int64_t i = 0; inputWord[i] != 0; i++)
    {
        if ((index + i > maxSize) || (inputWord[i] != buff[index + i]))
            return false;
    }
    return true;
}

inline int Text::countSlash(const char *buff, int64_t index, const int64_t maxSize)
{
    int result = 0;
    while (index < maxSize && buff[index] != ' ' && buff[index] != '\n')
    {
        if (buff[index] == '/')
        {
            if (buff[index + 1] == '/')
                return -1;
            else
                result += 1;
        }
        index++;
    }
    return result;
}

inline int64_t Text::skipCharSafe(const char *buff, int64_t index, const int64_t maxSize)
{
    while (index < maxSize && buff[index] != ' ' && buff[index] != '\n' && buff[index] != '/')
    {
        index += 1;
    }
    if (buff[index] == ' ')
    {
        while (index < maxSize && buff[index] == ' ')
        {
            index += 1;
        }
        return index;
    }
    return index + 1;
}

inline int64_t Text::skipChar(const char *buff, int64_t index, const int64_t maxSize, const char cr)
{
    while (index < maxSize && buff[index] != cr)
        index += 1;
    return index + 1;
}

inline int64_t Text::endLine(const char *buff, int64_t index, const int64_t maxSize)
{
    return skipChar(buff, index, maxSize, '\n');
}

int64_t Text::getInt(const char* buf, int64_t index, const int64_t maxSize)
{
    int64_t value = 0;
    if (index >= maxSize) return value;
    char n = buf[index];
    bool negative = false;
    if (n == '-')
    {
        negative = true;
        index++;
        n = buf[index];
    }
    while (n >= '0' && n <= '9' && index < maxSize)
    {
        value *= 10;
        value += (int64_t)n - '0';
        index++;
        n = buf[index];
    }
    return (negative ? -value : value);
}

float Text::getFloat(const char* buf, int64_t index, const int64_t maxSize)
{
    float value = 0;
    if (index >= maxSize) return value;
    char n = buf[index];
    bool negative = false;
    if (n == '-')
    {
        negative = true;
        index++;
        n = buf[index];
    }
    while (n >= '0' && n <= '9' && index < maxSize)
    {
        value *= 10;
        value += n - '0';
        index++;
        n = buf[index];
    }
    if (index >= maxSize) return (negative ? -value : value);
    if (n != '.') return (negative ? -value : value);
    index++;
    n = buf[index];
    unsigned int decimal = 10;
    while (n >= '0' && n <= '9' && decimal < 100000000u && index < maxSize)
    {
        value = value + (float)(n - '0') / decimal;
        decimal *= 10;
        index++;
        n = buf[index];
    }
    return (negative ? -value : value);
}

std::string Text::getText(const char* buf, int64_t index, const int64_t maxSize)
{
    std::string result;
    char n = buf[index];
    while (n != '\n' && n != '\r' && index < maxSize)
    {
        result += n;
        index++;
        n = buf[index];
    }
    return result;
}

std::string Text::getTextQuoted(const char* buf, int64_t index, const int64_t maxSize)
{
    while (index < maxSize && buf[index] != '"') index++;
    index++;
    std::string result;
    char n = buf[index];
    while (n != '"' && index < maxSize)
    {
        result += n;
        index++;
        n = buf[index];
    }
    return result;
}

bool Parsing::ReadBool(const char* buff, const int64_t index, const int64_t maxSize, bool& result)
{
    if (Text::compareWord(buff, index, maxSize, "True"))
    {
        result = true;
        return true;
    }
    else if (Text::compareWord(buff, index, maxSize, "False"))
    {
        result = false;
        return true;
    }
    return false;
}

bool Parsing::ReadVec3D(const char* buff, int64_t index, const int64_t maxSize, Core::Maths::Vec3D& result)
{
    float a = Text::getFloat(buff, index, maxSize);
    index = Text::skipCharSafe(buff, index, maxSize);
    if (index >= maxSize)
    {
        return false;
    }
    float b = Text::getFloat(buff, index, maxSize);
    index = Text::skipCharSafe(buff, index, maxSize);
    if (index >= maxSize)
    {
        return false;
    }
    float c = Text::getFloat(buff, index, maxSize);
    result = Core::Maths::Vec3D(a, b, c);
    return true;
}

bool Parsing::ReadVec2D(const char* buff, int64_t index, const int64_t maxSize, Core::Maths::Vec2D& result)
{
    float a = Text::getFloat(buff, index, maxSize);
    index = Text::skipCharSafe(buff, index, maxSize);
    if (index >= maxSize)
    {
        return false;
    }
    float b = Text::getFloat(buff, index, maxSize);
    result = Core::Maths::Vec2D(a, b);
    return true;
}

void Parsing::Shift(std::ofstream& fileOut, unsigned int rec)
{
    for (unsigned int i = 0; i < rec; i++)
    {
        fileOut << '\t';
    }
}

std::string Text::DeSerialize(const std::string& in)
{
    std::string out;
    bool skip = false;
    for (size_t i = 0; i < in.size(); i++)
    {
        if (skip)
        {
            skip = false;
            continue;
        }
        char c = in.c_str()[i];
        if (c == '\\')
        {
            if ((i + 1) == in.size()) break;
            skip = true;
            char c2 = in.c_str()[i+1];
            switch (c2)
            {
            case 'n':
                out += '\n';
                break;
            case 't':
                out += '\t';
                break;
            default:
                out += c2;
                break;
            }
            continue;
        }
        out += c;
    }
    return out;
}

std::string Text::Serialize(const std::string& in)
{
    std::string out;
    for (size_t i = 0; i < in.size(); i++)
    {
        char c = in.c_str()[i];
        switch (c)
        {
        case '\n':
            out += '\\';
            out += 'n';
            break;
        case '\t':
            out += '\\';
            out += 't';
            break;
        case '\\':
            out += '\\';
            out += '\\';
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

std::string Text::UTFToASCII(const std::string& in)
{
    std::string out;
    for (size_t i = 0; i < in.size(); i++)
    {
        char c = in.c_str()[i];
        switch (c)
        {
        case '\n':
            out += '\\';
            out += 'n';
            break;
        case '\t':
            out += '\\';
            out += 't';
            break;
        default:
            out += c;
            break;
        }
    }
}