#include <cstdio>

#include "Maths.hpp"
#include "Core/Debug/Assert.hpp"

#include <corecrt_math_defines.h>

namespace Core::Maths
{

    // -----------------------   Int2D    -----------------------

    inline Int2D::Int2D(const Vec2D& in) : x((int)in.x), y((int)in.y) {}

    inline Int2D Int2D::operator+(const Int2D& a)
    {
        Int2D res = Int2D(a.x + this->x, a.y + this->y);
        return res;
    }

    inline Int2D Int2D::operator-(const Int2D& a)
    {
        return Int2D(x - a.x, y - a.y);
    }

    inline Int2D Int2D::operator*(const Int2D& a)
    {
        Int2D res = Int2D(this->x * a.x, this->y * a.y);
        return res;
    }

    inline Int2D Int2D::operator*(const float& a)
    {
        Int2D res = Int2D(this->x * (int)a, this->y * (int)a);
        return res;
    }

    inline Int2D Int2D::operator/(const float& a)
    {
        if ((int)a == 0)
            return Int2D(_CRT_INT_MAX, _CRT_INT_MAX);
        Int2D res = Int2D(x / (int)a, y / (int)a);
        return res;
    }

    inline bool Int2D::operator==(const Int2D& b)
    {
        return (x == b.x && y == b.y);
    }

    // -----------------------   Vec2D    -----------------------

    inline float Vec2D::lengthSquared() const
    {
        return (x * x + y * y);
    }

    inline float Vec2D::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Vec2D Vec2D::operator+(const Vec2D& a) const
    {
        Vec2D res = Vec2D(a.x + this->x, a.y + this->y);
        return res;
    }

    inline Vec2D Vec2D::operator-(const Vec2D& a) const
    {
        Vec2D res = Vec2D(this->x - a.x, this->y - a.y);
        return res;
    }

    inline Vec2D Vec2D::operator-() const
    {
        return this->negate();
    }

    inline Vec2D Vec2D::operator*(const Vec2D& a) const
    {
        Vec2D res = Vec2D(this->x * a.x, this->y * a.y);
        return res;
    }

    inline Vec2D Vec2D::operator*(const float& a) const
    {
        Vec2D res = Vec2D(this->x * a, this->y * a);
        return res;
    }

    inline Vec2D Vec2D::operator/(const float& a) const
    {
        if (a == 0.0)
            return operator*(VEC_HIGH_VALUE);
        Vec2D res = operator*(1 / a);
        return res;
    }

    inline bool Vec2D::operator==(const Vec2D& b) const
    {
        return (x == b.x && y == b.y);
    }

    inline float& Vec2D::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    inline const float& Vec2D::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline bool Vec2D::isCollinearWith(Vec2D a) const
    {
        float res = a.x * y - a.y * x;
        return (res < VEC_COLLINEAR_PRECISION);
    }

    inline float Vec2D::dotProduct(Vec2D a) const
    {
        return (a.x * x + a.y * y);
    }

    inline float Vec2D::crossProduct(Vec2D a) const
    {
        return (x * a.y - y * a.x);
    }

    inline Vec2D Vec2D::unitVector() const
    {
        return operator/(getLength());
    }

    inline Vec2D Vec2D::negate() const
    {
        return operator*(-1);
    }

    inline Vec2D Vec2D::getNormal() const
    {
        return Vec2D(-y, x);
    }

    inline bool Vec2D::isIntEquivalent(Vec2D a) const
    {
        return ((int)x == (int)a.x && (int)y == a.y);
    }

    inline float Vec2D::getDistanceFromPoint(Vec2D a) const
    {
        float i = a.x - x;
        float j = a.y - y;
        return sqrtf(i * i + j * j);
    }

    inline float Core::Maths::Vec2D::GetAngle() const
    {
        if (lengthSquared() == 0.0f) return 0.0f;
        Vec2D tmp = unitVector();
        float outValue;
        if (tmp.x == 0)
        {
            if (tmp.y > 0)
                outValue = (float)M_PI / 2.0f;
            else
                outValue = -(float)M_PI / 2.0f;
        }
        else
        {
            outValue = (atanf(tmp.y / tmp.x));
            if (tmp.x < 0)
            {
                outValue += (float)M_PI;
            }
        }
        return Util::toDegrees(outValue);
    }

    // -----------------------   Int3D    -----------------------

    inline Int3D::Int3D(const Vec3D& in) : x((int)in.x), y((int)in.y), z((int)in.z) {}

