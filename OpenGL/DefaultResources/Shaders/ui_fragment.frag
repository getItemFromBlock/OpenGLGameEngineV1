#version 450 core

out vec4 FragColor;

in vec3 ourNormal;
in vec2 TexCoord;
in vec3 position;

uniform vec3 MatAmbient;
uniform float MatAlpha;

uniform sampler2D m_Texture;

void main()
{
	vec2 size = textureSize(m_Texture,0);
	ivec2 coord = ivec2(TexCoord * size);
	vec4 texColor = texture(m_Texture, vec2(coord / size));
	if(MatAlpha < 0 && texColor.a < 0.1)
        discard;
	FragColor = texColor * vec4(MatAmbient, MatAlpha < 0 ? -MatAlpha : MatAlpha);
}