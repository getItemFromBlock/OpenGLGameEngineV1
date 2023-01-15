#pragma once

#include "Resources/Shader.hpp"
#include "Core/DataStructure/INameable.hpp"

namespace Resources
{
	enum class ShaderData : unsigned char
	{
		MVP = 0,
		M,
		Texture,
		NTexture,
		LightMVP,
		MatAmbient,
		MatDiffuse,
		MatSpecular,
		MatEmissive,
		MatAlpha,
		MatSmoothness,
		MatShininess,
		MatAbsorbtion,
		LDirectional,
		LDirectionalCount,
		LPoint,
		LPointCount,
		LSpot,
		LSpotCount,
		LShadowMap,
		LShadowMapIndex,
		CameraPosition,
		GlobalTime,
		RNGState,
		DeltaPos,
		DeltaUV,
		BonesMVP,
		BonesM,
		Count,
	};

	class ShaderProgram : public Core::DataStructure::INameable
	{
	public:
		ShaderProgram();
		~ShaderProgram();

		const char* GetName() override { return Name; };

		void Create(Resources::VertexShader* vertex, Resources::FragmentShader* fragment, const char* name);
		void Reload(bool init = false);
		void Destroy();

		int GetLocation(ShaderData id);

		unsigned int GetProgramID();

	private:
		const char* Name = nullptr;
		unsigned int programID = 0;
		Resources::VertexShader* vertex = nullptr;
		Resources::FragmentShader* fragment = nullptr;
		int locations[static_cast<unsigned long long>(ShaderData::Count)];
	};
}