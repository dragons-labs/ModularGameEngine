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
*/

#pragma   once

namespace MGE {

/// @addtogroup CoreUtils
/// @{
/// @file

/**
 * @brief Adaptor for std::map / std::set / ... containers equal_range() to use in range-based for loop.
 * 
 * \par Example
	\code{.cpp}
		std::multimap<int, void*> s;
		// ...
		for (auto&& [key, val] i : MGE::Range(s, 13)) {
			// iteration over s element with key value == 13
			// key will be substituted as `key`, value as `val`
		}
	\endcode
 */
template<typename Container, typename Key> struct Range {
	Range(Container& c, const Key& k) {
		auto p = c.equal_range(k);
		b = p.first;
		e = p.second;
	}
	typename Container::iterator begin() const { return b; }
	typename Container::iterator end() const { return e; }
protected:
	typename Container::iterator b;
	typename Container::iterator e;
};

}
