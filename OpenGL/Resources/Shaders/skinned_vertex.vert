#version 450 core

#define LS 8u
#define LSM 2u
#define BONES 64u

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in ivec3 aBID;
layout(location = 5) in ivec3 aBID2;
layout(location = 6) in vec3 aBW;
layout(location = 7) in vec3 aBW2;

out vec3 ourNormal;
out vec2 TexCoord;
out vec3 position;
out vec4 SPos[LSM];

uniform mat4 BonesMVP[BONES];
uniform mat4 BonesM[BONES];
uniform uint SLightsCount;
uniform mat4 Smvp[LSM];
uniform int SIndex[LS];

void main()
{
	if (aBID.x == 0 && aBID.y == 0 && aBID.z == 0)
	{
		gl_Position = vec4(aPos, 1.0);
		ourNormal = aNormal;
		position = aPos;
	}
	else
	{
		vec4 glOut = vec4(0);
		vec3 nOut = vec3(0);
		vec3 pOut = vec3(0);
		for (int i = 0; i < 3 && aBW[i] != 0.0f; i++)
		{
			glOut += BonesMVP[aBID[i]] * vec4(aPos, 1.0) * aBW[i];
			nOut += vec3(BonesM[aBID[i]] * vec4(aNormal, 0.0)) * aBW[i];
			pOut += vec3(BonesM[aBID[i]] * vec4(aPos, 1.0)) * aBW[i];
			glOut += BonesMVP[aBID2[i]] * vec4(aPos, 1.0) * aBW2[i];
			nOut += vec3(BonesM[aBID2[i]] * vec4(aNormal, 0.0)) * aBW2[i];
			pOut += vec3(BonesM[aBID2[i]] * vec4(aPos, 1.0)) * aBW2[i];
		}
		gl_Position = glOut;
		ourNormal = nOut;
		position = pOut;
	}
	TexCoord = aTexCoord;
	for (uint i = 0u; i < SLightsCount; i++)
	{
		int n = SIndex[i];
		if (n < 0) continue;
		SPos[n] = Smvp[n] * vec4(position, 1.0);
	}
}