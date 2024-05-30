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
#include "ScriptsSystem.h"
#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	struct PythonEventMsg : MGE::EventMsg  {
		std::string type;
		
		const std::string_view getType() const override final {
			return type;
		}
		
		PythonEventMsg(const std::string_view& _type) :
			type(_type)
		{}
	};
	
	void msgReceiver(const std::string& scriptName, const MGE::EventMsg* eventMsg, void* receiverID) {
		MGE::ScriptsSystem::getPtr()->runObjectWithVoid( scriptName.c_str(), eventMsg, receiverID );
	}
	
	void sendMessage(const MessagesSystem* msgSys, const PythonEventMsg* msg, const py::object& sender /*void* sender*/) {
		void* sender_ptr = sender.is_none() ? 0 : sender.ptr();
		return msgSys->sendMessage( msg, sender_ptr );
	}
	
	bool registerReceiver(
		MessagesSystem* msgSys,
		const std::string_view& messageTypeName,
		const std::string_view& scriptName,
		const py::object& ownerID,
		const py::object& ownerSubID,
		const py::object& receivOnlyFrom
	) {
		return msgSys->registerReceiver(
			messageTypeName,
			std::bind(msgReceiver, std::string(scriptName), std::placeholders::_1, std::placeholders::_2),
			ownerID.is_none() ? 0 : ownerID.ptr(),
			ownerSubID.is_none() ? 0 : ownerSubID.ptr(),
			receivOnlyFrom.is_none() ? 0 : receivOnlyFrom.ptr()
		);
	}
	
	void unregisterReceiver(
		MessagesSystem* msgSys,
		const std::string_view& messageTypeName,
		const std::string_view& scriptName,
		const py::object& ownerID,
		const py::object& ownerSubID,
		const py::object& receivOnlyFrom
	) {
		return msgSys->unregisterReceiver(
			messageTypeName,
			std::bind(msgReceiver, std::string(scriptName), std::placeholders::_1, std::placeholders::_2),
			ownerID.is_none() ? 0 : ownerID.ptr(),
			ownerSubID.is_none() ? 0 : ownerSubID.ptr(),
			receivOnlyFrom.is_none() ? 0 : receivOnlyFrom.ptr()
		);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(MessagesSystem) {
	py::class_<MGE::MessagesSystem, std::unique_ptr<MGE::MessagesSystem, py::nodelete>>(
		m, "MessagesSystem", DOC(MGE, MessagesSystem)
	)
		.def("registerReceiver", &MGE::ScriptsInterface::registerReceiver,
			 DOC(MGE, MessagesSystem, registerReceiver),
			 py::arg("messageTypeName"), py::arg("scriptName"), py::arg("ownerID"), py::arg("ownerSubID") = py::none(), py::arg("receivOnlyFrom") = py::none()
		)
		.def("unregisterReceiver", &MGE::ScriptsInterface::unregisterReceiver,
			 DOC(MGE, MessagesSystem, unregisterReceiver),
			 py::arg("messageTypeName"), py::arg("scriptName"), py::arg("ownerID"), py::arg("ownerSubID") = py::none(), py::arg("receivOnlyFrom") = py::none()
		)
		.def("sendMessage", &MGE::ScriptsInterface::sendMessage,
			 DOC(MGE, MessagesSystem, sendMessage),
			 py::arg("msg"), py::arg("sender") = py::none()
		)
	;
	py::class_<MGE::ScriptsInterface::PythonEventMsg>(m, "EventMsg", DOC(MGE, EventMsg))
		.def(py::init<const std::string &>(), "constructor from \"message type\" string")
		.def("getType", &MGE::ScriptsInterface::PythonEventMsg::getType)
	;
	py::class_<MGE::EventMsg>(m, "EventMsgCpp", "Base python class for MGE::EventMsg derivered messages. Do not use directly this class. Use EventMsg instead.")
		.def("getType", &MGE::EventMsg::getType)
	;
}
#endif
