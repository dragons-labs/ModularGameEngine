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

#include "input/InputSystem.h"
#include "ScriptsSystem.h"

#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	class InputListener4Scripts : public MGE::InputSystem::Listener {
	public:
		bool mousePressed( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* fromWindow = NULL ) override {
			if (! _mousePressed.empty()) {
				return MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>( _mousePressed.c_str(), false,  mousePos, static_cast<int>(buttonID) );
			}
			return false;
		}
		
		bool mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override {
			if (! _mouseMoved.empty()) {
				return MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>( _mouseMoved.c_str(), false,  mousePos, static_cast<int>(arg.state.buttons) );
			}
			return false;
		}
		
		bool mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override {
			if (! _mouseReleased.empty()) {
				return MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>( _mouseReleased.c_str(), false,  mousePos, static_cast<int>(buttonID) );
			}
			return false;
		}
		
		bool keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override {
			if (! _keyPressed.empty()) {
				return MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>( _keyPressed.c_str(), false,  static_cast<int>(arg.key) );
			}
			return false;
		}
		
		bool keyReleased( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override {
			if (! _keyReleased.empty()) {
				return MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>( _keyReleased.c_str(), false,  static_cast<int>(arg.key) );
			}
			return false;
		}
		
		bool lostInput(bool full = false) override {
			if (! _lostInput.empty()) {
				return MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>( _lostInput.c_str(), full );
			}
			return false;
		}
		
		InputListener4Scripts(
			std::string_view __keyPressed = "",   std::string_view __keyReleased = "", 
			std::string_view __mousePressed = "", std::string_view __mouseMoved = "",  std::string_view __mouseReleased = "",
			std::string_view __lostInput = ""
		) : 
			_mousePressed(__mousePressed), _mouseMoved(__mouseMoved), _mouseReleased(__mouseReleased),
			_keyPressed(__keyPressed), _keyReleased(__keyReleased), _lostInput(__lostInput)
		{
			LOG_DEBUG(
				"InputListener4Scripts: keyPressed:" << _keyPressed << " keyReleased:" << _keyReleased << 
				" mousePressed:" << _mousePressed << " mouseMoved:" << _mouseMoved << " mouseReleased:" << _mouseReleased << " lostInput:" << _lostInput
			);
		}
		
		~InputListener4Scripts() {
			LOG_DEBUG("Destroy InputListener4Scripts");
			MGE::InputSystem::getPtr()->unregisterListener(this);
		}
		
	private:
		std::string  _mousePressed;
		std::string  _mouseMoved;
		std::string  _mouseReleased;
		std::string  _keyPressed;
		std::string  _keyReleased;
		std::string  _lostInput;
	};
	
	bool isModifierDown(const MGE::InputSystem& input, int mod) {
		return input.isModifierDown(static_cast<OIS::Keyboard::Modifier>(mod));
	}
	
	bool isKeyDown(const MGE::InputSystem& input, int key) {
		return input.isKeyDown(static_cast<OIS::KeyCode>(key));
	}
} }

MGE_SCRIPT_API_FOR_MODULE(Input) {
	py::class_<MGE::InputSystem, std::unique_ptr<MGE::InputSystem, py::nodelete>>(
		m, "InputSystem", DOC(MGE, InputSystem)
	)
		.def("registerListener", &MGE::InputSystem::registerListener,
			"register input events listener"
		)
		.def("unregisterListener", &MGE::InputSystem::unregisterListener,
			"unregister input events listener"
		)
		.def("isModifierDown", &isModifierDown,
			DOC(MGE, InputSystem, isModifierDown)
		)
		.def("isKeyDown", &isKeyDown,
			DOC(MGE, InputSystem, isKeyDown)
		)
		
		.def_static("get", &MGE::InputSystem::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("InputSystem"))
	;
	
	py::class_<MGE::InputSystem::Listener>(
		m, "BaseInputListener", DOC(MGE, InputSystem, Listener)
	)
	;
	
	py::class_<InputListener4Scripts, MGE::InputSystem::Listener>(
		m, "InputListener", "Equivalent of MGE::InputSystem::Listener for scripts"
	)
		.def(py::init<std::string_view, std::string_view, std::string_view, std::string_view, std::string_view, std::string_view>(),
			"constructor",
			py::arg("keyPressed"), py::arg("keyReleased"), py::arg("mousePressed"), py::arg("mouseMoved"), py::arg("mouseReleased"), py::arg("lostInput")
		)
	;
}
#endif
