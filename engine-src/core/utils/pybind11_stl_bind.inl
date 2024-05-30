/*
Copyright (c) 2022-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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


-- Based on: pybind11/stl_bind.h, original copyright information follows --

Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Please also refer to the file .github/CONTRIBUTING.md, which clarifies licensing of
external contributions to this project including patches, pull requests, etc.

-- End original copyright --
*/

#pragma   once

#include "pragma.h"

#include <pybind11/stl_bind.h>

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace detail {
	template <typename, typename, typename... Args>
	void list_if_copy_constructible(const Args &...) {}
	template <typename, typename, typename... Args>
	void list_if_equal_operator(const Args &...) {}
	
	template <typename ListType>
	std::unique_ptr<ListType> crateList(const pybind11::list& py_list) {
		std::unique_ptr<ListType> l(new ListType);
		for (auto x : py_list) {
			l->push_back(x.cast<typename ListType::value_type>());
		}
		return l;
	}
	
	template <typename ListType, typename PyBindClass>
	void list_if_copy_constructible(
		std::enable_if_t<
		    pybind11::detail::is_copy_constructible<ListType>::value
		&& !pybind11::detail::cast_is_temporary_value_reference<typename ListType::value_type>::value, PyBindClass> &bindClass
	) {
		bindClass.def(pybind11::init<const ListType &>(), "Copy constructor");
		bindClass.def(pybind11::init(&crateList<ListType>));
	}
	
	template <typename ListType, typename PyBindClass>
	void list_if_equal_operator(std::enable_if_t<pybind11::detail::is_comparable<ListType>::value, PyBindClass> &bindClass) {
		using ValueType = typename ListType::value_type;
		
		bindClass.def("__contains__",  [](ListType &self, const ValueType& e) -> bool { return std::find(self.begin(), self.end(), e) != self.end(); });
		bindClass.def("__contains__",  [](ListType &, const pybind11::object& ) -> bool { return false; }); // when the object is not of ValueType type
		bindClass.def("find",
			[](ListType &self, typename ListType::value_type e) -> int {
				typename ListType::size_type i = 0;
				for (auto iter = self.begin(); iter != self.end(); ++iter, ++i) {
					if (*iter == e) { return i; }
				}
				return -1;
			}, 
			"return index of first occurrence od element `e`"
		);
	}
	
	MGE_GNUC_WARNING_IGNORED("-Wsign-compare")
	
	template <typename ListType>
	typename ListType::value_type get_list_element_at(ListType &self, int i) {
		if (i < 0) i = self.size() + i;
		if (i >= self.size() || i < 0) throw pybind11::index_error();
		for (auto iter = self.begin(); iter != self.end(); ++iter) {
			if (--i < 0) {
				return *iter;
			}
		}
		throw pybind11::index_error();
	}
	
	template <typename ListType>
	void erase_list_element_at(ListType &self, int i) {
		if (i < 0) i = self.size() + i;
		if (i >= self.size() || i < 0) throw pybind11::index_error();
		for (auto iter = self.begin(); iter != self.end(); ++iter) {
			if (--i < 0) {
				self.erase(iter);
				break;
			}
		}
	}
	
	template <typename ListType>
	void insert_list_element_at(ListType &self, int i, typename ListType::value_type e) {
		if (i < 0) i = self.size() + i;
		if (i >= self.size() || i < 0) throw pybind11::index_error();
		for (auto iter = self.begin(); iter != self.end(); ++iter) {
			if (--i < 0) {
				self.insert(iter, e);
				break;
			}
		}
	}
	
	MGE_GNUC_WARNING_POP
	
	template <typename ListType>
	inline void throwOnEmpty(ListType &self) {
		if (self.empty()) throw pybind11::index_error();
	}
} }
#endif // __DOCUMENTATION_GENERATOR__

namespace MGE {
	/// @addtogroup ScriptsSystem
	/// @{
	/// @file
	
