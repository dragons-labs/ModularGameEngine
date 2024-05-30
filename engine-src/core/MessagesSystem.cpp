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

#include "MessagesSystem.h"
#include "LogSystem.h"

MGE::MessagesSystem::MessagesSystem() {
	LOG_INFO("Initialize Message System");
}

void MGE::MessagesSystem::sendMessage(const EventMsg* msg, const void* sender) const {
	const std::string_view& msgTypeName = msg->getType();
	
	auto iter = receiversMap.find( msgTypeName );
	if (iter != receiversMap.end()) {
		for (auto& iter2 : iter->second) {
			if (iter2.onlyFrom == NULL || iter2.onlyFrom == sender || sender == NULL) {
				LOG_DEBUG("MessagesSystem", "send: " << msgTypeName << " from: " << sender << " to " << iter2.receiverID );
				iter2.exec(msg, iter2.receiverID);
			} else {
				LOG_DEBUG("MessagesSystem", "skip: " << msgTypeName << " from: " << sender << " to " << iter2.receiverID << " due to filter");
			}
		}
	} else {
		LOG_VERBOSE("MessagesSystem", "no receivers for: " << msgTypeName);
	}
}

bool MGE::MessagesSystem::registerReceiver(
	const std::string_view& messageTypeName,
	const MsgReceiverFunction& receiverFunction,
	void* regOwnerID,
	void* regOwnerSubID,
	void* receivOnlyFrom
) {
	LOG_VERBOSE("MessagesSystem", "register receiver for message type: " << messageTypeName << " for: " << regOwnerID << "/" << regOwnerSubID << " with filter: " << receivOnlyFrom);
	
	// create object for new registration
	ReceiverInfo receiverInfo(receiverFunction, regOwnerID, regOwnerSubID, receivOnlyFrom);
	
	// find set of ReceiverInfo for used messageTypeName
	auto receiversSetIter = receiversMap.find(messageTypeName);
	if (receiversSetIter != receiversMap.end()) {
		// check if is not already registered
		if (receiversSetIter->second.find(receiverInfo) != receiversSetIter->second.end()) {
			LOG_ERROR("MessagesSystem", "receiver for message type: " << messageTypeName << " for: " << regOwnerID << "/" << regOwnerSubID << " with filter: " << receivOnlyFrom << " already registered");
			return false;
		}
		
		// insert (move) receiverInfo to existed set
		receiversSetIter->second.insert(std::move(receiverInfo));
	} else {
		LOG_INFO("MessagesSystem", "Register new message type: " << messageTypeName);
		// create new set (for messageTypeName) and insert (move) receiverInfo into it
		receiversMap.insert({std::string(messageTypeName), {std::move(receiverInfo)}});
	}
	
	return true;
}

void MGE::MessagesSystem::unregisterReceiver(
	const std::string_view& messageTypeName,
	const MsgReceiverFunction& receiverFunction,
	void* regOwnerID,
	void* regOwnerSubID,
	void* receivOnlyFrom
) {
	auto receiversSetIter = receiversMap.find(messageTypeName);
	if (receiversSetIter != receiversMap.end()) {
		if (receiversSetIter->second.erase( ReceiverInfo(receiverFunction, regOwnerID, regOwnerSubID, receivOnlyFrom) ) == 0) {
			LOG_WARNING("MessagesSystem", "try remove non registered receiver for message type: " << messageTypeName << " for: " << regOwnerID << "/" << regOwnerSubID << " with filter: " << receivOnlyFrom);
		} else {
			LOG_VERBOSE("MessagesSystem", "remove receiver for message type: " << messageTypeName << " for: " << regOwnerID << "/" << regOwnerSubID);
		}
	} else {
		LOG_WARNING("MessagesSystem", "try remove receiver for non registered message type: " << messageTypeName << " for: " << regOwnerID << "/" << regOwnerSubID);
	}
}

void MGE::MessagesSystem::unregisterReceiver(void* regOwnerID, void* regOwnerSubID, bool ignoreOwnerSubID) {
	for (auto receiversSet : receiversMap) {
		for (auto iter = receiversSet.second.begin(); iter!=receiversSet.second.end(); ) { // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
			if (iter->receiverID == regOwnerID && (ignoreOwnerSubID || iter->receiverInternalID == regOwnerSubID))
			receiversSet.second.erase(iter++);
		}
	}
}