    inline Int3D Int3D::operator+(const Int3D& a)
    {
        return Int3D(a.x + this->x, a.y + this->y, a.z + this->z);
    }

    inline Int3D Int3D::operator-(const Int3D& a)
    {
        return Int3D(x - a.x, y - a.y, z - a.z);
    }

    inline Int3D Int3D::operator*(const Int3D& a)
    {
        Int3D res = Int3D(this->x * a.x, this->y * a.y, this->z * a.z);
        return res;
    }

    inline Int3D Int3D::operator*(const float& a)
    {
        Int3D res = Int3D(this->x * (int)a, this->y * (int)a, this->z * (int)a);
        return res;
    }

    inline Int3D Int3D::operator/(const float& a)
    {
        if ((int)a == 0)
            return Int3D(_CRT_INT_MAX, _CRT_INT_MAX, _CRT_INT_MAX);
        Int3D res = Int3D(x / (int)a, y / (int)a, z / (int)a);
        return res;
    }

    inline bool Int3D::operator==(const Int3D& b)
    {
        return (x == b.x && y == b.y);
    }

    inline int& Int3D::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    // -----------------------   Vec3D    -----------------------

    inline float Vec3D::lengthSquared() const
    {
        return (x * x + y * y + z * z);
    }

    inline float Vec3D::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Vec3D Vec3D::operator+(const Vec3D& a) const
    {
        Vec3D res = Vec3D(a.x + this->x, a.y + this->y, a.z + this->z);
        return res;
    }

    inline Vec3D Vec3D::operator-(const Vec3D& a) const
    {
        Vec3D res = Vec3D(this->x - a.x, this->y - a.y, this->z - a.z);
        return res;
    }

    inline Vec3D Vec3D::operator-() const
    {
        return this->negate();
    }

    inline Vec3D Vec3D::operator*(const Vec3D& a) const
    {
        Vec3D res = Vec3D(this->x * a.x, this->y * a.y, this->z * a.z);
        return res;
    }

    inline Vec3D Vec3D::operator*(const float& a) const
    {
        Vec3D res = Vec3D(this->x * a, this->y * a, this->z * a);
        return res;
    }

    inline Vec3D Vec3D::operator/(const float& a) const
    {
        if (a == 0.0f)
            return operator*(VEC_HIGH_VALUE);
        Vec3D res = operator*(1 / a);
        return res;
    }

    inline bool Vec3D::operator==(const Vec3D& b) const
    {
        return (x == b.x && y == b.y && z == b.z);
    }

    inline float& Vec3D::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    inline const float& Vec3D::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline float Vec3D::dotProduct(Vec3D a) const
    {
        return (a.x * x + a.y * y + a.z * z);
    }

    inline bool Vec3D::isCollinearWith(Vec3D a) const
    {
        float res = this->dotProduct(a);
        return (res < VEC_COLLINEAR_PRECISION);
    }

    inline Vec3D Vec3D::crossProduct(Vec3D a) const
    {
        return Vec3D((y * a.z) - (z * a.y), (z * a.x) - (x * a.z), (x * a.y) - (y * a.x));
    }

    inline Vec3D Vec3D::unitVector() const
    {
        return operator/(getLength());
    }

    inline Vec3D Vec3D::negate() const
    {
        return operator*(-1);
    }

    inline bool Vec3D::isIntEquivalent(Vec3D a) const
    {
        return ((int)x == (int)a.x && (int)y == a.y && (int)z == (int)a.z);
    }

    // -----------------------   Vec4D    -----------------------

    inline Vec3D Vec4D::getVector() const
    {
        return Vec3D(x, y, z);
    }

    inline Vec4D Vec4D::homogenize() const
    {
        return Vec4D(getVector() / w);
    }

    inline Vec4D Vec4D::unitVector() const
    {
        Vec4D homogenized = homogenize();
        return homogenized / homogenized.getVector().getLength();
    }

    inline float Vec4D::lengthSquared() const
    {
        return homogenize().getVector().lengthSquared();
    }

    inline float Vec4D::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Vec4D Vec4D::operator+(const Vec4D& a) const
    {
        return Vec4D(x + a.x, y + a.y, z + a.z, w + a.w);
    }

    inline Vec4D Vec4D::operator-(const Vec4D& a) const
    {
        return Vec4D(x - a.x, y - a.y, z - a.z, w - a.w);
    }

    inline Vec4D Vec4D::operator-() const
    {
        return this->negate();
    }

    inline Vec4D Vec4D::operator*(const Vec4D& a) const
    {
        return Vec4D(x * a.x, y * a.y, z * a.z, w * a.w);
    }

