/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Based on:
	→ OGRE (MIT licensed)
*/

#include "rendering/utils/Decals.h"

#include "LogSystem.h"

#include <pugixml.hpp>

#include <OgreSceneManager.h>
#include <OgreHlms.h>
#include <OgreTextureFilters.h>
#include <OgreTextureGpuManager.h>
#include <OgrePixelFormatGpuUtils.h>

/**
@page XMLSyntax_Misc

@subsection XMLNode_Decals \<Decals\>

Decals textures configuration node have next attributes:
	- @c textureWidth     texture width (all decals textures must have this same size)
	- @c textureHeight    texture height (all decals textures must have this same size)
	- @c numSlices        maximum number of decals textures
	- @c numMmipmaps      numbers of mipmaps in each decals terxture
	- @c colorTexFormat decals diffuse texture format (ag Ogre enum, eg "PF_A8R8G8B8")
	- @c normalsTexFormat  decals normals texture format (ag Ogre enum, eg "PF_R8G8_SNORM")
and subnodes @c \<Texture\> (for each deacl textuere), with next attributes:
	- @c name             name of texture resources (filename)
	- @c type             "diffuse" or "normals"
	
*/

MGE::Decals::Decals(const pugi::xml_node& xmlNode, Ogre::SceneManager* scnMgr) {
	Ogre::TextureGpuManager* textureManager = Ogre::Root::getSingleton().getRenderSystem()->getTextureGpuManager();
	
	const Ogre::uint32 decalColorId = 1;
	const Ogre::uint32 decalNormalsId = 1;
	
	Ogre::uint32 textureWidth  = xmlNode.attribute("textureWidth").as_int(256);
	Ogre::uint32 textureHeight = xmlNode.attribute("textureHeight").as_int(256);
	Ogre::uint32 numSlices     = xmlNode.attribute("numSlices").as_int(16);
	Ogre::uint32 numMmipmaps   = xmlNode.attribute("numMmipmaps").as_int(8);
	
	Ogre::PixelFormatGpu colorTexFormat = Ogre::PixelFormatGpuUtils::getFormatFromName(
		 xmlNode.attribute("colorTexFormat").as_string("PFG_RGBA8_UNORM_SRGB")
	);
	Ogre::uint32 colorTexFormatPixelSize = textureWidth * textureHeight * Ogre::PixelFormatGpuUtils::getBytesPerPixel(colorTexFormat);
	
	Ogre::PixelFormatGpu normalsTexFormat  = Ogre::PixelFormatGpuUtils::getFormatFromName(
		 xmlNode.attribute("normalsTexFormat").as_string("PFG_RG8_SNORM")
	);
	Ogre::uint32 normalsTexFormatPixelSize = textureWidth * textureHeight * Ogre::PixelFormatGpuUtils::getBytesPerPixel(normalsTexFormat);
	
	colorTex    = textureManager->reservePoolId( decalColorId, textureWidth, textureHeight, numSlices, numMmipmaps, colorTexFormat );;
	normalsTex  = textureManager->reservePoolId( decalNormalsId, textureWidth, textureHeight, numSlices, numMmipmaps, normalsTexFormat );
	
	#if 0
	/*
		Create a blank map textures, so we can use index 0 to "disable" them
		if we want them disabled for a particular Decal.
	*/
	
	Ogre::Image2 blackImage;
	Ogre::uint8* blackBuffer;
	Ogre::TextureGpu* colorTexDisabled;
	Ogre::TextureGpu* normalsTexDisabled;
	
	blackBuffer = reinterpret_cast<Ogre::uint8*>( OGRE_MALLOC( colorTexFormatPixelSize, Ogre::MEMCATEGORY_RESOURCE ) );
	memset( blackBuffer, 0, colorTexFormatPixelSize );
	blackImage.loadDynamicImage( blackBuffer, textureWidth, textureHeight, 1u, Ogre::TextureTypes::Type2D, colorTexFormat, true );
	blackImage.generateMipmaps( false, Ogre::Image2::FILTER_NEAREST );
	colorTexDisabled = textureManager->createOrRetrieveTexture(
		"decals_disabled_color", Ogre::GpuPageOutStrategy::Discard,
		Ogre::TextureFlags::AutomaticBatching | Ogre::TextureFlags::ManualTexture,
		Ogre::TextureTypes::Type2D, Ogre::BLANKSTRING, 0, decalColorId
	);
	colorTexDisabled->setResolution( blackImage.getWidth(), blackImage.getHeight() );
	colorTexDisabled->setNumMipmaps( blackImage.getNumMipmaps() );
	colorTexDisabled->setPixelFormat( blackImage.getPixelFormat() );
	colorTexDisabled->scheduleTransitionTo( Ogre::GpuResidency::Resident );
	blackImage.uploadTo( colorTexDisabled, 0, colorTexDisabled->getNumMipmaps() - 1u );
	blackImage.freeMemory();
	
	blackBuffer = reinterpret_cast<Ogre::uint8*>( OGRE_MALLOC( normalsTexFormatPixelSize, Ogre::MEMCATEGORY_RESOURCE ) );
	memset( blackBuffer, 0, normalsTexFormatPixelSize );
	blackImage.loadDynamicImage( blackBuffer, textureWidth, textureHeight, 1u, Ogre::TextureTypes::Type2D, normalsTexFormat, true );
	blackImage.generateMipmaps( false, Ogre::Image2::FILTER_NEAREST );
	normalsTexDisabled = textureManager->createOrRetrieveTexture(
		"decals_disabled_normals", Ogre::GpuPageOutStrategy::Discard,
		Ogre::TextureFlags::AutomaticBatching | Ogre::TextureFlags::ManualTexture,
		Ogre::TextureTypes::Type2D, Ogre::BLANKSTRING, 0, decalNormalsId
	);
	normalsTexDisabled->setResolution( blackImage.getWidth(), blackImage.getHeight() );
	normalsTexDisabled->setNumMipmaps( blackImage.getNumMipmaps() );
	normalsTexDisabled->setPixelFormat( blackImage.getPixelFormat() );
	normalsTexDisabled->scheduleTransitionTo( Ogre::GpuResidency::Resident );
	blackImage.uploadTo( normalsTexDisabled, 0, normalsTexDisabled->getNumMipmaps() - 1u );
	blackImage.freeMemory();
	#endif
	
	/*
		Now actually load the decals we want into the array.
		Note aliases are all lowercase! Ogre automatically aliases
		all resources as lowercase, thus we need to do that too, or else
		the texture will end up being loaded twice
	*/
	
	Ogre::TextureGpu* decalTexture;
	for (auto xmlSubNode : xmlNode.children("Texture")) {
		std::string_view type = xmlSubNode.attribute("type").as_string();
		std::string      file = xmlSubNode.attribute("file").as_string();
		std::string      name = xmlSubNode.attribute("name").as_string(file.c_str());
		
		 if (type == "emissive") {
			decalTexture = textureManager->createOrRetrieveTexture(
				file,
				Ogre::GpuPageOutStrategy::Discard, Ogre::CommonTextureTypes::Diffuse,
	            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
				decalColorId
			);
			emissiveTexNames[name] = decalTexture;
		} else if (type == "diffuse") {
			decalTexture = textureManager->createOrRetrieveTexture(
				file,
				Ogre::GpuPageOutStrategy::Discard, Ogre::CommonTextureTypes::Diffuse,
	            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
				decalColorId
			);
			diffuseTexNames[name] = decalTexture;
		} else if (type == "normals") {
			decalTexture = textureManager->createOrRetrieveTexture(
				file,
				Ogre::GpuPageOutStrategy::Discard, Ogre::CommonTextureTypes::NormalMap,
	            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
				decalNormalsId
			);
			normalsTexNames[name] = decalTexture;
		}
		decalTexture->scheduleTransitionTo( Ogre::GpuResidency::Resident );
	}
}

