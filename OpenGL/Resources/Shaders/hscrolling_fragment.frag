#version 450 core

out vec4 FragColor;

in vec3 ourNormal;
in vec2 TexCoord;
in vec3 position;

uniform vec3 MatAmbient;
uniform vec3 MatEmissive;
uniform float MatAlpha;
uniform float GlobalTime;

uniform sampler2D m_Texture;

void main()
{
	vec2 pos = TexCoord + vec2(GlobalTime*2, 0);
    FragColor = texture(m_Texture, pos) * vec4(MatEmissive + MatAmbient, MatAlpha);
}