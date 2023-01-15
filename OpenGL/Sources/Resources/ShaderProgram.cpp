#include "Resources/ShaderProgram.hpp"

#include <glad/glad.h>

#include "Core/Debug/Log.hpp"
using namespace Resources;

ShaderProgram::ShaderProgram()
{
	for (unsigned int i = 0; i < static_cast<unsigned int>(ShaderData::Count); i++)
	{
		locations[i] = -1;
	}
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::Create(VertexShader* vertex, FragmentShader* fragment, const char* name)
{
	if (!fragment || !vertex)
	{
		LOG("Error: Invalid Shader provided for ShaderProgram %s", name);
		return;
	}
	this->fragment = fragment;
	this->vertex = vertex;
	size_t length = strlen(name);
	char* tmp = new char[length+1llu];
	for (unsigned int i = 0; i < length; i++)
	{
		tmp[i] = tolower(name[i]);
	}
	tmp[length] = 0;
	Name = tmp;
	programID = glCreateProgram();
	glAttachShader(programID, vertex->GetShaderIndex());
	glAttachShader(programID, fragment->GetShaderIndex());
	Reload(true);
}

void ShaderProgram::Destroy()
{
	delete[] Name;
}

unsigned int ShaderProgram::GetProgramID()
{
	return programID;
}

void ShaderProgram::Reload(bool init)
{
	if (!init) glUseProgram(programID);
	glLinkProgram(programID);
	// check for linking errors
	int success;
	char infoLog[512];
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 512, 0, infoLog);
		LOG("Error: shader program linking failed\n%s", infoLog);
		return;
	}
	locations[0] = glGetUniformLocation(programID, "mvp");
	locations[1] = glGetUniformLocation(programID, "m");
	locations[2] = glGetUniformLocation(programID, "m_Texture");
	locations[3] = glGetUniformLocation(programID, "m_NTexture");
	locations[4] = glGetUniformLocation(programID, "Smvp");
	locations[5] = glGetUniformLocation(programID, "MatAmbient");
	locations[6] = glGetUniformLocation(programID, "MatDiffuse");
	locations[7] = glGetUniformLocation(programID, "MatSpecular");
	locations[8] = glGetUniformLocation(programID, "MatEmissive");
	locations[9] = glGetUniformLocation(programID, "MatAlpha");
	locations[10] = glGetUniformLocation(programID, "MatSmoothness");
	locations[11] = glGetUniformLocation(programID, "MatShininess");
	locations[12] = glGetUniformLocation(programID, "MatAbsorbtion");
	locations[13] = glGetUniformLocation(programID, "DLights");
	locations[14] = glGetUniformLocation(programID, "DLightsCount");
	locations[15] = glGetUniformLocation(programID, "PLights");
	locations[16] = glGetUniformLocation(programID, "PLightsCount");
	locations[17] = glGetUniformLocation(programID, "SLights");
	locations[18] = glGetUniformLocation(programID, "SLightsCount");
	locations[19] = glGetUniformLocation(programID, "SShadowMaps");
	locations[20] = glGetUniformLocation(programID, "SIndex");
	locations[21] = glGetUniformLocation(programID, "cameraPos");
	locations[22] = glGetUniformLocation(programID, "GlobalTime");
	locations[23] = glGetUniformLocation(programID, "RNGState");
	locations[24] = glGetUniformLocation(programID, "DeltaPos");
	locations[25] = glGetUniformLocation(programID, "DeltaUV");
	locations[26] = glGetUniformLocation(programID, "BonesMVP");
	locations[27] = glGetUniformLocation(programID, "BonesM");
}

int ShaderProgram::GetLocation(ShaderData id)
{
	return id >= ShaderData::Count ? -1 : locations[static_cast<unsigned long long>(id)];
}