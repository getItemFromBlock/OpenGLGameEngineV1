#version 450 core

in vec4 gl_FragCoord;

out vec4 FragColor;

uniform vec3 MatAmbient;
uniform float MatAlpha;

uniform sampler2D m_Texture;

void main()
{
	vec2 size = textureSize(m_Texture,0);
	vec2 TexCoord = vec2(gl_FragCoord.x/size.x,gl_FragCoord.y/size.y);
	FragColor = texture(m_Texture, TexCoord) * vec4(MatAmbient, MatAlpha);
}