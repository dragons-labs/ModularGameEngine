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

#ifndef __DOCUMENTATION_GENERATOR__

#include "pragma.h"

#if !defined(LOG_DEBUG) || !defined(LOG_WARNING)
	#include "LogSystem.h"
#endif

#include <pybind11/pybind11.h>
#include <type_traits>

MGE_GNUC_WARNING_IGNORED("-Wpragmas", "-Wold-style-cast", "-Wshadow", "-Wreserved-id-macro")
#define SWIGPYTHON_BUILTIN
#include "swigpyrun.h"
MGE_GNUC_WARNING_POP

struct PySwigObject {
    PyObject_HEAD
    void* ptr;
    const char* desc;
};

#define PYBIND11_SWIG_CONSTRUCTABLE(CLASS_TYPE, CLASS_NAME) \
    return SWIG_InternalNewPointerObj((new CLASS_TYPE(inObj)), getTypeInfo(), SWIG_POINTER_OWN );

#define PYBIND11_SWIG_NON_CONSTRUCTABLE(CLASS_TYPE, CLASS_NAME) \
    throw std::logic_error( "[C++ to Python] can't construct (copy) of " #CLASS_TYPE " object" );

#endif // __DOCUMENTATION_GENERATOR__

/// @addtogroup ScriptsSystem
/// @{
/// @file

/**
 * @brief Macro to add mapping of C++ type <--> Python type via external python module generated with SWIG.
 * 
 * @param CLASS_TYPE  C++ scoped type.
 * @param CLASS_NAME  Python type name (value returned by python type() function on object from external python module).
 * @param CLASS_MODE  One of following value:
 *                     - ``PYBIND11_SWIG_CONSTRUCTABLE`` for copy-constructable types
 *                     - ``PYBIND11_SWIG_CONSTRUCTABLE`` for non copy-constructable types
 *                      (can be returned (to python) as pointer, but return by value will throw exception)
 * 
 * @note Must be used outside any namespace.
 */
#define PYBIND11_SWIG_GENERATE_CAST(CLASS_TYPE, CLASS_NAME, CLASS_MODE)                                          \
namespace pybind11 { namespace detail {                                                                          \
    template <> struct type_caster< CLASS_TYPE > {                                                               \
    protected:                                                                                                   \
        CLASS_TYPE* value;                                                                                       \
        operator CLASS_TYPE &&() && { return std::move(*value); }                                                \
    public:                                                                                                      \
        static constexpr auto name = const_name(CLASS_NAME);                                                     \
        template <typename _T> using cast_op_type = cast_op_type<_T>;                                            \
        operator CLASS_TYPE *() { return value; }                                                                \
        operator CLASS_TYPE &() { return* value; }                                                               \
                                                                                                                 \
        bool load(handle src, bool) {                                                                            \
            LOG_DEBUG("Convert SWIG object from Python to C++ for: " CLASS_NAME " -> " #CLASS_TYPE);             \
                                                                                                                 \
            PyObject* obj = src.ptr();                                                                           \
            if (obj == Py_None) {                                                                                \
                LOG_DEBUG("Source (Python) object is None, so return nullptr");                                  \
                value = nullptr;                                                                                 \
                return true;                                                                                     \
            }                                                                                                    \
            if (strcmp(obj->ob_type->tp_name, CLASS_NAME) != 0) {                                                \
                LOG_DEBUG("[Python to C++] refuse use " << obj->ob_type->tp_name << " as " CLASS_NAME );         \
                /* not LOG_WARNING here because this happen while using overload functions */                    \
                return false;                                                                                    \
            }                                                                                                    \
                                                                                                                 \
            PyObject* thisAttr;                                                                                  \
            if ( !PyObject_HasAttrString(obj, "this") || !(thisAttr = PyObject_GetAttrString(obj, "this")) ) {   \
                LOG_WARNING("[Python to C++] error on get attribute \"this\" from: " CLASS_NAME );               \
                return false;                                                                                    \
            }                                                                                                    \
                                                                                                                 \
            void* pointer = reinterpret_cast<PySwigObject*>(thisAttr)->ptr;                                      \
            Py_DECREF(thisAttr);                                                                                 \
                                                                                                                 \
            value = reinterpret_cast<CLASS_TYPE*>(pointer);                                                      \
            return true;                                                                                         \
        }                                                                                                        \
                                                                                                                 \
        static inline swig_type_info* getTypeInfo() {                                                            \
            static swig_type_info* typeInfo = SWIG_TypeQuery(#CLASS_TYPE " *");                                  \
            if (!typeInfo)                                                                                       \
                throw std::logic_error( "[C++ to Python] can't find typeInfo for: " CLASS_NAME );                \
            return typeInfo;                                                                                     \
        }                                                                                                        \
                                                                                                                 \
        static handle cast(const CLASS_TYPE* inPtr, return_value_policy, handle) {                               \
            LOG_DEBUG("Convert ptr (existing_object) from C++ to Python: " #CLASS_TYPE "-> " CLASS_NAME);        \
                                                                                                                 \
            return SWIG_InternalNewPointerObj(                                                                   \
                const_cast<void*>(reinterpret_cast<const void*>(inPtr)), getTypeInfo(), 0                        \
            );                                                                                                   \
        }                                                                                                        \
                                                                                                                 \
        static handle cast(const CLASS_TYPE& inObj, return_value_policy, handle) {                               \
            LOG_DEBUG("Convert obj from C++ to Python: " #CLASS_TYPE "-> " CLASS_NAME);                          \
                                                                                                                 \
            CLASS_MODE(CLASS_TYPE, CLASS_NAME)                                                                   \
        }                                                                                                        \
    };                                                                                                           \
}}                                                                                                               \


/**
 * @brief Shortcut macro to add mapping of copy-constructable C++ type <--> Python type via external python module generated with SWIG.
 * 
 * @param scope  C++ namespace
 * @param name   C++ class name
 * 
 * Will add mapping C++ ``scope::name`` to/from Python ``scope.name``.
 * For more advanced control use directly @ref PYBIND11_SWIG_GENERATE_CAST.
 */
#define PYBIND11_SWIG_GENERATE_CAST_FULL(scope, name) \
    PYBIND11_SWIG_GENERATE_CAST(scope::name, #scope"."#name, PYBIND11_SWIG_CONSTRUCTABLE)

/**
 * @brief Shortcut macro to add mapping of non copy-constructable C++ type <--> Python type via external python module generated with SWIG.
 * 
 * @param scope  C++ namespace
 * @param name   C++ class name
 * 
 * Will add mapping C++ ``scope::name`` to/from Python ``scope.name``.
 * For more advanced control use directly @ref PYBIND11_SWIG_GENERATE_CAST.
 */
#define PYBIND11_SWIG_GENERATE_CAST_ONLYPTR(scope, name) \
    PYBIND11_SWIG_GENERATE_CAST(scope::name, #scope"."#name, PYBIND11_SWIG_NON_CONSTRUCTABLE)

/// @}
