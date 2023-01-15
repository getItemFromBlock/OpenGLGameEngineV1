#include <cstdio>

#include "Core/Maths/Maths.hpp"

#include "Core/Debug/Log.hpp"

namespace Core::Maths
{
    // -----------------------   Vec3D    -----------------------

    void Vec3D::print() const
    {
        printf("(%.2f, %.2f, %.2f)\n", x, y, z);
    }

    // -----------------------   Vec4D    -----------------------

    void Vec4D::print() const
    {
        printf("(%.2f, %.2f, %.2f, %.2f)\n", x, y, z, w);
    }

    // -----------------------   Mat4D    -----------------------

    Mat4D::Mat4D(float diagonal)
    {
        for (size_t i = 0; i < 4; i++) content[i*5] = diagonal;
    }

    Mat4D::Mat4D(const Mat4D& in)
    {
        for (size_t j = 0; j < 16; j++)
        {
            content[j] = in.content[j];
        }
    }

    Mat4D::Mat4D(const float* data)
    {
        for (size_t j = 0; j < 4; j++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                content[j*4+i] = data[j+i*4];
            }
        }
    }

    Mat4D Mat4D::operator*(const Mat4D& in) const
    {
        Mat4D out;
        for (size_t j = 0; j < 4; j++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                float res = 0;
                for (size_t k = 0; k < 4; k++)
                    res += content[j+k*4] * in.content[k+i*4];

                out.content[j+i*4] = res;
            }
        }
        return out;
    }

    Vec4D Mat4D::operator*(const Vec4D& in) const
    {
        Vec4D out;
        for (size_t i = 0; i < 4; i++)
        {
            float res = 0;
            for (size_t k = 0; k < 4; k++) res += content[i+k*4] * in[k];
            out[i] = res;
        }
        return out;
    }

    Mat4D Mat4D::CreateXRotationMatrix(float angle)
    {
        Mat4D out = Mat4D(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(1, 1) = cosA;
        out.at(2, 1) = -sinA;
        out.at(1, 2) = sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat4D Mat4D::CreateYRotationMatrix(float angle)
    {
        Mat4D out = Mat4D(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(2, 0) = sinA;
        out.at(0, 2) = -sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat4D Mat4D::CreateZRotationMatrix(float angle)
    {
        Mat4D out = Mat4D(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(1, 0) = -sinA;
        out.at(0, 1) = sinA;
        out.at(1, 1) = cosA;
        return out;
    }

    Mat4D Mat4D::CreateScaleMatrix(const Vec3D& scale)
    {
        Mat4D out;
        for (int i = 0; i < 3; i++) out.at(i, i) = scale[i];
        out.content[15] = 1;
        return out;
    }

    Mat4D Mat4D::CreateTranslationMatrix(const Vec3D& translation)
    {
        Mat4D out = Mat4D(1);
        for (int i = 0; i < 3; i++) out.at(3, i) = translation[i];
        return out;
    }

    Mat4D Mat4D::CreateTransformMatrix(const Vec3D& position, const Vec3D& rotation, const Vec3D& scale)
    {
        return CreateTranslationMatrix(position) * CreateRotationMatrix(rotation) * CreateScaleMatrix(scale);
    }

    Mat4D Core::Maths::Mat4D::CreateRotationMatrix(Vec3D angles)
    {
        return CreateYRotationMatrix(angles.y) * CreateXRotationMatrix(angles.x) * CreateZRotationMatrix(angles.z);
    }

    Mat4D Mat4D::CreatePerspectiveProjectionMatrix(float near, float far, float fov)
    {
        float s = 1.0f / tanf(Util::toRadians(fov / 2.0f));
        float param1 = -far / (far - near);
        float param2 = param1 * near;
        Mat4D out;
        out.at(0, 0) = s;
        out.at(1, 1) = s;
        out.at(2, 2) = param1;
        out.at(3, 2) = -1;
        out.at(2, 3) = param2;
        return out;
    }

    Mat4D Mat4D::transposeMatrix()
    {
        float x[16] = { 0 };
        for (int j = 0; j < 4; j++)
        {
            for (int i = 0; i < 4; i++)
            {
                x[i + j * 4] = content[i+j*4];
            }
        }

        return Mat4D{ x };
    }

    void Mat4D::printMatrix(bool raw)
    {
        if (raw)
        {
            printf("[ ");
            for (int j = 0; j < 16; j++)
            {
                printf("%.2f",content[j]);
                if (j != 15) printf(", ");
            }
            printf("]\n");
        }
        else
        {
            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    printf("% 8.2f ", content[j + i * 4]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }

    Mat4D Mat4D::Identity()
    {
        return Mat4D(1);
    }

    Mat4D Mat4D::getCofactor(int p, int q, int n) const
    {
        Mat4D mat;
        int i = 0, j = 0;
        // Looping for each element of the matrix
        for (int row = 0; row < n; row++)
        {
            for (int col = 0; col < n; col++)
            {
                //  Copying into temporary matrix only those element
                //  which are not in given row and column
                if (row != p && col != q)
                {
                    mat.content[i+j*4] = content[row+col*4];
                    j++;

                    // Row is filled, so increase row index and
                    // reset col index
                    if (j == n - 1)
                    {
                        j = 0;
                        i++;
                    }
                }
            }
        }
        return mat;
    }

    float Mat4D::getDeterminant(float n) const
    {
        Mat4D a;
        float D = 0; // Initialize result

        //  Base case : if matrix contains single element
        if (n == 1)
            return content[0];

        char sign = 1;  // To store sign multiplier

         // Iterate for each element of first row
        for (int f = 0; f < n; f++)
        {
            // Getting Cofactor of matrix[0][f]
            a = getCofactor(0, f, (int)n);
            D += sign * content[f*4] * a.getDeterminant(n - 1);

            // terms are to be added with alternate sign
            sign = -sign;
        }

        return D;
    }

    Mat4D Mat4D::CreateInverseMatrix() const
    {
        // Find determinant of matrix
        Mat4D inverse;
        float det = getDeterminant(4);
        if (det == 0)
        {
            printf("Singular matrix, can't find its inverse\n");
            return Mat4D();
        }

        // Find adjoint
        Mat4D adj = CreateAdjMatrix();

        // Find Inverse using formula "inverse(A) = adj(A)/det(A)"
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                inverse.content[i+j*4] = adj.content[i+j*4] / det;

        return inverse;
    }

    Mat4D Mat4D::CreateAdjMatrix() const
    {
        // temp is used to store cofactors of matrix
        Mat4D temp;
        Mat4D adj;
        char sign = 1;

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                // Get cofactor of matrix[i][j]
                temp = getCofactor(i, j, 4);

                // sign of adj positive if sum of row
                // and column indexes is even.
                sign = ((i + j) % 2 == 0) ? 1 : -1;

                // Interchanging rows and columns to get the
                // transpose of the cofactor matrix
                adj.content[j+i*4] = (sign) * (temp.getDeterminant(3));
            }
        }
        return adj;
    }

    Mat3D::Mat3D(float diagonal)
    {
        for (size_t i = 0; i < 3; i++) content[i * 4] = diagonal;
    }

    Mat3D::Mat3D(const Mat3D& in)
    {
        for (size_t j = 0; j < 9; j++)
        {
            content[j] = in.content[j];
        }
    }

    Mat3D::Mat3D(const float* data)
    {
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t i = 0; i < 3; i++)
            {
                content[j * 3 + i] = data[j + i * 3];
            }
        }
    }

    Mat3D Mat3D::operator*(const Mat3D& in)
    {
        Mat3D out;
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t i = 0; i < 3; i++)
            {
                float res = 0;
                for (size_t k = 0; k < 3; k++)
                    res += content[j + k * 3] * in.content[k + i * 3];

                out.content[j + i * 3] = res;
            }
        }
        return out;
    }

    Vec3D Mat3D::operator*(const Vec3D& in)
    {
        Vec3D out;
        for (size_t i = 0; i < 3; i++)
        {
            float res = 0;
            for (size_t k = 0; k < 3; k++) res += content[i + k * 3] * in[k];
            out[i] = res;
        }
        return out;
    }

    Mat3D Mat3D::CreateXRotationMatrix(float angle)
    {
        Mat3D out = Mat3D(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(1, 1) = cosA;
        out.at(2, 1) = -sinA;
        out.at(1, 2) = sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat3D Mat3D::CreateYRotationMatrix(float angle)
    {
        Mat3D out = Mat3D(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(2, 0) = sinA;
        out.at(0, 2) = -sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat3D Mat3D::CreateZRotationMatrix(float angle)
    {
        Mat3D out = Mat3D(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(1, 0) = -sinA;
        out.at(0, 1) = sinA;
        out.at(1, 1) = cosA;
        return out;
    }

    Mat3D Mat3D::CreateScaleMatrix(const Vec3D& scale)
    {
        Mat3D out;
        for (int i = 0; i < 3; i++) out.at(i, i) = scale[i];
        return out;
    }

    Mat3D Mat3D::transposeMatrix()
    {
        float x[9] = { 0 };
        for (int j = 0; j < 3; j++)
        {
            for (int i = 0; i < 3; i++)
            {
                x[i + j * 3] = content[i + j * 3];
            }
        }

        return Mat3D{ x };
    }

    void Mat3D::printMatrix(bool raw)
    {
        if (raw)
        {
            printf("[ ");
            for (int j = 0; j < 9; j++)
            {
                printf("%.2f", content[j]);
                if (j != 8) printf(", ");
            }
            printf("]\n");
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                for (int i = 0; i < 3; i++)
                {
                    printf("% 8.2f ", content[j + i * 3]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }

    Mat3D Mat3D::Identity()
    {
        return Mat3D(1);
    }

    Mat3D Mat3D::getCofactor(int p, int q, int n)
    {
        Mat3D mat;
        int i = 0, j = 0;
        // Looping for each element of the matrix
        for (int row = 0; row < n; row++)
        {
            for (int col = 0; col < n; col++)
            {
                //  Copying into temporary matrix only those element
                //  which are not in given row and column
                if (row != p && col != q)
                {
                    mat.content[i + j * 3] = content[row + col * 3];
                    j++;

                    // Row is filled, so increase row index and
                    // reset col index
                    if (j == n - 1)
                    {
                        j = 0;
                        i++;
                    }
                }
            }
        }
        return mat;
    }

    float Mat3D::getDeterminant(float n)
    {
        Mat3D a;
        float D = 0; // Initialize result

        //  Base case : if matrix contains single element
        if (n == 1)
            return content[0];

        char sign = 1;  // To store sign multiplier

         // Iterate for each element of first row
        for (int f = 0; f < n; f++)
        {
            // Getting Cofactor of matrix[0][f]
            a = getCofactor(0, f, (int)n);
            D += sign * content[f * 3] * a.getDeterminant(n - 1);

            // terms are to be added with alternate sign
            sign = -sign;
        }

        return D;
    }

    Mat3D Mat3D::CreateInverseMatrix()
    {
        // Find determinant of matrix
        Mat3D inverse;
        float det = getDeterminant(3);
        if (det == 0)
        {
            printf("Singular matrix, can't find its inverse\n");
            return 1;
        }

        // Find adjoint
        Mat3D adj = CreateAdjMatrix();

        // Find Inverse using formula "inverse(A) = adj(A)/det(A)"
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                inverse.content[i + j * 3] = adj.content[i + j * 3] / det;

        return inverse;
    }

    Mat3D Mat3D::CreateAdjMatrix()
    {
        // temp is used to store cofactors of matrix
        Mat3D temp;
        Mat3D adj;
        char sign = 1;

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                // Get cofactor of matrix[i][j]
                temp = getCofactor(i, j, 3);

                // sign of adj positive if sum of row
                // and column indexes is even.
                sign = ((i + j) % 2 == 0) ? 1 : -1;

                // Interchanging rows and columns to get the
                // transpose of the cofactor matrix
                adj.content[j + i * 3] = (sign) * (temp.getDeterminant(2));
            }
        }
        return adj;
    }

    Vec3D Core::Maths::Mat4D::GetPositionFromTranslation() const
    {
        return Vec3D(content[12], content[13], content[14]);
    }

    Vec3D Core::Maths::Mat4D::GetRotationFromTranslation(const Vec3D& scale) const
    {
        /*
        * 00 | 04 | 08
        * 01 | 05 | 09
        * 02 | 06 | 10
        */
        float thetaX;
        float thetaY;
        float thetaZ;
        if (Util::minF(fabsf(scale.x), Util::minF(fabsf(scale.y), fabsf(scale.z))) < 0.0001f) return Vec3D();
        float a = content[9] / scale.z;
        if (a < 0.9999f)
        {
            if (a > -0.9999f)
            {
                thetaX = asinf(-a);
                thetaY = atan2f(content[8] / scale.z, content[10] / scale.z);
                thetaZ = atan2f(content[1] / scale.x, content[5] / scale.y);
            }
            else
            {
                thetaX = (float)M_PI_2;
                thetaY = -atan2(-content[4] / scale.y, content[0] / scale.x);
                thetaZ = 0;
            }
        }
        else
        {
            thetaX = -(float)M_PI_2;
            thetaY = atan2(-content[4] / scale.y, content[0] / scale.x);
            thetaZ = 0;
        }
        return Vec3D(Util::toDegrees(thetaX), Util::toDegrees(thetaY), Util::toDegrees(thetaZ));
    }

    Vec3D Core::Maths::Mat4D::GetScaleFromTranslation() const
    {
        Vec3D x = Vec3D(content[0], content[1], content[2]);
        Vec3D y = Vec3D(content[4], content[5], content[6]);
        Vec3D z = Vec3D(content[8], content[9], content[10]);
        return Vec3D(x.getLength(), y.getLength(), z.getLength());
    }

    void Core::Maths::Util::GenerateSphere(int x, int y, std::vector<Vec3D>* PosOut, std::vector<Vec3D>* NormOut, std::vector<Vec2D>* UVOut)
    {
        float DtY = 180.0f / y;
        float DtX = 360.0f / x;
        for (int b = 1; b < y+1; b++)
        {
            for (int a = 0; a < x; a++)
            {
                if (b != y)
                {
                    PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2D(0, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2D(1, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2D(1, 0));
                }
                if (b == 1) continue;
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2D(0, 0));
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2D(0, 1));
                PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2D(1, 0));
            }
        }
    }

    void Core::Maths::Util::GenerateCube(std::vector<Vec3D>* PosOut, std::vector<Vec3D>* NormOut, std::vector<Vec2D>* UVOut)
    {
        float sign = 1.0f;
        Vec3D V[4];
        Vec3D N;
        Vec2D UV[4];
        for (char i = 0; i < 6; i++)
        {
            if (i == 3) sign = -sign;
            float A = 1.0f;
            float B = 1.0f;
            for (char j = 0; j < 4; j++)
            {
                V[j][i%3] = sign;
                V[j][(i + 1 + (i < 3)) % 3] = A;
                V[j][(i + 2 - (i < 3)) % 3] = B;
                N[i % 3] = sign;
                N[(i + 1) % 3] = 0;
                N[(i + 2) % 3] = 0;
                UV[j][i % 2] = A;
                UV[j][(i + 1) % 2] = B * sign;
                A = -A;
                std::swap(A,B);
            }
            for (char j = 0; j < 2; j++)
            {
                PosOut->push_back(V[0]);
                PosOut->push_back(V[1+j]);
                PosOut->push_back(V[2+j]);
                for (char k = 0; k < 3; k++) NormOut->push_back(N);
                UVOut->push_back(UV[0]);
                UVOut->push_back(UV[1+j]);
                UVOut->push_back(UV[2+j]);
            }
        }
    }

    void Util::GenerateDome(int x, int y, bool reversed, std::vector<Vec3D>* PosOut, std::vector<Vec3D>* NormOut, std::vector<Vec2D>* UVOut)
    {
        float DtY = 180.0f / y;
        float DtX = 360.0f / x;
        for (int b = (reversed ? 1 : y/2 + 1); b < (reversed ? y / 2 : y) + 1; b++)
        {
            for (int a = 0; a < x; a++)
            {
                if (b != y)
                {
                    PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2D(0, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2D(1, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2D(1, 0));
                }
                if (b == 1) continue;
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2D(0, 0));
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2D(0, 1));
                PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2D(1, 0));
            }
        }
    }

    void Util::GenerateCylinder(int x, int y, std::vector<Vec3D>* PosOut, std::vector<Vec3D>* NormOut, std::vector<Vec2D>* UVOut)
    {
        float DtY = 2.0f / y;
        float DtX = 360.0f / x;
        for (int b = 0; b < y; b++)
        {
            for (int a = 0; a < x; a++)
            {
                NormOut->push_back(GetSphericalCoord(DtX * a, 0));
                PosOut->push_back(NormOut->back() + Vec3D(0, DtY * (b + 1) - 1, 0));
                UVOut->push_back(Vec2D(0, 1));
                NormOut->push_back(GetSphericalCoord(DtX * (a + 1), 0));
                PosOut->push_back(NormOut->back() +Vec3D(0, DtY * (b + 1) - 1, 0));
                UVOut->push_back(Vec2D(1, 1));
                NormOut->push_back(GetSphericalCoord(DtX * (a + 1), 0));
                PosOut->push_back(NormOut->back() + Vec3D(0, DtY * b - 1, 0));
                UVOut->push_back(Vec2D(1, 0));
                NormOut->push_back(GetSphericalCoord(DtX * a, 0));
                PosOut->push_back(NormOut->back() +Vec3D(0, DtY * b - 1, 0));
                UVOut->push_back(Vec2D(0, 0));
                NormOut->push_back(GetSphericalCoord(DtX * a, 0));
                PosOut->push_back(NormOut->back() + Vec3D(0, DtY * (b + 1) - 1, 0));
                UVOut->push_back(Vec2D(0, 1));
                NormOut->push_back(GetSphericalCoord(DtX * (a + 1), 0));
                PosOut->push_back(NormOut->back() + Vec3D(0, DtY * b - 1, 0));
                UVOut->push_back(Vec2D(1, 0));
            }
        }
    }

    void Util::GeneratePlane(std::vector<Vec3D>* PosOut, std::vector<Vec3D>* NormOut, std::vector<Vec2D>* UVOut)
    {
        PosOut->push_back(Vec3D(-1, 1, 0));
        NormOut->push_back(Vec3D(0, 0, 1));
        UVOut->push_back(Vec2D(0, 0));
        PosOut->push_back(Vec3D(-1, -1, 0));
        NormOut->push_back(Vec3D(0, 0, 1));
        UVOut->push_back(Vec2D(0, 1));
        PosOut->push_back(Vec3D(1, 1, 0));
        NormOut->push_back(Vec3D(0, 0, 1));
        UVOut->push_back(Vec2D(1, 0));
        PosOut->push_back(Vec3D(1, 1, 0));
        NormOut->push_back(Vec3D(0, 0, 1));
        UVOut->push_back(Vec2D(1, 0));
        PosOut->push_back(Vec3D(-1, -1, 0));
        NormOut->push_back(Vec3D(0, 0, 1));
        UVOut->push_back(Vec2D(0, 1));
        PosOut->push_back(Vec3D(1, -1, 0));
        NormOut->push_back(Vec3D(0, 0, 1));
        UVOut->push_back(Vec2D(1, 1));
    }

    Vec3D Core::Maths::Util::GetSphericalCoord(float longitude, float latitude)
    {
        longitude = toRadians(longitude);
        latitude = toRadians(latitude);
        return Vec3D(cosf(longitude)*cosf(latitude),sinf(latitude), sinf(longitude) * cosf(latitude));
    }
}