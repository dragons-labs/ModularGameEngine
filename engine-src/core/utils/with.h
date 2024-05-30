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

#include <boost/preprocessor/facilities/overload.hpp>

/// @addtogroup CoreUtils
/// @{
/// @file

/**
 * @brief  Macro conditional running statement or block of code.
 *         Block will be executed only if first and third argument is evaluated to true.
 *         Value of first argument will be available inside block as variable with name provided by second argument.
 * 
 *         Typically first argument is function returned pointer (or pointer-like object) to check and (if not null) use in block.
 *         The function will be call only once (no separate calls for get value to check and for get value to use in block).
 * 
 * @param  value      Function / expression returning value to check if not null.
 * @param  var_name   (optional) Name of variable to set to value.
 *                    If not provided statement called on on @a value must be added directly to macro - see example bellow
 *                    (`WITH_NOT_NULL_TMP_VALUE` will be used as hidden variable).
 * @param  condition  (optional) Extra condition to check (must be true to execute block). Default true (not checked).
 * 
 * \par Example
	\code{.cpp}
		WITH_NOT_NULL(SomeSingleton::getPtr())->do_something(); // note `->` added directly to WITH_NOT_NULL macro
		
		// or
		
		WITH_NOT_NULL(SomeSingleton::getPtr(), WITH_AS) WITH_AS->do_something(); // note no `;` after WITH_NOT_NULL macro
		
		// or
		
		WITH_NOT_NULL(SomeSingleton::getPtr(), myPtr, !otherPtr->empty()) {
			myPrt->do_something1();
			myPrt->do_something2(otherObject);
		}
	\endcode
 *
 * @remark With -O1 flag `WITH_NOT_NULL(a)->do()` is the same as `if(a) a->do()`.
 *         On x86 its are 3 asm instructions: `test`, `je` (to label after `call`), `call`.
 *
 * @{
 */
#define WITH_NOT_NULL(...)                                        BOOST_PP_OVERLOAD(WITH_NOT_NULL_, __VA_ARGS__)(__VA_ARGS__)
#define WITH_NOT_NULL_3(value, var_name, condition)               if (auto var_name = (value); var_name && (condition))
#define WITH_NOT_NULL_2(value, var_name)                          if (auto var_name = (value))
#define WITH_NOT_NULL_1(value)                                    WITH_NOT_NULL_2(value, WITH_NOT_NULL_TMP_VALUE) WITH_NOT_NULL_TMP_VALUE
/// @}


/// @}
