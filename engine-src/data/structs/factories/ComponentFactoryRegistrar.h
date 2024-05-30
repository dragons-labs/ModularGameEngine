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

#pragma   once

#include "pragma.h"
#include "ListenerSet.h"
#include "BaseClasses.h"

namespace MGE {
/// @addtogroup WorldStruct
/// @{
/// @file

	/**
	 * @brief Support auto registration actor components in ComponentFactory.
	 *        See @ref MGE_REGISTER_ACTOR_COMPONENT macro.
	 * 
	 * @remark
	 *    (Trivial) Singleton, for support auto registration in @ref initFunctions.
	 */
	struct ComponentFactoryRegistrar : MGE::TrivialSingleton<ComponentFactoryRegistrar> {
		typedef bool (*ComponentInitFunction)(MGE::ComponentFactory*);
		
		/// call all registered module init functions
		inline void initAll(MGE::ComponentFactory* f) {
			initFunctions.callAll(f);
		}
		
		/// register module init function
		/// use @ref MGE_REGISTER_ACTOR_COMPONENT macro instead of direct call this function.
		inline bool registerModule(ComponentInitFunction function) {
			return initFunctions.addListener(function, 0);
		}
		
	protected:
		/// set of function to call on module import ... each of them create part of module API with pybind11
		MGE::FunctionListenerSet<ComponentInitFunction> initFunctions;
		
		friend class TrivialSingleton<ComponentFactoryRegistrar>;
		ComponentFactoryRegistrar() = default;
		~ComponentFactoryRegistrar() = default;
	};

/// @}

}

/// @addtogroup WorldStruct
/// @{

/**
 * @brief Register component "setup" <i>FUNCTION</i> to call in @ref MGE::ComponentFactory constructor.
 *        For auto registration create and use bool variable @c isRegistred__<i>COMPONENT</i> in MGE::ComponentFactoryRegistration namespace.
 * 
 *        See also @ref MGE_ACTOR_COMPONENT_CREATOR.
 */
#define MGE_REGISTER_ACTOR_COMPONENT(COMPONENT, FUNCTION) \
	MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors") \
	namespace MGE { namespace ComponentFactoryRegistration { bool isRegistred__ ## COMPONENT = MGE::ComponentFactoryRegistrar::getPtr()->registerModule(FUNCTION); } } \
	MGE_CLANG_WARNING_POP

/**
 * @brief Register function code to execute as component creator.
 * 
 * @param COMPONENT       Class to register as component.
 * @param COMPONENT_NAME  Name of registered component (pass to @ref MGE::ComponentFactory::registerComponent and used to construct function and variables names).
 * 
 * @remark Create two functions:
 *           @li @c setup__<i>COMPONENT_NAME</i> (@ref MGE::ComponentFactoryRegistrar::ComponentInitFunction)
 *           @li @c create__<i>COMPONENT_NAME</i> (@ref MGE::ComponentFactory::ComponentCreator)
 *        
 *        and register the first one (@c setup__) in @ref MGE::ComponentFactoryRegistrar::initFunctions.
 *
 *        @ref MGE::ComponentFactory constructor will call all registered @c setup__* functions.
 *        At the time (created by this macro) function @c setup__<i>COMPONENT_NAME</i> make registration of @c create__<i>COMPONENT_NAME</i> function
 *        in @ref MGE::ComponentFactoryRegistrar::registeredComponents (by call @ref MGE::ComponentFactory::registerComponent).
 * 
 *        After macro should be present block of code constituting the body of @c create__<i>COMPONENT_NAME</i> function.
 * 
 *        For registration @c setup__<i>COMPONENT_NAME</i> is calling macro @ref MGE_REGISTER_ACTOR_COMPONENT.
 * 
 * @note  It is done this way (instead of direct static registration @c create__<i>COMPONENT_NAME</i> in @ref MGE::ComponentFactoryRegistrar::registeredComponents)
 *        to avoid call @ref MGE::ComponentFactory::registerComponent before start engine.
 * 
 * \par Example
	\code{.cpp}
		struct MyNamespace::MyComponent {
			inline static const int classID = 0x1234;
			// ...
		}
		
		MGE_ACTOR_COMPONENT_CREATOR(MyNamespace::MyComponent, MyComponentName) {
			// Create and return MyNamespace::MyComponent class object
			// MyNamespace::MyComponent should derived from MGE::BaseComponent.
			
			// This block of code is function body called with arguments:
			//  * MGE::NamedObject* parent
			//  * const pugi::xml_node& config
			//  * std::set<int>* typeIDs
			//  * int createdForID
			// see MGE::ComponentFactory::ComponentCreator for details.
			
			// Typical usage case:
			typeIDs->insert(classID);
			return new MGE::World3DObjectImpl(parent);
		}
	\endcode
 */
#define MGE_ACTOR_COMPONENT_CREATOR(COMPONENT, COMPONENT_NAME) \
	namespace MGE { namespace ComponentFactoryRegistration { \
		MGE::BaseComponent* create__ ## COMPONENT_NAME(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID); \
		bool setup__ ## COMPONENT_NAME(MGE::ComponentFactory* factory) { \
			factory->registerComponent( COMPONENT::classID, #COMPONENT_NAME, create__ ## COMPONENT_NAME ); \
			return true; \
		} \
	} } \
	MGE_REGISTER_ACTOR_COMPONENT(COMPONENT_NAME, MGE::ComponentFactoryRegistration::setup__ ## COMPONENT_NAME); \
	MGE::BaseComponent* MGE::ComponentFactoryRegistration::create__ ## COMPONENT_NAME(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID)

#define MGE_ACTOR_COMPONENT_DEFAULT_CREATOR(COMPONENT, COMPONENT_NAME) \
	MGE_ACTOR_COMPONENT_CREATOR(COMPONENT, COMPONENT_NAME) { \
		typeIDs->insert(COMPONENT::classID); \
		return new COMPONENT(parent); \
	}

/// @}
