/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

Inspired by:
	→ ogreHTML by Muhammed Ikbal Akpaca (https://bitbucket.org/saejox/ogrehtml/src) (Zlib licensed)
*/

#include "input/InteractiveTexture.h"

#include "with.h"
#include "input/InputSystem.h"

#include "physics/Raycast.h"
#include "data/structs/BaseActor.h"
#include "physics/utils/OgreMeshRaycast.h"
#include "data/QueryFlags.h"
#include "data/utils/NamedSceneNodes.h"

#include <OgreMaterialManager.h>
#include <OgreTextureGpuManager.h>
#include <OgreTextureBox.h>
#include <OgreStagingTexture.h>
#include <OgreTechnique.h>
#include <OgreBillboardSet.h>
#include <OgreBillboard.h>
#include <OgreItem.h>
#include <OgreRoot.h>
#include <OgreHlmsManager.h>
#include <Hlms/Unlit/OgreHlmsUnlit.h>
#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>

#ifdef USE_CEGUI
#include "gui/GuiSystem.h"
#include "gui/utils/CeguiString.h"
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#endif

bool MGE::InteractiveTextureManager::mousePressedOnWorld(
	const Ogre::Vector2& mouseWindowPos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, InteractiveTexture*& activeTextureObject, CEGUI::Window* fromWindow
) {
	Ogre::Ray cameraRay = MGE::CameraSystem::getPtr()->getCurrentCamera()->getCameraRay(mouseWindowPos.x, mouseWindowPos.y);
	MGE::RayCast::ResultsPtr searchResults = MGE::RayCast::searchFromRay(MGE::CameraSystem::getPtr()->getCurrentSceneManager(), cameraRay, MGE::QueryFlags::INTERACTIVE_TEXTURE, true);
	
	if (searchResults->hitObjects.empty()) {
		unset(activeTextureObject);
		return false;
	}
	
	std::pair<bool, Ogre::Vector2> res;
	MGE::InteractiveTexture* newTextureObject = NULL;
	
	std::map< std::string, MGE::InteractiveTexture* >::iterator listenersIter = listeners.end();
	for (auto& iter : searchResults->hitObjects) {
		Ogre::String searchName;
		if (iter.ogreObject) {
			LOG_DEBUG(" is ogreObject " << iter.ogreObject->getName());
			searchName = iter.ogreObject->getName();
		} else if (iter.gameObject) {
			LOG_DEBUG(" is gameObject "<< iter.gameObject->getName());
			searchName = iter.gameObject->getName();
		} else {
			LOG_DEBUG(" is other :-/");
			continue;
		}
		
		LOG_DEBUG("InteractiveTexture search for: " << searchName);
		listenersIter = listeners.find(searchName);
		if (listenersIter != listeners.end()) {
			LOG_DEBUG("InteractiveTexture test for: " << listenersIter->second->getObjectName());
			res = listenersIter->second->ogreObjectHitTest(cameraRay);
			if (res.first) {
				newTextureObject = listenersIter->second;
				break;
			}
		}
	}
	
	return _processHit( newTextureObject, res.second, buttonID, arg, activeTextureObject, fromWindow, "OgreNode" );
}

bool MGE::InteractiveTextureManager::_processHit(
	InteractiveTexture* newTextureObject,
	const Ogre::Vector2& position, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg,
	InteractiveTexture*& activeTextureObject, CEGUI::Window* window, MGE::null_end_string info
) {
	if (newTextureObject) {
		LOG_DEBUG("InteractiveTexture hit (" << position.x << ", " << position.y << ") for " << info);
		if (newTextureObject != activeTextureObject) {
			if (activeTextureObject) {
				LOG_DEBUG("lostInput for: " << activeTextureObject->getObjectName());
				if (!activeTextureObject->lostInput(newTextureObject, false)) {
					LOG_DEBUG("rejected lostInput, NOT change curentTextureObject to: " << newTextureObject->getObjectName());
					return activeTextureObject->mousePressed(position, buttonID, arg);
				}
			}
			activeTextureObject = newTextureObject;
		}
		
		activeTextureObject->clickWindow = window;
		return activeTextureObject->mousePressed(position, buttonID, arg);
	} else {
		unset(activeTextureObject);
		return false;
	}
}

bool MGE::InteractiveTextureManager::mousePressedOnGUI(
	const Ogre::Vector2& mouseWindowPos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, InteractiveTexture*& activeTextureObject, CEGUI::Window* window
) {
#ifdef USE_CEGUI
	std::map< std::string, MGE::InteractiveTexture* >::iterator listenersIter = listeners.find( window->getName().getString() );
	
	return _processHit(
		listenersIter != listeners.end() ? listenersIter->second : NULL,
		mouseWindowPos, buttonID, arg, activeTextureObject, window, "GUI"
	);
#else
	return false;
#endif
}

void MGE::InteractiveTextureManager::unset(InteractiveTexture*& activeTextureObject, bool toGUI) {
	if (activeTextureObject)
		(activeTextureObject)->lostInput(NULL, toGUI);
	activeTextureObject = NULL;
}


void MGE::InteractiveTextureManager::addTextureListener(
	const std::string_view& objectName,
	MGE::InteractiveTexture* textureObject
)  {
	LOG_DEBUG("register TextureListener: " << objectName);
	listeners.insert(std::make_pair(objectName, textureObject));
}

void MGE::InteractiveTextureManager::remTextureListener(const std::string_view& objectName) {
	#if __cplusplus > 202002L /* C++23 */
	listeners.erase(objectName);
	#else
	for (auto iter = listeners.find(objectName); iter != listeners.end(); iter = listeners.find(objectName)) { listeners.erase(iter); }
	#endif
}

MGE::InteractiveTexture* MGE::InteractiveTextureManager::getTextureListener(const std::string_view& objectName) {
	auto iter = listeners.find( objectName );
	if (iter != listeners.end()) {
		return iter->second;
	} else {
		return NULL;
	}
}

void MGE::InteractiveTexture::createMaterialOnOgreObject(bool isInteractive) {
	LOG_DEBUG("createMaterialOnOgreObject material=" << getMaterialName() << " texture=" <<  getTextureName());
	
	Ogre::HlmsMacroblock hlmsMacroblock;
	hlmsMacroblock.mCullMode = Ogre::CULL_NONE; // Ogre::CULL_CLOCKWISE;
	Ogre::HlmsBlendblock hlmsBlendblock;
	if (disableAlpha)
		hlmsBlendblock.setBlendType( Ogre::SBT_REPLACE );
	else
		hlmsBlendblock.setBlendType( Ogre::SBT_TRANSPARENT_ALPHA );
	ogreDatablock = static_cast<Ogre::HlmsUnlitDatablock*>(
		static_cast<Ogre::HlmsUnlit*>( Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT) )->createDatablock(
			getMaterialName(), Ogre::BLANKSTRING,
			hlmsMacroblock, hlmsBlendblock, Ogre::HlmsParamVec()
		)
	);
	ogreDatablock->setTexture( 0, getTextureName() );
	
	if (!ogreObject) {
		Ogre::SceneNode* node = MGE::NamedSceneNodes::getSceneNode(getObjectName());
		if (!node)
			LOG_ERROR("Can't find node for name: " + getObjectName());
		
		Ogre::String searchName = getObjectName();
		auto iter1 = node->getAttachedObjectIterator();
		while(iter1.hasMoreElements()) {
			Ogre::MovableObject* m = iter1.getNext();
			if (
				m->getName() == searchName && (
					m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME ||
					m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME ||
					m->getMovableType() == Ogre::v1::BillboardSetFactory::FACTORY_TYPE_NAME
				)
			) {
				ogreObject = m;
				break;
			}
		}
		
		if (!ogreObject)
			LOG_ERROR("Can't find ogre object for name: " + getObjectName());
	}
	
	const Ogre::String movableType = ogreObject->getMovableType();
	getBillboardInfoWhenDoTest = false;
	
	if (isInteractive) {
		ogreObject->setQueryFlags( MGE::QueryFlags::INTERACTIVE_TEXTURE );
		if (movableType == Ogre::v1::BillboardSetFactory::FACTORY_TYPE_NAME) {
			getBillboardInfoWhenDoTest = true;
			MGE::OgreMeshRaycast::getBillboardInformation( static_cast<Ogre::v1::BillboardSet*>(ogreObject), NULL, &indices, &UVs );
		} else {
			MGE::OgreMeshRaycast::getMeshInformation( ogreObject, &vertices, &indices, &UVs, isNotMovable );
		}
	}
	
	if        (movableType == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
		static_cast<Ogre::Item*>(ogreObject)->setDatablock(ogreDatablock);
	} else if (movableType == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
		static_cast<Ogre::v1::Entity*>(ogreObject)->setDatablock(ogreDatablock);
	} else if (movableType == Ogre::v1::BillboardSetFactory::FACTORY_TYPE_NAME) {
		auto billboard = static_cast<Ogre::v1::BillboardSet*>(ogreObject);
		billboard->beginBillboards(); billboard->endBillboards(); // prepare billboard renderable BEFORE set material, otherwise set material will NOT working
		billboard->setDatablock(ogreDatablock);
		
	}
}

