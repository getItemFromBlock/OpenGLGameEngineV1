#version 450 core

#define DD 13u
#define DP 15u
#define DS 20u
#define LS 8u
#define LSM 2u
#define SHADOWMAP_RESOLUTION 2048
#define DELTA 0.000001

out vec4 FragColor;

in vec3 ourNormal;
in vec2 TexCoord;
in vec3 position;
in vec4 SPos[LSM];

uniform float DLights[DD*LS];
uniform float PLights[DP*LS];
uniform float SLights[DS*LS];

uniform int SIndex[LS];
uniform sampler2DShadow SShadowMaps[LSM];

uniform uint DLightsCount;
uniform uint PLightsCount;
uniform uint SLightsCount;

uniform vec3 MatAmbient;
uniform vec3 MatDiffuse;
uniform vec3 MatSpecular;
uniform vec3 MatEmissive;
uniform float MatAlpha;
uniform float MatSmoothness;
uniform float MatShininess;
uniform float MatAbsorbtion;

uniform sampler2D m_Texture;
uniform vec3 cameraPos;

float CalcShadowFactor(int index)
{
	if (index < 0 || index >= LSM) return 1.0;
	vec4 LightSpacePos = SPos[index];
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
	float xOffset = 1.0/SHADOWMAP_RESOLUTION;
    float yOffset = 1.0/SHADOWMAP_RESOLUTION;

    float Factor = 0.0;

    for (int y = -1 ; y <= 1 ; y++) {
        for (int x = -1 ; x <= 1 ; x++) {
            vec2 Offsets = vec2(x * xOffset, y * yOffset);
            vec3 UVC = vec3(UVCoords + Offsets, z + DELTA);
            Factor += texture(SShadowMaps[index], UVC);
        }
    }
    return (Factor / 9.0);
}

vec3 GetPointLights(vec3 textureColor)
{
	vec3 amb;
	vec3 dif;
	vec3 spe;
	for (uint i = 0u; i < PLightsCount; i++)
	{
		vec3 lposition = vec3(PLights[i*DP], PLights[i*DP+1u], PLights[i*DP+2u]);
		vec3 lambient = vec3(PLights[i*DP+3u], PLights[i*DP+4u], PLights[i*DP+5u]);
		vec3 ldiffuse = vec3(PLights[i*DP+6u], PLights[i*DP+7u], PLights[i*DP+8u]);
		vec3 lspecular = vec3(PLights[i*DP+9u], PLights[i*DP+10u], PLights[i*DP+11u]);
		float lsmoothness = PLights[i*DP+12u];
		vec2 lattenuation = vec2(PLights[i*DP+13u], PLights[i*DP+14u]);
		vec3 AB = lposition-position;
		float att = length(AB);
		att = clamp(1.0/(1.0+lattenuation.x*att+lattenuation.y*att*att),0.0,1.0);
		if (att < 0.01) continue;
		AB = normalize(AB);
		vec3 normal = normalize(ourNormal);
		float deltaA = dot(AB, normal);
		float mult = 1.0;
		if (deltaA < 0.7)
        {
            mult = 0.79;
            if (deltaA < 0.45)
            {
                mult = 0.46;
                if (deltaA < 0.2)
                {
                    mult = 0.09;
                }
            }
        }
		vec3 halfV = normalize(AB + normalize(cameraPos-position));
		float deltaB = pow(max(dot(normal, halfV), 0.0), lsmoothness * MatSmoothness);
		att = att * mult;
		amb = amb + vec3(lambient * MatAmbient) * att;
		dif = dif + vec3(ldiffuse*MatDiffuse*deltaA) * att;
		spe = spe + vec3(MatSpecular*lspecular*deltaB) * att;
	}
	spe = spe * MatShininess;
	return textureColor * (amb+dif+spe) + (1-MatAbsorbtion) * spe;
}

