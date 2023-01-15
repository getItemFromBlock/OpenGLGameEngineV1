#version 450 core

out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 MatAmbient;
uniform vec3 MatEmissive;
uniform float MatAlpha;

uniform sampler2D m_Texture;

void main()
{
	vec4 color = clamp(texture(m_Texture, TexCoord) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0);
	float value = (color.x + color.y + color.z) / 3;
	FragColor = vec4(value, value, value, color.w);
}