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

/// @addtogroup CoreUtils
/// @{
/// @file

/**
 * @def FORCE_INLINE
 * 
 * @brief  Macro replaced to "inline" keyword with additional forcing attributes.
 * 
 * On Clang and GCC compilers (besides using `inline` keyword) attribute `always_inline` will be added.
 * 
 * @remark Macro can be set on build time to other value (by -D option), in this case will not be redefined here.
*/
#ifndef FORCE_INLINE
	#ifdef __GNUC__
		#define FORCE_INLINE  inline __attribute__((always_inline))
	#else
		#define FORCE_INLINE  inline
	#endif
#endif

/// @}