void MGE::InteractiveTexture::createImageForCEGUIWindow(bool autoscale) {
#ifdef USE_CEGUI
	WITH_NOT_NULL(MGE::GUISystem::getPtr(), guiSystem) {
		guiTexture = static_cast<CEGUI::Texture*>( &guiSystem->getRenderer()->createTexture(getTextureName(), renderTexture, true) );
		guiImage = static_cast<CEGUI::BitmapImage*>( &CEGUI::ImageManager::getSingleton().create("BitmapImage", getImageName()) );
		guiImage->setTexture(guiTexture);
		guiImage->setImageArea( CEGUI::Rectf(
			0.0f,
			0.0f,
			renderTexture->getWidth(),
			renderTexture->getHeight()
		) );
		guiImage->setNativeResolution( CEGUI::Sizef(
			renderTexture->getWidth(),
			renderTexture->getHeight()
		) );
		if (autoscale)
			guiImage->setAutoScaled( CEGUI::AutoScaledMode::Both );
	}
#endif
}

void MGE::InteractiveTexture::fillTexture(const Ogre::uint8* data) {
	int xSize = renderTexture->getWidth();
	int ySize = renderTexture->getHeight();
	Ogre::PixelFormatGpu format = renderTexture->getPixelFormat();
	size_t bytesPerPixel = Ogre::PixelFormatGpuUtils::getBytesPerPixel( format );
	
	Ogre::TextureGpuManager* textureMgr = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();
	Ogre::StagingTexture* stagingTexture = textureMgr->getStagingTexture( xSize, ySize, 1, 1, format );
	stagingTexture->startMapRegion();
	Ogre::TextureBox texBox = stagingTexture->mapRegion( xSize, ySize, 1, 1, format );
	texBox.copyFrom( data, xSize, ySize, bytesPerPixel * xSize );
	stagingTexture->stopMapRegion();
	stagingTexture->upload( texBox, renderTexture, 0, 0, 0, true );
	textureMgr->removeStagingTexture( stagingTexture );
}