vec3 GetSpotLights(vec3 textureColor)
{
	vec3 amb;
	vec3 dif;
	vec3 spe;
	for (uint i = 0u; i < SLightsCount; i++)
	{
		vec3 lposition = vec3(SLights[i*DS], SLights[i*DS+1u], SLights[i*DS+2u]);
		vec3 lambient = vec3(SLights[i*DS+3u], SLights[i*DS+4u], SLights[i*DS+5u]);
		vec3 ldiffuse = vec3(SLights[i*DS+6u], SLights[i*DS+7u], SLights[i*DS+8u]);
		vec3 lspecular = vec3(SLights[i*DS+9u], SLights[i*DS+10u], SLights[i*DS+11u]);
		float lsmoothness = SLights[i*DS+12u];
		vec2 lattenuation = vec2(SLights[i*DS+13u], SLights[i*DS+14u]);
		vec3 ldirection = vec3(SLights[i*DS+15u], SLights[i*DS+16u], SLights[i*DS+17u]);
		float lspotangle = SLights[i*DS+18u];
		float lspotratio = SLights[i*DS+19u];
		vec3 AB = lposition-position;
		float att = length(AB);
		att = clamp(1.0/(1.0+lattenuation.x*att+lattenuation.y*att*att),0.0,1.0);
		if (att < 0.01) continue;
		AB = normalize(AB);
		float ang = acos(dot(AB,-ldirection));
		if (ang > lspotangle) continue;
		if (1-ang/lspotangle < lspotratio)
		{
			att = att * (1-ang/lspotangle)/lspotratio;
		}
		vec3 normal = normalize(ourNormal);
		float deltaA = dot(AB, normal);
		float mult = 1.0;
		if (deltaA < 0.7)
        {
            mult = 0.79;
            if (deltaA < 0.45)
            {
                mult = 0.46;
                if (deltaA < 0.2)
                {
                    mult = 0.09;
                }
            }
        }
		vec3 halfV = normalize(AB + normalize(cameraPos-position));
		float deltaB = pow(max(dot(normal, halfV), 0.0), lsmoothness * MatSmoothness);
		float deltaS = CalcShadowFactor(SIndex[i]);
		att = att * mult;
		amb = amb + vec3(lambient * MatAmbient) * att;
		dif = dif + vec3(ldiffuse*MatDiffuse*deltaA*deltaS) * att;
		spe = spe + vec3(MatSpecular*lspecular*deltaB*deltaS) * att;
	}
	spe = spe * MatShininess;
	return textureColor * (amb+dif+spe) + (1-MatAbsorbtion) * spe;
}

vec3 GetDirectionalLights(vec3 textureColor)
{
	vec3 amb;
	vec3 dif;
	vec3 spe;
	for (uint i = 0u; i < DLightsCount; i++)
	{
		vec3 lambient = vec3(DLights[i*DD], DLights[i*DD+1u], DLights[i*DD+2u]);
		vec3 ldiffuse = vec3(DLights[i*DD+3u], DLights[i*DD+4u], DLights[i*DD+5u]);
		vec3 lspecular = vec3(DLights[i*DD+6u], DLights[i*DD+7u], DLights[i*DD+8u]);
		float lsmoothness = DLights[i*DD+9u];
		vec3 ldirection = normalize(vec3(DLights[i*DD+10u], DLights[i*DD+11u], DLights[i*DD+12u]));
		vec3 normal = normalize(ourNormal);
		float deltaA = dot(-ldirection, normal);
		float mult = 1.0;
		if (deltaA < 0.7)
        {
            mult = 0.79;
            if (deltaA < 0.45)
            {
                mult = 0.46;
                if (deltaA < 0.2)
                {
                    mult = 0.09;
                }
            }
        }
		vec3 halfV = normalize(-ldirection + normalize(cameraPos-position));
		float deltaB = pow(max(dot(normal, halfV), 0.0), lsmoothness * MatSmoothness);
		amb = amb + vec3(lambient * MatAmbient) * mult;
		dif = dif + vec3(ldiffuse*MatDiffuse*deltaA) * mult;
		spe = spe + vec3(MatSpecular*lspecular*deltaB) * mult;
	}
	spe = spe * MatShininess;
	return textureColor * (amb+dif+spe) + (1-MatAbsorbtion) * spe;
}


void main()
{
	vec4 texColor = texture(m_Texture, TexCoord);
	FragColor = vec4(GetDirectionalLights(texColor.xyz) + GetPointLights(texColor.xyz) + GetSpotLights(texColor.xyz) + texColor.xyz*MatEmissive, texColor.w*MatAlpha);
}