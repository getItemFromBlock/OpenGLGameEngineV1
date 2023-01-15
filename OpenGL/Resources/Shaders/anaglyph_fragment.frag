#version 450 core

in vec4 gl_FragCoord;

out vec4 FragColor;

uniform vec3 MatAmbient;
uniform vec3 MatEmissive;
uniform float MatAlpha;

uniform sampler2D m_Texture;
uniform sampler2D m_NTexture;

void main()
{
	vec2 size = textureSize(m_Texture,0);
	vec2 TexCoord = vec2(gl_FragCoord.x/size.x,gl_FragCoord.y/size.y);
	vec4 colorA = clamp(texture(m_Texture, TexCoord) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0);
	vec4 colorB = clamp(texture(m_NTexture, TexCoord) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0);
	FragColor = vec4(colorA.x, colorB.y, colorB.z, 1);
}