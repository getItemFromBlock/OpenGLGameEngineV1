#pragma once

#include "Texture.hpp"

namespace Resources
{
	class Font : public Texture
	{
	public:
		Font();
		~Font();

		virtual void Load(const char* path) override;
		virtual void UnLoad() override;

		float GetSpacing(unsigned char i) { return FontSpacings[i]; }

	protected:
		void GenerateSpacings();

		float FontSpacings[256] = { 0.0f };
	};

}