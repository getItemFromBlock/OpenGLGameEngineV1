#pragma once

#include "Resources/Texture.hpp"

enum class TextureFilterType : unsigned int
{
    Nearest,
    Linear,
};

enum class TextureWrapType : unsigned int
{
    Repeat,
    MirroredRepeat,
    MirroredClamp,
    ClampToEdge,
    ClampToBorder,
};

namespace LowRenderer::Rendering
{
    class FrameBuffer : public Resources::Texture
    {
    public:
        friend class RenderCamera;
        FrameBuffer();
        ~FrameBuffer();

        bool Init(unsigned int width, unsigned int height, TextureFilterType TFilter, TextureWrapType TWrap);
        virtual void Load(const char* path) override;
        virtual void UnLoad() override;

        void Update(unsigned int width, unsigned int height, TextureFilterType TFilter, TextureWrapType TWrap);

        bool BindForWriting();

        void BindForReading();

        TextureFilterType FilterType = TextureFilterType::Linear;
        TextureWrapType WrapType = TextureWrapType::ClampToEdge;

    private:
        unsigned int FBO = 0;
        unsigned int depthID = 0;
    };
}