Ogre::TextureGpu* MGE::InteractiveTexture::createTexture(int xSize, int ySize, bool isInteractive, int usage, Ogre::PixelFormatGpu format) {
	Ogre::TextureGpuManager* textureMgr = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();
	renderTexture = textureMgr->createTexture(
		getTextureName(),
		Ogre::GpuPageOutStrategy::Discard,
		usage,
		Ogre::TextureTypes::Type2D,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
	);
	renderTexture->setPixelFormat(format);
	renderTexture->setNumMipmaps(1u);
	renderTexture->setResolution(xSize, ySize);
	
	if (mode == OnOgreObject) {
		createMaterialOnOgreObject(isInteractive);
	} else if (mode == OnGUIWindow) {
		createImageForCEGUIWindow();
	}
	
	if (isInteractive) {
		MGE::InteractiveTextureManager::getPtr()->addTextureListener(getObjectName(), this);
	}
	
	// move texture to GPU
	renderTexture->_transitionTo( Ogre::GpuResidency::Resident, nullptr );
	renderTexture->_setNextResidencyStatus( Ogre::GpuResidency::Resident );
	
	#ifdef MGE_DEBUG_PREFILL_TEXTURE
	size_t dataSize = Ogre::PixelFormatGpuUtils::getBytesPerPixel( format ) * xSize * ySize;
	Ogre::uint8* data = reinterpret_cast<Ogre::uint8*>(OGRE_MALLOC_SIMD( dataSize, Ogre::MEMCATEGORY_RENDERSYS ));
	
	if (format == Ogre::PFG_RGBA8_UNORM) {
		LOG_DEBUG("pre-fill texture: " << getTextureName());
		for (int yy=0; yy<ySize; ++yy) {
			for (int xx=0; xx<xSize; ++xx) {
				*(data+(yy*xSize+xx)*4)   = xx % 256;
				*(data+(yy*xSize+xx)*4+1) = xx % 256;
				*(data+(yy*xSize+xx)*4+2) = yy % 256;
				*(data+(yy*xSize+xx)*4+3) = yy % 256;
			}
		}
	} else {
		LOG_DEBUG("unsupported texture format for preffill: " << format);
	}
	
	fillTexture(data);
	
	OGRE_FREE_SIMD(data, Ogre::MEMCATEGORY_RENDERSYS);
	#endif
	
	/* renderTexture->notifyDataIsReady(); ... Ogre 2.3 automatically calls notifyDataIsReady for ManualTexture */
	return renderTexture;
}

