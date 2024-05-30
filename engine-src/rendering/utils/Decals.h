/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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
*/

#pragma   once
#include "config.h"

#include "BaseClasses.h"
#include "StringUtils.h"

#include <OgreRoot.h>
#include <unordered_map>

namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup Rendering
/// @{
/// @file

/**
 * @brief class for preparing and settings decal textures
 */
class Decals :  public MGE::Singleton<Decals> {
public:
	/// pointer to texture array with colours textures (emmisive & diffuse) for decals
	Ogre::TextureGpu* colorTex;
	/// pointer to texture array with normals textures for decals
	Ogre::TextureGpu* normalsTex;
	
	/// return array textue index for emmisive texture name
	Ogre::TextureGpu* getEmissive(const std::string_view& name);
	
	/// return array textue index for diffuse texture name
	Ogre::TextureGpu* getDiffuse(const std::string_view& name);
	
	/// return array textue index for normals texture name
	Ogre::TextureGpu* getNormals(const std::string_view& name);
	
	/// destructor
	~Decals();
	
	/// constructor
	Decals(const pugi::xml_node& xmlNode, Ogre::SceneManager* scnMgr);
	
protected:
	std::unordered_map<std::string, Ogre::TextureGpu*, MGE::string_hash, std::equal_to<>> emissiveTexNames;
	std::unordered_map<std::string, Ogre::TextureGpu*, MGE::string_hash, std::equal_to<>> diffuseTexNames;
	std::unordered_map<std::string, Ogre::TextureGpu*, MGE::string_hash, std::equal_to<>> normalsTexNames;
};

/// @}

}
