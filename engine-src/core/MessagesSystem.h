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

#include "force_inline.h"

#include <string>
#include <string_view>

#include <functional>
#include <map>
#include <set>

namespace MGE {

/// @addtogroup MessagesSystem
/// @{
/// @file

/**
 * @brief Base class for event message.
 * 
 * Any message class should derived by EventMsg. Example:
	\code{.cpp}
		struct MyMsg : MGE::EventMsg  {
			/// message type string
			inline static const std::string_view MsgType = "MyMsg"sv;
			
			/// @copydoc MGE::EventMsg::getType
			const std::string_view getType() const override final {
				return MsgType;
			}
		protected:
			friend MGE::MyMsgSender;
			MyMsg() {}
		};
	\endcode
 */
struct EventMsg {
	/// return message type string
	virtual const std::string_view getType() const = 0;
	
	/// destructor
	virtual ~EventMsg() = default;
};

/**
 * @brief Event message system.
 * 
 */
class MessagesSystem {
public:
	/// @{
	/**
	 * @brief Send event message.
	 * 
	 * @param msg message to send
	 * @param sender (optional) pointer to sender object, used for filtering
	 * 
	 * @note Function returns after all registred receivers recive and proccessing message,
	 *       so message can be deleted after this function exit.
	 */
	void sendMessage(const EventMsg* msg, const void* sender = NULL) const;
	
	FORCE_INLINE void sendMessage(const EventMsg&& msg, const void* sender = NULL) const {
		return sendMessage(&msg);
	}
	/// @}
	
	/**
	 * @brief Type of function to receive event message.
	 * 
	 * This is two argument function or class member function.
	 * - First argument is a pointer to message. This pointer can be deleted after this function return (do not store this pointer).
	 * - Second argument is regOwnerID used in @ref registerReceiver.
	 */
	typedef std::function<void(const EventMsg* eventMsg, void* regOwnerID)> MsgReceiverFunction;
	
	/**
	 * @brief Register event message receiver.
	 * 
	 * @param messageTypeName  Message type name for messages that we want to receive.
	 * @copydetails ReceiverInfo::ReceiverInfo
	 * 
	 * @return true on success, false on error.
	 */
	bool registerReceiver(const std::string_view& messageTypeName, const MsgReceiverFunction& receiverFunction, void* regOwnerID, void* regOwnerSubID = nullptr, void* receivOnlyFrom = nullptr);
	
	/**
	 * @brief Unregister event message receiver.
	 * 
	 * @param messageTypeName  Message type name for messages that we want to receive.
	 * @param receiverFunction Function or class member function to unregistered.
	 * @param regOwnerID       Pointer (void*) to registration owner.
	 * @param regOwnerSubID    Value of @a regOwnerSubID used on registration via @ref registerReceiver.
	 * @param receivOnlyFrom   Value of @a receivOnlyFrom used on registration via @ref registerReceiver.
	 */
	void unregisterReceiver(const std::string_view& messageTypeName, const MsgReceiverFunction& receiverFunction, void* regOwnerID, void* regOwnerSubID = nullptr, void* receivOnlyFrom = nullptr);
	
	/**
	 * @brief Unregister event message receivers registered for provided @a regOwnerID (and @a regOwnerSubID if @ref ignoreOwnerSubID == false).
	 * 
	 * @param regOwnerID       Pointer (void*) to registration owner.
	 * @param regOwnerSubID    Value of @a regOwnerSubID used on registration via @ref registerReceiver.
	 * @param ignoreOwnerSubID When true do not compare value of @a regOwnerSubID
	 */
	void unregisterReceiver(void* regOwnerID, void* regOwnerSubID = nullptr, bool ignoreOwnerSubID = false);
	
	/// constructor
	MessagesSystem();
	
protected:
	/**
	 * @brief Struct for store info about C++ message subscriber.
	 * 
	 */
	struct ReceiverInfo {
		/// Callback function.
		MsgReceiverFunction exec;
		
		/// Unique subscriber ID.
		void* receiverID;
		
		/// Unique ID of registration for specific @a receiverID.
		/// @remark we need this because `exec.target<>()` need exactly type of registered function,
		///         so ``exec.target<void(*)(const EventMsg* eventMsg, void*)>()`` return 0 for functions registered by std::bind
		void* receiverInternalID;
		
		/// Sender filter value (NULL == disable filtering).
		void* onlyFrom;
		
		/// %Compare operation (compare based on values of all field without @a exec). Needed to use in std::set.
		bool operator <(const ReceiverInfo& x) const {
			return receiverID < x.receiverID || receiverInternalID < x.receiverInternalID || onlyFrom < x.onlyFrom;
		}
		
		/**
		 * @brief Constructor - init all struct field
		 * 
		 * @param receiverFunction Function or class member function for receive messages.
		 * @param regOwnerID       Pointer (void*) to registration owner, used as unregister key (will be passed to executed function as second argument too).
		 * @param regOwnerSubID    Owner internal ID of receiver function (need only in case of register multiple function with the same @a regOwnerID and @a receivOnlyFrom)
		 *                         (can be function address or other numeric value).
		 * @param receivOnlyFrom   Optional filter value to compare with @c sender value from @ref sendMessage, default NULL == no filtering.
		 */
		ReceiverInfo(const MsgReceiverFunction& receiverFunction, void* regOwnerID, void* regOwnerSubID, void* receivOnlyFrom) :
			exec(receiverFunction), receiverID(regOwnerID), receiverInternalID(regOwnerSubID), onlyFrom(receivOnlyFrom)
		{}
	};
	
	/**
	 * @brief Message type --> set of receivers (subscribing this type of message) map.
	 * 
	 * Key of this std::map is message type string. Values is set of registered receivers (represented by @ref ReceiverInfo).
	 * 
	 * @remark Used ``std::less<>`` to accept ``std::string_view`` on ``receiversMap.find()`` and compare it with key value (``std::string``)
	 *         without creating ``std::string`` (by ``string_view < string_view`` operator).
	 */
	std::map<std::string, std::set<ReceiverInfo>, std::less<>> receiversMap;
};
/// @}
}