	/**
	 * @brief pybind11 bind function for const std::list.
	 */
	template <typename ListType, typename holder_type = std::unique_ptr<ListType>, typename... Args>
	pybind11::class_<ListType, holder_type> py_bind_const_list(pybind11::handle scope, const char* name, Args &&...args) {
		pybind11::class_<ListType, holder_type> bindClass(scope, name, std::forward<Args>(args)...);
		
		bindClass.def(pybind11::init<>());
		
		bindClass.def("__iter__",
			[](const ListType &self) { return pybind11::make_iterator<pybind11::return_value_policy::reference>(self.begin(), self.end()); },
			pybind11::keep_alive<0, 1>(), // keep list alive while using iterator
			"iterate over elements in the container"
		);
		
		MGE::detail::list_if_equal_operator<ListType, pybind11::class_<ListType, holder_type>>(bindClass);
		
		bindClass.def("size",          &ListType::size,          "returns the number of elements");
		bindClass.def("__len__",       &ListType::size);
		
		bindClass.def("empty",         &ListType::empty,         "checks whether the container is empty");
		bindClass.def("__bool__",      [](const ListType &m) -> bool { return !m.empty(); });
		
		bindClass.def("back",          [](ListType &self) {MGE::detail::throwOnEmpty(self); return self.back();},   "access the last element " );
		bindClass.def("front",         [](ListType &self) {MGE::detail::throwOnEmpty(self); return self.front();},  "access the first element" );
		bindClass.def("__getitem__",   &MGE::detail::get_list_element_at<ListType>);
		
		return bindClass;
	}
	/**
	 * @brief pybind11 bind function for std::list.
	 */
	template <typename ListType, typename holder_type = std::unique_ptr<ListType>, typename... Args>
	pybind11::class_<ListType> py_bind_list(pybind11::handle scope, const char* name, Args &&...args) {
		using ValueType = typename ListType::value_type;
		
		auto bindClass = py_bind_const_list<ListType, holder_type, Args...>(scope, name, std::forward<Args>(args)...);
		
		MGE::detail::list_if_copy_constructible<ListType, pybind11::class_<ListType, holder_type>>(bindClass);
		
		bindClass.def("push_back",     pybind11::overload_cast<const ValueType&>(&ListType::push_back),    "adds an element to the end");
		bindClass.def("append",        pybind11::overload_cast<const ValueType&>(&ListType::push_back),    "adds an element to the end"); // for python list compatibility
		bindClass.def("pop_back",      &ListType::pop_back,                                                "removes the last element");
		
		bindClass.def("push_front",    pybind11::overload_cast<const ValueType&>(&ListType::push_front),   "adds an element to the front");
		bindClass.def("pop_front",     &ListType::pop_front,                                               "removes the first element");
		
		bindClass.def("insert",        &MGE::detail::insert_list_element_at<ListType>,                     "insert element `e` at index `i`");
		bindClass.def("erase",         &MGE::detail::erase_list_element_at<ListType>,                      "erases element at index `i`");
		bindClass.def("__delitem__",   &MGE::detail::erase_list_element_at<ListType>);
		
		bindClass.def("clear",         &ListType::clear,         "clears the contents");
		bindClass.def("swap",          &ListType::swap,          "swaps the contents");
		bindClass.def("reverse",       &ListType::reverse,       "reverse list");
		
		return bindClass;
	}
	
	/**
	 * @brief pybind11 bind function for const std::set.
	 */
	template <typename SetType, typename holder_type = std::unique_ptr<SetType>, typename... Args>
	pybind11::class_<SetType> py_bind_const_set(pybind11::handle scope, const char* name, Args &&...args) {
		using ValueType = typename SetType::value_type;
		
		pybind11::class_<SetType, holder_type> bindClass(scope, name, std::forward<Args>(args)...);
		
		bindClass.def(pybind11::init<>());
		
		bindClass.def("__iter__",
			[](const SetType &self) { return pybind11::make_iterator<pybind11::return_value_policy::reference>(self.begin(), self.end()); },
			pybind11::keep_alive<0, 1>(), // keep set alive while using iterator
			"iterate over elements in the container"
		);
		
		bindClass.def("__contains__",  [](SetType &self, const ValueType& e) -> bool { return self.find(e) != self.end(); });
		bindClass.def("__contains__",  [](SetType &, const pybind11::object& ) -> bool { return false; }); // when the object is not of ValueType type
		
		bindClass.def("size",          &SetType::size,                                              "returns the number of elements");
		bindClass.def("__len__",       &SetType::size);
		
		bindClass.def("empty",         &SetType::empty,                                             "checks whether the container is empty");
		bindClass.def("__bool__",      [](const SetType &self) -> bool { return !self.empty(); });
		
		return bindClass;
	}
	
	/**
	 * @brief pybind11 bind function for std::set.
	 */
	template <typename SetType, typename holder_type = std::unique_ptr<SetType>, typename... Args>
	pybind11::class_<SetType> py_bind_set(pybind11::handle scope, const char* name, Args &&...args) {
		using ValueType = typename SetType::value_type;
		
		auto bindClass = py_bind_const_set<SetType, holder_type, Args...>(scope, name, std::forward<Args>(args)...);
		
		bindClass.def("insert",        [](SetType &self, const ValueType& e) { self.insert(e); },   "adds an element to set" );
		bindClass.def("erase",         pybind11::overload_cast<const ValueType&>(&SetType::erase),  "removes element from set");
		bindClass.def("__delitem__",   pybind11::overload_cast<const ValueType&>(&SetType::erase));
		
		bindClass.def("clear",         &SetType::clear,                                             "clears the contents");
		bindClass.def("swap",          &SetType::swap,                                              "swaps the contents");
		
		return bindClass;
	}
	
