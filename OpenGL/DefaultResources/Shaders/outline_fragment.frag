#version 450 core

out vec4 FragColor;

in vec3 ourNormal;
in vec2 TexCoord;
in vec3 position;

uniform vec3 MatAmbient;
uniform float MatAlpha;

void main()
{
	FragColor = vec4(MatAmbient, MatAlpha);
}