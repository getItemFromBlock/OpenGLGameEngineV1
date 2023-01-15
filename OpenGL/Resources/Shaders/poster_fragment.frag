#version 450 core

#define LEVEL 4

out vec4 FragColor;

in vec3 ourNormal;
in vec2 TexCoord;
in vec3 position;

uniform vec3 MatAmbient;
uniform vec3 MatEmissive;
uniform float MatAlpha;

uniform sampler2D m_Texture;

void main()
{
	vec4 color = clamp(texture(m_Texture, TexCoord) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0);
	FragColor = vec4(int(color.x*LEVEL)*1.0/LEVEL, int(color.y*LEVEL)*1.0/LEVEL, int(color.z*LEVEL)*1.0/LEVEL, color.w);
}