	/**
	 * @brief pybind11 bind function for const std::map.
	 */
	template <typename MapType, typename holder_type = std::unique_ptr<MapType>, typename... Args>
	pybind11::class_<MapType, holder_type> py_bind_const_map(pybind11::handle scope, const std::string &name, Args &&...args) {
		using KeyType = typename MapType::key_type;
		using MappedType = typename MapType::mapped_type;
		using Class_ = pybind11::class_<MapType, holder_type>;
		
		// If either type is a non-module-local bound type then make the map binding non-local as well;
		// otherwise (e.g. both types are either module-local or converting) the map will be
		// module-local.
		auto *tinfo = pybind11::detail::get_type_info(typeid(MappedType));
		bool local = !tinfo || tinfo->module_local;
		if (local) {
			tinfo = pybind11::detail::get_type_info(typeid(KeyType));
			local = !tinfo || tinfo->module_local;
		}
		
		Class_ bindClass(scope, name.c_str(), pybind11::module_local(local), std::forward<Args>(args)...);
		
	#if PYBIND11_VERSION_MAJOR == 2 && PYBIND11_VERSION_MINOR >= 12
		using KeysView = pybind11::detail::keys_view;
		using ValuesView = pybind11::detail::values_view;
		using ItemsView = pybind11::detail::items_view;
		
		// Wrap KeysView if it wasn't already wrapped
		if (!pybind11::detail::get_type_info(typeid(KeysView))) {
			pybind11::class_<KeysView> keys_view(scope, "KeysView", pybind11::module_local(local));
			keys_view.def("__len__", &KeysView::len);
			keys_view.def("__iter__",
						&KeysView::iter,
						pybind11::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
			);
			keys_view.def("__contains__", &KeysView::contains);
		}
		// Similarly for ValuesView:
		if (!pybind11::detail::get_type_info(typeid(ValuesView))) {
			pybind11::class_<ValuesView> values_view(scope, "ValuesView", pybind11::module_local(local));
			values_view.def("__len__", &ValuesView::len);
			values_view.def("__iter__",
							&ValuesView::iter,
							pybind11::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
			);
		}
		// Similarly for ItemsView:
		if (!pybind11::detail::get_type_info(typeid(ItemsView))) {
			pybind11::class_<ItemsView> items_view(scope, "ItemsView", pybind11::module_local(local));
			items_view.def("__len__", &ItemsView::len);
			items_view.def("__iter__",
						&ItemsView::iter,
						pybind11::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
			);
		}
	#elif PYBIND11_VERSION_HEX >= 0x020A0200
		using StrippedKeyType = pybind11::detail::remove_cvref_t<KeyType>;
		using StrippedMappedType = pybind11::detail::remove_cvref_t<MappedType>;
		using KeysView = pybind11::detail::keys_view<StrippedKeyType>;
		using ValuesView = pybind11::detail::values_view<StrippedMappedType>;
		using ItemsView = pybind11::detail::items_view<StrippedKeyType, StrippedMappedType>;
		static constexpr auto key_type_descr = pybind11::detail::make_caster<KeyType>::name;
		static constexpr auto mapped_type_descr = pybind11::detail::make_caster<MappedType>::name;
		std::string key_type_name(key_type_descr.text), mapped_type_name(mapped_type_descr.text);

		// If key type isn't properly wrapped, fall back to C++ names
		if (key_type_name == "%") {
			key_type_name = pybind11::detail::type_info_description(typeid(KeyType));
		}
		// Similarly for value type:
		if (mapped_type_name == "%") {
			mapped_type_name = pybind11::detail::type_info_description(typeid(MappedType));
		}

		// Wrap KeysView[KeyType] if it wasn't already wrapped
		if (!pybind11::detail::get_type_info(typeid(KeysView))) {
			pybind11::class_<KeysView> keys_view(
				scope, ("KeysView[" + key_type_name + "]").c_str(), pybind11::module_local(local));
			keys_view.def("__len__", &KeysView::len);
			keys_view.def("__iter__",
						&KeysView::iter,
						pybind11::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
			);
			keys_view.def("__contains__",
						static_cast<bool (KeysView::*)(const KeyType &)>(&KeysView::contains));
			// Fallback for when the object is not of the key type
			keys_view.def("__contains__",
						static_cast<bool (KeysView::*)(const pybind11::object &)>(&KeysView::contains));
		}
		// Similarly for ValuesView:
		if (!pybind11::detail::get_type_info(typeid(ValuesView))) {
			pybind11::class_<ValuesView> values_view(scope,
										("ValuesView[" + mapped_type_name + "]").c_str(),
										pybind11::module_local(local));
			values_view.def("__len__", &ValuesView::len);
			values_view.def("__iter__",
							&ValuesView::iter,
							pybind11::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
			);
		}
		// Similarly for ItemsView:
		if (!pybind11::detail::get_type_info(typeid(ItemsView))) {
			pybind11::class_<ItemsView> items_view(
				scope,
				("ItemsView[" + key_type_name + ", ").append(mapped_type_name + "]").c_str(),
				pybind11::module_local(local));
			items_view.def("__len__", &ItemsView::len);
			items_view.def("__iter__",
						&ItemsView::iter,
						pybind11::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
			);
		}
	#else
		#error "Too old version of pybind 11"
	#endif // PYBIND11_VERSION...
		
		bindClass.def(pybind11::init<>());
		
		bindClass.def(
			"__bool__",
			[](const MapType &self) -> bool { return !self.empty(); },
			"Check whether the map is nonempty");
		
		bindClass.def(
			"__iter__",
			[](MapType &self) { return pybind11::make_key_iterator(self.begin(), self.end()); },
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while iterator exists */
		);
		
	#if PYBIND11_VERSION_MAJOR == 2 && PYBIND11_VERSION_MINOR >= 12
		bindClass.def(
			"keys",
			[](MapType &self) { return std::unique_ptr<KeysView>(new pybind11::detail::KeysViewImpl<MapType>(self)); },
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while view exists */
		);
		
		bindClass.def(
			"values",
			[](MapType &self) { return std::unique_ptr<ValuesView>(new pybind11::detail::ValuesViewImpl<MapType>(self)); },
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while view exists */
		);
		
		bindClass.def(
			"items",
			[](MapType &self) { return std::unique_ptr<ItemsView>(new pybind11::detail::ItemsViewImpl<MapType>(self)); },
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while view exists */
		);
	#else
		bindClass.def(
			"keys",
			[](MapType &self) {
				return std::unique_ptr<KeysView>(new pybind11::detail::KeysViewImpl<MapType, KeysView>(self));
			},
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while view exists */
		);
		
		bindClass.def(
			"values",
			[](MapType &self) {
				return std::unique_ptr<ValuesView>(new pybind11::detail::ValuesViewImpl<MapType, ValuesView>(self));
			},
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while view exists */
		);
		
		bindClass.def(
			"items",
			[](MapType &self) {
				return std::unique_ptr<ItemsView>(new pybind11::detail::ItemsViewImpl<MapType, ItemsView>(self));
			},
			pybind11::keep_alive<0, 1>() /* Essential: keep map alive while view exists */
		);
	#endif // PYBIND11_VERSION_MAJOR == 2 && PYBIND11_VERSION_MINOR >= 12
		
		bindClass.def(
			"__getitem__",
			[](MapType &self, const KeyType &k) -> MappedType & {
				auto it = self.find(k);
				if (it == self.end()) {
					throw pybind11::key_error();
				}
				return it->second;
			},
			pybind11::return_value_policy::reference_internal // ref + keepalive
		);
		
		bindClass.def(
			"__contains__",
			[](MapType &self, const KeyType &k) -> bool {
				auto it = self.find(k);
				if (it == self.end()) {
					return false;
				}
				return true;
			}
		);
		// Fallback for when the object is not of the key type
		bindClass.def("__contains__", [](MapType &, const pybind11::object &) -> bool { return false; });
		
		// Always use a lambda in case of `using` declaration
		bindClass.def("__len__", [](const MapType &self) { return self.size(); });
		
		return bindClass;
	}
	
	/**
	 * @brief pybind11 bind function for std::map.
	 */
	template <typename MapType, typename holder_type = std::unique_ptr<MapType>, typename... Args>
	pybind11::class_<MapType, holder_type> py_bind_map(pybind11::handle scope, const std::string &name, Args &&...args) {
		using KeyType = typename MapType::key_type;
		using Class_ = pybind11::class_<MapType, holder_type>;
		
		auto bindClass = py_bind_const_set<MapType, holder_type, Args...>(scope, name, std::forward<Args>(args)...);
		
		// Register stream insertion operator (if possible)
		pybind11::detail::map_if_insertion_operator<MapType, Class_>(bindClass, name);
		
		// Assignment provided only if the type is copyable
		pybind11::detail::map_assignment<MapType, Class_>(bindClass);
		
		bindClass.def(
			"__delitem__",
			[](MapType &self, const KeyType &k) {
				auto it = self.find(k);
				if (it == self.end()) {
					throw pybind11::key_error();
				}
				self.erase(it);
			}
		);
		return bindClass;
	}
	
	/// @}
}
