#pragma once

#include "Resources/Texture.hpp"
#include "Core/Maths/Maths.hpp"

namespace LowRenderer::Lightning
{
    class ShadowMapBuffer : public Resources::Texture
    {
    public:
        ShadowMapBuffer();
        ~ShadowMapBuffer();

        bool Init(unsigned int width, unsigned int height);
        virtual void Load(const char* path) override;
        virtual void UnLoad() override;

        void BindForWriting();

        void BindForReading();

        void RefreshSize();

        static void SetShadowMapResolution(unsigned int newRes);

        Core::Maths::Mat4D VPLight;

    private:
        unsigned int FBO = 0;
        static unsigned int Resolution;
    };
}