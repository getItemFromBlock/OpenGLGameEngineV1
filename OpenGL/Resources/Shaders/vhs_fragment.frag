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
uniform float GlobalTime;
uniform float RNGState;

uniform sampler2D m_Texture;

void main()
{
	int x = int(TexCoord.x * COLUMNS);
	int y = int(TexCoord.y * LINES);
    float deltaPx = mod(TexCoord.x * COLUMNS, 1.0);
    float deltaPy = mod(TexCoord.y * LINES, 1.0);
	vec2 texPos = vec2((x+(0.3 * sin(GlobalTime * 30 + y / 20.f)))*1.0/COLUMNS, y*1.0/LINES);
	vec4 color = clamp(texture(m_Texture, texPos) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0) * vec4(1.1, 1.13, 1.28, 1);
    vec2 size = textureSize(m_Texture,0);
    vec4 prevX = clamp(texture(m_Texture, vec2(texPos.x-1.0/COLUMNS,texPos.y)) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0) * vec4(1.1, 1.13, 1.28, 1);
    vec4 prevY = clamp(texture(m_Texture, vec2(texPos.x,texPos.y-1.0/LINES)) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0) * vec4(1.1, 1.13, 1.28, 1);
    vec4 nextX = clamp(texture(m_Texture, vec2(texPos.x+1.0/COLUMNS,texPos.y)) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0) * vec4(1.1, 1.13, 1.28, 1);
    vec4 nextY = clamp(texture(m_Texture, vec2(texPos.x,texPos.y+1.0/LINES)) * vec4(MatEmissive + MatAmbient, MatAlpha), 0.0, 1.0) * vec4(1.1, 1.13, 1.28, 1);
    float t = mod((pow((x + y * COLUMNS) / (COLUMNS * LINES * 0.114f) + RNGState, 5)), 1) * ((y < (LINES / 2) ? (1 - pow(y* 1.0 / (LINES / 2), 1.5)) : 0));
    if (x < 2 || x > (COLUMNS-3) || y < 1 || y > (LINES-2))
    {
        color = vec4(0,0,0,1);
        t = 0;
        deltaPx = 0.5;
        deltaPy = 0.5;
    }
    else if (x < 3 || x > (COLUMNS - 4))
    {
        color = vec4(0, 0.9, 0, 1);
        t = 0;
        deltaPx = 0.5;
        deltaPy = 0.5;
    }
    if (t > 0.8)
    {
        color = (color + vec4(4.2,4.2,4.2,2)) / 3;
    }
    if (deltaPy <= 0.25)
    {
        color = vec4(color * 2 + prevY) / 3;
    }
    if (deltaPy > 0.75)
    {
        color = vec4(color * 2 + nextY) / 3;
    }
    if (deltaPx <= 0.25)
    {
        color = vec4(color * 2 + prevX) / 3;
    }
    if (deltaPx > 0.75)
    {
        color = vec4(color * 2 + nextX) / 3;
    }
    FragColor = color;
}