    inline Vec4D Vec4D::operator*(const float& a) const
    {
        return Vec4D(x * a, y * a, z * a, w * a);
    }

    inline Vec4D Vec4D::operator/(const float& b) const
    {
        if (b == 0.0f)
            return operator*(VEC_HIGH_VALUE);
        Vec4D res = operator*(1 / b);
        return res;
    }

    inline bool Vec4D::operator==(const Vec4D& b) const
    {
        return (x == b.x && y == b.y && z == b.z && w == b.w);
    }

    inline float& Vec4D::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    inline const float& Vec4D::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline bool Vec4D::isCollinearWith(Vec4D a) const
    {
        float res = dotProduct(a);
        return (res < VEC_COLLINEAR_PRECISION);
    }

    inline float Vec4D::dotProduct(Vec4D a) const
    {
        return (a.x * x + a.y * y + a.z * z);
    }

    inline Vec4D Vec4D::crossProduct(Vec4D a) const
    {
        return Vec4D((y * a.z) - (z * a.y), (z * a.x) - (x * a.z), (x * a.y) - (y * a.x), 1.0f);
    }

    inline Vec4D Vec4D::negate() const
    {
        return operator*(-1);
    }

    inline bool Vec4D::isIntEquivalent(Vec4D a) const
    {
        return ((int)x == (int)a.x && (int)y == a.y && (int)z == (int)a.z && (int)w == (int)a.w);
    }

    // -----------------------   UChar4D    -----------------------

    inline UChar4D::UChar4D(const float* in)
    {
        r = (unsigned char)(in[0] * 255);
        g = (unsigned char)(in[1] * 255);
        b = (unsigned char)(in[2] * 255);
        a = (unsigned char)(in[3] * 255);
    }

    inline UChar4D::UChar4D(const Vec4D& in)
    {
        r = (unsigned char)(Util::cut(in[0], 0, 1) * 255);
        g = (unsigned char)(Util::cut(in[1], 0, 1) * 255);
        b = (unsigned char)(Util::cut(in[2], 0, 1) * 255);
        a = (unsigned char)(Util::cut(in[3], 0, 1) * 255);
    }

    inline UChar4D UChar4D::operator*(const float& in)
    {
        return UChar4D(r * (int)in, g * (int)in, b * (int)in, a);
    }

    inline UChar4D UChar4D::operator+(const UChar4D& in)
    {
        return UChar4D(r + in.r, g + in.g, b + in.b, a);
    }

    // -----------------------   Mat4D    -----------------------

    inline float& Mat4D::operator[](const size_t in)
    {
        Assert(in < 16);
        return content[in];
    }

    inline float& Mat4D::at(const unsigned char x, const unsigned char y)
    {
        Assert(x < 4 && y < 4);
        return content[x*4+y];
    }

    inline float& Mat3D::operator[](const size_t in)
    {
        Assert(in < 9);
        return content[in];
    }

    inline float& Mat3D::at(const unsigned char x, const unsigned char y)
    {
        Assert(x < 3 && y < 3);
        return content[x * 3 + y];
    }

    // ----------------------- Math Utils -----------------------

    inline float Util::toRadians(float in)
    {
        return in / 180.0f * (float)M_PI;
    }

    inline float Util::toDegrees(float in)
    {
        return in * 180.0f / (float)M_PI;
    }

    inline float Util::cut(float in, float min, float max)
    {
        if (in < min)
            in = min;
        if (in > max)
            in = max;
        return in;
    }

    inline int Util::cutInt(int in, int min, int max)
    {
        if (in < min)
            in = min;
        if (in > max)
            in = max;
        return in;
    }

    inline float Util::mod(float in, float value)
    {
        return in - value * floorf(in / value);
    }

    inline float Util::minF(float a, float b)
    {
        if (a > b)
            return b;
        return a;
    }

    inline float Util::maxF(float a, float b)
    {
        if (a > b)
            return a;
        return b;
    }

    inline int Util::minI(int a, int b)
    {
        if (a > b)
            return b;
        return a;
    }

    inline int Util::maxI(int a, int b)
    {
        if (a > b)
            return a;
        return b;
    }

    inline bool Util::isEqual(float a, float b, float prec)
    {
        return (a - prec < b&& a + prec > b);
    }

    inline bool Util::isEqualVec4(Vec4D a, Vec4D b, float prec)
    {
        return (isEqual(b.x, a.x, prec) && isEqual(b.y, a.y, prec) && isEqual(b.z, a.z, prec) && isEqual(b.w, a.w, prec));
    }
}