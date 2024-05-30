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

#include "force_inline.h"

#include <stdexcept>
#include <typeinfo>

#ifndef __DOCUMENTATION_GENERATOR__
#if __has_include(<boost/core/demangle.hpp>)
	#include <boost/core/demangle.hpp>
	#define MGE_DEMANGLE(a) boost::core::demangle(a)
#else
	#define MGE_DEMANGLE(a) a
#endif
#endif

#ifdef MGE_DEBUG_SINGLETON
#include "LogSystem.h"
#endif

namespace MGE {

/// @addtogroup CoreUtils
/// @{
/// @file

/**
 * @brief Base class for non copyable objects.
 */
class NoCopyableNoMovable {
	protected:
		/// No copy constructor.
		NoCopyableNoMovable(const NoCopyableNoMovable&) = delete;
		
		/// No move constructor.
		NoCopyableNoMovable(const NoCopyableNoMovable&&) = delete;
		
		/// No assignment operator.
		NoCopyableNoMovable& operator=(const NoCopyableNoMovable&) = delete;
		
		/// No move operator.
		NoCopyableNoMovable& operator=(const NoCopyableNoMovable&&) = delete;
		
		/// Default constructor.
		NoCopyableNoMovable() = default;
		
		/// Default destructor.
		virtual ~NoCopyableNoMovable() = default;
};

/**
 * @brief Base class template for trivial (created on first usage) singletons.
 * 
 * @tparam CLASS Type of derived class.
 * 
 * @note   Derived class should: 
 *          - public inherit from TrivialSingleton<...>
 *          - have protected/private constructor and ``friend class TrivialSingleton;`` declaration
 *          - have protected/private destructor
 * 
 * @warning
 *         Deleting pointer returned by ``getPtr()`` will results return invalid not NULL pointer by next call ``getPtr()``.
 */
template <typename CLASS> class TrivialSingleton : NoCopyableNoMovable {
	public:
		/**
		 * @brief Return pointer to singleton object.
		 */
		inline static CLASS* getPtr() { // do not FORCE_INLINE due to size of this function
			static CLASS _singleton_obj_;
			return &_singleton_obj_;
		}
		
	protected:
		/// Singleton constructor - do nothing, protected (only for use in derived class initializer).
		TrivialSingleton() = default;
		
		/// Singleton destructor - do nothing, protected (only for use in derived class initializer).
		virtual ~TrivialSingleton() = default;
};

/**
 * @brief Base class template for singletons (with separated initialization and get functions).
 * 
 * @tparam CLASS Type of derived class.
 * 
 * @note   Derived class should:
 *          - public inherit from Singleton<...>
 *          - have public constructor (or construction member function)
 *          - have public destructor (or destruction member function)
 */
template <typename CLASS> class Singleton : NoCopyableNoMovable {
	public:
		/**
		 * @brief Return pointer to singleton object, return NULL, when singleton object is not initialized.
		 * 
		 * @note  When build with defined macro DEBUG_SINGLETON, then check value of returned pointer, and log warning if it's NULL.
		 *        This is not a error, because we use if(...::getPtr()) to check existence of module (before use or even before create in some cases).
		 *        On non debug builds return NULL without warning!
		 */
		FORCE_INLINE static CLASS* getPtr() {
			#ifdef MGE_DEBUG_SINGLETON
			if (!_singleton_ptr_) LOG_WARNING("Singleton getPtr() return NULL for " << MGE_DEMANGLE(typeid(CLASS).name()) << " (this doesn't have to be a bad thing)");
			#endif
			return _singleton_ptr_;
		}
		
	protected:
		/**
		 * @brief Singleton constructor - check and set singleton pointer (@ref _singleton_ptr_).
		 * 
		 * @remark
		 *          - It is protected => only for use in derived class initializer.
		 *          - It will be automatically call on construction derived class.
		 */
		Singleton() {
			if (!_singleton_ptr_) {
				_singleton_ptr_ = static_cast<CLASS*>(this);
					// 1. `this` is base object of `CLASS`
					// 2. static_cast is more efficient than dynamic_cast
					// 3. we can't use dynamic_cast
			} else {
				throw std::logic_error(std::string("Can't create second instance of \"") + MGE_DEMANGLE(typeid(CLASS).name()) + "\", this is singleton!");
			}
		};
		
		/**
		 * @brief Singleton destructor - set singleton pointer (@ref _singleton_ptr_) to NULL.
		 * 
		 * @remark
		 *          - It will be automatically call after derived class destructor.
		 */
		virtual ~Singleton() {
			_singleton_ptr_ = nullptr;
		}
		
		/// Pointer to singleton object.
		inline static CLASS* _singleton_ptr_ = nullptr;
};
/// @}
}

#undef MGE_DEMANGLE
