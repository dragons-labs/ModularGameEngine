/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "modules/gui/MiniMap.h"
#include "game/actorComponents/SelectableObject.h"
#include "ModuleBase.h"

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class for getting actors info for minimap
 */
class MiniMapSelectableObjectsInfoProvider : 
	public MGE::Module,
	public MGE::MiniMap::ObjectsInfoProvider
{
	std::set<MGE::SelectableObject*>::iterator iter;
public:
	/// @copydoc MGE::MiniMap::ObjectsInfoProvider::resetMinimapInfo
	void resetMinimapInfo() override;
	
	/// @copydoc MGE::MiniMap::ObjectsInfoProvider::getNextMinimapInfo
	bool getNextMinimapInfo(const uint16_t*& buf, int& width, int& height, Ogre::Vector3& worldPos) override;
};

/// @}

}
