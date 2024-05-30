/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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
#include <CEGUI/CEGUI.h>

/// @addtogroup GUI_Utils
/// @{
/// @file

/// set C style null-end string as CEGUI UTF8 data
#define CEGUI_UTF8(txt) reinterpret_cast<const char *>(txt)

/// convert CEGUI::String to std::string
/// @warning Due to CEGUI_STRING_CLASS_UTF_32 mode compatibility NEVER store/assign value returned by this macro. Only to use in functions arguments.
#if (CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_UTF_32)
#define STRING_FROM_CEGUI(a)  CEGUI::String::convertUtf32ToUtf8(a.data(), a.length())
#else
#define STRING_FROM_CEGUI(a)  a.getString()
#endif

/// convert std::string / std::string_view to CEGUI::String
#if (CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_UTF_32)
#define STRING_TO_CEGUI(a)  CEGUI::String(CEGUI_UTF8(a.data()), a.length())
#else
#define STRING_TO_CEGUI(a)  CEGUI::String(a.data(), a.length())
#endif

/// @}
