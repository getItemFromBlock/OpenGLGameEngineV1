#version 450 core

#define COLUMNS 256
#define LINES 144

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
	int x = int(TexCoord.x * COLUMNS);
	int y = int(TexCoord.y * LINES);
	vec2 texPos = vec2(x*1.0/COLUMNS, y*1.0/LINES);
	FragColor = texture(m_Texture, texPos) * vec4(MatEmissive + MatAmbient, MatAlpha);
}