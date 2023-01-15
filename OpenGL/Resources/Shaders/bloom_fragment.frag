#version 450 core

#define LEVEL 7

out vec4 FragColor;

in vec3 ourNormal;
in vec2 TexCoord;
in vec3 position;

uniform vec3 MatAmbient;
uniform vec3 MatEmissive;
uniform float MatAlpha;
uniform float GlobalTime;
uniform float RNGState;

uniform sampler2D m_Texture;

void main()
{
    const int dMax = LEVEL*LEVEL*2;
    vec4 colorOut = texture(m_Texture, TexCoord);
    vec2 size = textureSize(m_Texture,0);
    for (int x = -LEVEL; x <= LEVEL; x++)
    {
        for (int y = -LEVEL; y <= LEVEL; y++)
        {
            vec4 pixel = max(texture(m_Texture, TexCoord + vec2(x/size.x, y/size.y))-vec4(1,1,1,1),0)/8.0;
            if (pixel.x+pixel.y+pixel.z > 0)
            {
                float det = 1.1 - (x*x+y*y) * 1.0 / dMax;
                colorOut += pixel * det;
            }
        }
    }
    FragColor = colorOut;
}