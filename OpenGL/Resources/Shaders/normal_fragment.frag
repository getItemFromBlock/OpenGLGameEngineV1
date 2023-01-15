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
	FragColor = vec4((1+ourNormal.x)/2, (1+ourNormal.y)/2, (1+ourNormal.z)/2, 1.0);
}