/*
Copyright (c) 2020-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "MessagesSystem.h"

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief Structure for actor created message (send @b after creating new actor).
 */
struct ActorCreatedEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActorCreated"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// created actor
	MGE::BaseActor* actor;
	
protected:
	friend struct ActorFactory;
	
	/// constructor
	ActorCreatedEventMsg(MGE::BaseActor* _actor) :
		actor(_actor)
	{}
};

/**
 * @brief Structure for actor destroy message (send @b before deleting actor).
 */
struct ActorDestroyEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActorDestroy"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// actor to destroy
	MGE::BaseActor* actor;
	
protected:
	friend struct ActorFactory;
	
	/// constructor
	ActorDestroyEventMsg(MGE::BaseActor* _actor) :
		actor(_actor)
	{}
};

/**
 * @brief Structure for actor available message (send in setAvailable call).
 */
struct ActorAvailableEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActorAvailable"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// available actor
	MGE::BaseActor* actor;
	
protected:
	friend class SelectableObject;
	
	/// constructor
	ActorAvailableEventMsg(MGE::BaseActor* _actor) :
		actor(_actor)
	{}
};

/**
 * @brief Structure for actor not available message (send in setAvailable call).
 */
struct ActorNotAvailableEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActorNotAvailable"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// not available destroy
	MGE::BaseActor* actor;
	
protected:
	friend class SelectableObject;
	
	/// constructor
	ActorNotAvailableEventMsg(MGE::BaseActor* _actor) :
		actor(_actor)
	{}
};


/// @}

}