Ogre::TextureGpu* MGE::InteractiveTexture::resizeTexture(int xSize, int ySize) {
	if (!renderTexture)
		return NULL;
	
	/*
	bool isRTT = renderTexture->isRenderToTexture();
	Ogre::PixelFormatGpu format = renderTexture->getPixelFormat();
	
	if (mode == OnGUIWindow) {
		CEGUI::ImageManager::getSingleton().destroy(*guiImage);
		MGE::GUISystem::getPtr()->getRenderer()->destroyTexture(*guiTexture);
	}
	Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager()->destroyTexture(renderTexture);
	
	Ogre::TextureGpuManager* textureMgr = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();
	renderTexture = textureMgr->createOrRetrieveTexture(
		getTextureName(),
		Ogre::GpuPageOutStrategy::Discard,
		Ogre::TextureFlags::ManualTexture | (isRTT ? Ogre::TextureFlags::RenderToTexture : 0),
		Ogre::TextureTypes::Type2D,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		0u
	);
	renderTexture->setPixelFormat(format);
	renderTexture->setNumMipmaps(1u);
	*/
	renderTexture->setResolution(xSize, ySize);
	
	if (mode == OnGUIWindow) {
		createImageForCEGUIWindow();
	}
	
	return renderTexture;
}


std::pair<bool, Ogre::Vector2> MGE::InteractiveTexture::ogreObjectHitTest(const Ogre::Ray& mouseRay) {
	if (getBillboardInfoWhenDoTest) {
		Ogre::v1::BillboardSet* billboardSet = static_cast<Ogre::v1::BillboardSet*>(ogreObject);
		MGE::OgreMeshRaycast::getBillboardInformation(
			billboardSet, &vertices, NULL, NULL, billboardSet->getBillboard(0)->getPosition()
		);
	}
	
	MGE::OgreMeshRaycast::Results res = MGE::OgreMeshRaycast::entityHitTest(
		mouseRay, ogreObject, vertices, indices, true, false, !isNotMovable
	);
	
	if (res.index < 0) {
		return std::pair<bool, Ogre::Vector2>(false, Ogre::Vector2::ZERO);
	} else {
		return std::pair<bool, Ogre::Vector2>(
			true, 
			MGE::OgreMeshRaycast::getTexturePoint(
				res, vertices, indices, UVs
			)
		);
	}
}

