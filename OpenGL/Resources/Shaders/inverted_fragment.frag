#version 450 core

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
	FragColor = vec4(1 - color.x, 1 - color.y, 1 - color.z, color.w);
}