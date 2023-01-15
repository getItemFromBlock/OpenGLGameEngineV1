#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 ourNormal;
out vec2 TexCoord;
out vec3 position;

uniform mat4 mvp;
uniform mat4 m;

void main()
{
	gl_Position = mvp * vec4(aPos, 1.0);
	ourNormal = vec3(m * vec4(aNormal, 0.0));
	TexCoord = aTexCoord;
	position = vec3(mvp * vec4(aPos, 1.0));
}