std::pair<bool, Ogre::Vector2> MGE::InteractiveTexture::textureHitTest(const Ogre::Vector2& mousePos) {
	if (mode == OnOgreObject) {
		return ogreObjectHitTest(MGE::CameraSystem::getPtr()->getCurrentCamera()->getCameraRay(mousePos.x, mousePos.y));
	} else {
		#ifdef USE_CEGUI
		if (MGE::GUISystem::getPtr()) {
			return std::pair<bool, Ogre::Vector2>(
				true, MGE::InputSystem::getPtr()->getInputAggregator()->calcViewportRelativePosition(mousePos, clickWindow)
			);
		}
		#endif
		return std::pair<bool, Ogre::Vector2>( false, Ogre::Vector2::ZERO );
	}
}

void MGE::InteractiveTexture::putOnGUIWindow(const std::string_view& winName) {
#ifdef USE_CEGUI
	if (!winName.empty()) {
		WITH_NOT_NULL(MGE::GUISystem::getPtr())->getMainWindow()->getChild( STRING_TO_CEGUI(winName) )->setProperty(
			"Image", getImageName()
		);
	}
#endif
}

MGE::InteractiveTexture::InteractiveTexture(
	const std::string_view& _namePrefix,
	const std::string_view& _objectName,
	Mode _mode,
	Ogre::SceneManager* _scnMgr,
	bool _isNotMovable,
	bool _disableAlpha,
	Ogre::MovableObject* _ogreObject
) :
	mode          (_mode),
	scnMgr        (_scnMgr),
	namePrefix    (_namePrefix),
	objectName    (_objectName),
	ogreObject    (_ogreObject),
	isNotMovable  (_isNotMovable),
	disableAlpha  (_disableAlpha),
	renderTexture (nullptr),
	ogreDatablock (nullptr),
	guiTexture    (nullptr),
	guiImage      (nullptr)
{}

MGE::InteractiveTexture::~InteractiveTexture() {
	if (mode == OnOgreObject) {
		if (ogreDatablock) {
			if (ogreObject->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
				static_cast<Ogre::Item*>(ogreObject)->setDatablock(
					Ogre::Root::getSingletonPtr()->getHlmsManager()->getDefaultDatablock()
				);
			} else if (ogreObject->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
				static_cast<Ogre::v1::Entity*>(ogreObject)->setDatablock(
					Ogre::Root::getSingletonPtr()->getHlmsManager()->getDefaultDatablock()
				);
			} else if (ogreObject->getMovableType() == Ogre::v1::BillboardSetFactory::FACTORY_TYPE_NAME) {
				static_cast<Ogre::v1::BillboardSet*>(ogreObject)->Ogre::Renderable::setDatablock(
					Ogre::Root::getSingletonPtr()->getHlmsManager()->getDefaultDatablock()
				);
			}
			Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT)->destroyDatablock(
				getMaterialName()
			);
		}
		Ogre::MaterialManager::getSingletonPtr()->remove(getTextureName());
#ifdef USE_CEGUI
	} else if (mode == OnGUIWindow && guiTexture) {
		CEGUI::ImageManager::getSingleton().destroy(*guiImage);
		WITH_NOT_NULL(MGE::GUISystem::getPtr())->getRenderer()->destroyTexture(*guiTexture);
#endif
	}
	if (renderTexture) {
		try {
			Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager()->destroyTexture(renderTexture);
		} catch(Ogre::Exception& e) {
			 LOG_ERROR("InteractiveTexture destructor - error in destroyTexture (call with renderTexture, mode = " << mode << "): " << e.getFullDescription());
		}
	}
	MGE::InteractiveTextureManager::getPtr()->remTextureListener(getObjectName());
}
