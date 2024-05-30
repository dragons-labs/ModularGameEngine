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

#include "data/structs/factories/PrototypeFactory.h"
#include "LogSystem.h"

MGE::PrototypeFactory::PrototypeFactory() {
	LOG_INFO("Create PrototypeFactory");
}

MGE::BasePrototype* MGE::PrototypeFactory::getPrototype(
	const std::string_view& name,
	const std::string_view& fileName,
	const std::string_view& fileGroup
) {
	LOG_INFO("Search prototype with: name=" + name + " file=" + fileName + " group=" + fileGroup);
	MGE::BasePrototypeImpl* ret = NULL;
	
	auto iter = allPrototypes.find(name);
	if (iter != allPrototypes.end()) {
		LOG_INFO("found prototype: " << iter->second);

		return iter->second;
	}
	
	// try create new global accessible actor prototype
	if (name.empty() || fileName.empty() || fileGroup.empty()) {
		LOG_WARNING("No valid config info for create prototype");
	} else {
		try {
			ret = new MGE::BasePrototypeImpl(
				static_cast<std::string>(name),
				static_cast<std::string>(fileName),
				static_cast<std::string>(fileGroup)
			);
			allPrototypes[ret->config.name] = ret;
			LOG_INFO("created (new) prototype: " << ret);
		} catch(std::exception& e) {
			LOG_ERROR("create prototype fail: " << e.what());
			ret = NULL;
		} catch(...) {
			LOG_ERROR("create prototype fail");
			ret = NULL;
		}
	}
	
	return ret;
}
