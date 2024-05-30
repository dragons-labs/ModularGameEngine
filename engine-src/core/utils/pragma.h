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

#ifdef __DOCUMENTATION_GENERATOR__
/// @addtogroup CoreUtils
/// @{
/// @file

/// Make #pragma "diagnostic push" and set up to 8  #pragma "diagnostic ignored" on GCC and Clang
#define MGE_GNUC_WARNING_IGNORED(...)
/// Make #pragma "diagnostic push" and set up to 8  #pragma "diagnostic ignored" on Clang (and not on GCC)
#define MGE_CLANG_WARNING_IGNORED(...)
/// Make #pragma "diagnostic push" and set up to 8  #pragma "diagnostic ignored" on GCC (and not on Clang)
#define MGE_GCC_WARNING_IGNORED(...)

/// Make #pragma "diagnostic pop" on GCC and Clang
#define MGE_GNUC_WARNING_POP
/// Make #pragma "diagnostic pop" on Clang (and not on GCC)
#define MGE_CLANG_WARNING_POP
/// Make #pragma "diagnostic pop" on GCC (and not on Clang)
#define MGE_GCC_WARNING_POP

/// @}

#else

#include <boost/preprocessor/facilities/overload.hpp>

#ifdef __GNUC__
	#define MGE_GNUC_WARNING_IGNORED(...)   MGE_WARNING_IGNORED( __VA_ARGS__)
	#define MGE_GNUC_WARNING_POP            MGE_WARNING_IGNORED_POP
	#ifdef __clang__
		#define MGE_CLANG_WARNING_IGNORED(...)  MGE_WARNING_IGNORED( __VA_ARGS__)
		#define MGE_CLANG_WARNING_POP           MGE_WARNING_IGNORED_POP
		#define MGE_GCC_WARNING_IGNORED(...)
		#define MGE_GCC_WARNING_POP
	#else
		#define MGE_CLANG_WARNING_IGNORED(...)
		#define MGE_CLANG_WARNING_POP
		#define MGE_GCC_WARNING_IGNORED(...)  MGE_WARNING_IGNORED( __VA_ARGS__)
		#define MGE_GCC_WARNING_POP           MGE_WARNING_IGNORED_POP
	#endif
#else
	#define MGE_GNUC_WARNING_IGNORED(...)
	#define MGE_CLANG_WARNING_IGNORED(...)
	#define MGE_GCC_WARNING_IGNORED(...)
	#define MGE_GNUC_WARNING_POP
	#define MGE_CLANG_WARNING_POP
	#define MGE_GCC_WARNING_POP
#endif

#define MGE_DO_PRAGMA(p)                               _Pragma(#p)

#define MGE_WARNING_IGNORED(...)                       BOOST_PP_OVERLOAD(MGE_WARNING_IGNORED_, __VA_ARGS__)(__VA_ARGS__)
#define MGE_WARNING_IGNORED_1(A)                       MGE_DO_PRAGMA(GCC diagnostic push)          MGE_DO_PRAGMA(GCC diagnostic ignored A )
#define MGE_WARNING_IGNORED_2(A, B)                    MGE_WARNING_IGNORED_1(A)                    MGE_DO_PRAGMA(GCC diagnostic ignored B )
#define MGE_WARNING_IGNORED_3(A, B, C)                 MGE_WARNING_IGNORED_2(A, B)                 MGE_DO_PRAGMA(GCC diagnostic ignored C )
#define MGE_WARNING_IGNORED_4(A, B, C, D)              MGE_WARNING_IGNORED_3(A, B, C)              MGE_DO_PRAGMA(GCC diagnostic ignored D )
#define MGE_WARNING_IGNORED_5(A, B, C, D, E)           MGE_WARNING_IGNORED_4(A, B, C, D)           MGE_DO_PRAGMA(GCC diagnostic ignored E )
#define MGE_WARNING_IGNORED_6(A, B, C, D, E, F)        MGE_WARNING_IGNORED_5(A, B, C, D, E)        MGE_DO_PRAGMA(GCC diagnostic ignored F )
#define MGE_WARNING_IGNORED_7(A, B, C, D, E, F, G)     MGE_WARNING_IGNORED_6(A, B, C, D, E, F)     MGE_DO_PRAGMA(GCC diagnostic ignored G )
#define MGE_WARNING_IGNORED_8(A, B, C, D, E, F, G, H)  MGE_WARNING_IGNORED_7(A, B, C, D, E, F, G)  MGE_DO_PRAGMA(GCC diagnostic ignored H )
#define MGE_WARNING_IGNORED_POP                        MGE_DO_PRAGMA(GCC diagnostic pop)

#endif
