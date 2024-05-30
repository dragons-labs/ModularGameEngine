/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#ifndef __DOCUMENTATION_GENERATOR__

#include "config.h"

#include "input/InputSystem.h"
#include "rendering/CameraSystem.h"
#include "game/misc/PrimarySelection.h"
#include "data/structs/components/3DWorld.h"
#include "game/gui/ActorInfo.h"
#include "gui/modules/Gui3DProgressBar.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 0
#include "physics/Physics.h"
#include "physics/TimeSystem.h"
#include "rendering/audio-video/AudioSystem.h"
#endif

#include <iostream>

struct GameInput : MGE::InputSystem::Listener { /// @todo TODO.4: use config based approach
	MGE::ProgressBar3D* g3d;
	Ogre::Controller<Ogre::Real>* controller = 0;
	
	bool keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override {
		switch(arg.key) {
			case OIS::KC_NUMPAD5: {
				// when NumLock is off
				if (MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::NumLock))
					return false;
				
				// detach camera from current owner or attach camera to selected object (form primary selection)
				auto camera = MGE::CameraSystem::getPtr()->getCurrentCamera();
				if (camera->getOwner()) {
					camera->setOwner(NULL);
				} else if (MGE::PrimarySelection::getPtr()->selectedObjects.selection.size() == 1) {
					MGE::BaseActor* actor = *(MGE::PrimarySelection::getPtr()->selectedObjects.selection.begin());
					camera->setOwner(
						actor->getComponent<MGE::World3DObject>()->getOgreSceneNode()
					);
				}
				return true;
			}
			case OIS::KC_F1: { // hide / show actor info
				if (MGE::ActorInfo::getPtr())
					MGE::ActorInfo::getPtr()->toggleVisibility();
				return true;
			}
			
		#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 0
			case OIS::KC_F12: { // switch bullet physics debug
				#if defined(USE_BULLET) && defined(MGE_DEBUG_PHYSICS_DRAW)
				static bool debug_physics_draw = false;
				debug_physics_draw = ! debug_physics_draw;
				MGE::Physics::Physics::getPtr()->setDebugMode( debug_physics_draw );
				#endif
				return true;
			}
			case OIS::KC_F11: { // show info about camera, time and audio
				LOG_INFO("CAMERA INFO:\n" 
					<< "    TRG POSITION:     "   << MGE::CameraSystem::getPtr()->getCurrentCamera()->getPosition() << "\n"
					<< "    TRG ORIENTATION:  "   << MGE::CameraSystem::getPtr()->getCurrentCamera()->getOrientation() << "\n"
					<< "    CAM ORIENTATION:  "
					<< "       - zoom:          " << MGE::CameraSystem::getPtr()->getCurrentCamera()->getZoom()   << "\n"
					<< "       - angleTOground: " << MGE::CameraSystem::getPtr()->getCurrentCamera()->getPitch()  << "  /  "
												<< MGE::CameraSystem::getPtr()->getCurrentCamera()->getCamera()->getOrientation().getPitch() << "\n"
					<< "       - angleONground: " << MGE::CameraSystem::getPtr()->getCurrentCamera()->getYaw()    << "  /  "
												<< MGE::CameraSystem::getPtr()->getCurrentCamera()->getCamera()->getOrientation().getYaw() << "\n"
				);
				LOG_INFO("TIME INFO: " 
					<< MGE::TimeSystem::getPtr()->gameTimer->getCounterStr()
					<< " (" << MGE::TimeSystem::getPtr()->gameTimer->getCounter() << " ms)"
				);
				#ifdef USE_OGGSOUND
				if (MGE::AudioSystem::getPtr()->getSoundManager() && MGE::AudioSystem::getPtr()->getSoundManager()->getListener()) {
					LOG_INFO("AUDIO LISTENER INFO:\n"
						<< "    POSITION:          "   << MGE::AudioSystem::getPtr()->getSoundManager()->getListener()->getParentSceneNode()->getPosition()
						<< "    ORIENTATION:       "   << MGE::AudioSystem::getPtr()->getSoundManager()->getListener()->getParentSceneNode()->getOrientation()
					);
					if (MGE::AudioSystem::getPtr()->getSoundManager()->getSound("Sound1")) {
						LOG_INFO("SOUND 1 INFO:\n"
							<< "    POSITION:         "   << MGE::AudioSystem::getPtr()->getSoundManager()->getSound("Sound1")->getParentSceneNode()->getPosition()
						);
					}
				} else {
					LOG_INFO("NO AUDIO SYSTEM OR LISTENER");
				}
				#endif
				std::cout << "FULL CAMERA INFO:\n";
				pugi::xml_document xmlDoc;
				auto xmlNode = xmlDoc.append_child("CamraSystem");
				MGE::CameraSystem::getPtr()->storeToXML(xmlNode, false);
				std::cout << MGE::XMLUtils::nodeAsString(xmlNode, "  ", pugi::format_default) << "\n";
				return true;
			}
			case OIS::KC_F9: {
				throw std::logic_error("Intentional crash from keyboard (via F9).");
			}
		#endif
			
			default:
				return false;
		}
	}
};

void createGameControler() {
	GameInput* gameInput = new GameInput();
	MGE::InputSystem::getPtr()->registerListener(gameInput, -1, -1, -1, -1, 130, -1);
}

#endif