MGE::Decals::~Decals() {
	Ogre::TextureGpuManager* textureManager = Ogre::Root::getSingleton().getRenderSystem()->getTextureGpuManager();
	for (auto& iter : emissiveTexNames) {
		textureManager->destroyTexture( iter.second );
	}
	emissiveTexNames.clear();
	
	for (auto& iter : diffuseTexNames) {
		textureManager->destroyTexture( iter.second );
	}
	diffuseTexNames.clear();
	
	for (auto& iter : normalsTexNames) {
		textureManager->destroyTexture( iter.second );
	}
	normalsTexNames.clear();
	
	/// @todo TODO.7: we probably should destroy colorTex normalsTex and "unreserved" pools (destroy TextureArray and remove from TextureGpuManager::mTextureArrays)
}

/// @todo TODO.7: decals not project on Unlit materials ... maybe we should patch Ogre for this ...

Ogre::TextureGpu* MGE::Decals::getEmissive(const std::string_view& name) {
	auto iter = emissiveTexNames.find(name);
	if (iter != emissiveTexNames.end())
		return iter->second;
	
	LOG_INFO("Can't find decal texture: " + name);
	return nullptr;
}

Ogre::TextureGpu* MGE::Decals::getDiffuse(const std::string_view& name) {
	auto iter = diffuseTexNames.find(name);
	if (iter != diffuseTexNames.end())
		return iter->second;
	
	LOG_INFO("Can't find decal texture: " + name);
	return nullptr;
}

Ogre::TextureGpu* MGE::Decals::getNormals(const std::string_view& name) {
	auto iter = normalsTexNames.find(name);
	if (iter != normalsTexNames.end())
		return iter->second;
	
	LOG_INFO("Can't find decal texture: " + name);
	return nullptr;
}
