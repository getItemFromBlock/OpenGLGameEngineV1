#pragma once

#include "Resources/IResource.hpp"
#include "Core/DataStructure/INameable.hpp"
#include "Core/Maths/Maths.hpp"
#include "Resources/Texture.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/TextureManager.hpp"

namespace Resources
{
	class Material : public IResource, public Core::DataStructure::INameable
	{
	public:
		Core::Maths::Vec3D AmbientColor = Core::Maths::Vec3D(1);	// Ka parameter in OBJ
		Core::Maths::Vec3D DiffuseColor = Core::Maths::Vec3D(1);	// Kd parameter
		Core::Maths::Vec3D SpecularColor = Core::Maths::Vec3D(1); // Ks parameter
		Core::Maths::Vec3D EmissiveColor = Core::Maths::Vec3D(0);	// Ke parameter
		float Alpha = 1.0f;				// d parameter (or 1 - Tr)
		float Smoothness = 64.0f;		// Ns parameter
		float Shininess = 1.0f;
		float Absorbtion = 1.0f;

		Material();
		~Material();
		void Load(const char* path) override;
		void UnLoad() override;
		void Update(ResourceManager* manager);
		const char* GetPath() override;
		const char* GetName() override;
		void SetTexture(ResourceManager* manager, TextureManager* textures, const char* path, bool raw = false);
		void SetTexture(Texture* tex);
		void SetNormalMap(ResourceManager* manager, TextureManager* textures, const char* path, bool raw = false);
		void SetNormalMap(Texture* tex);
		Texture* GetTexture() { return texture; }
		Texture* GetNormalMap() { return normalMap; }
		void RenderGUI(TextureManager* textureManager);
		void Deserialize(ResourceManager* resources, const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err);
		void Serialize(std::ofstream& fileOut, unsigned int rec);
		void TexturePopUp(TextureManager* textureManager, bool normal);
		void SetSearchData(Texture* searchIndex);

		Material& operator=(const Material& other);
	private:
		char deltaF = 0;
		char Name[256] = "";
		std::string fullPath = "";
		std::string tmpTexturePath = "";
		Texture* texture; // map_Kd or map_Ka parameter (if present in file)
		Texture* normalMap; // map_Kn (if present in file AND # GenerateTangent is present in file header AND tangent shader is used)
		static char searchText[64];
		static Texture* selectedTexture;
	};
}