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

#pragma   once

#include "force_inline.h"
#include "Range.h"

#include <map>
#include <functional>
#include <stdint.h>

// do NOT use logSystem.h here - ListenerSet can be used in global-constructors (before create default log)

namespace MGE {

/// @addtogroup CoreUtils
/// @{
/// @file

/**
 * @brief Base class for @ref FunctionListenerSet and @ref ClassListenerSet templates. Provide add / remove listener interface.
 * 
 * @tparam ListenerType    Type of listener class/function/...
 * @tparam KeyType         Type of member functions key argument. Must be comparable with @a KeyStorageType by std::less<>, and convertible to @a KeyStorageType (for add operation).
 * @tparam KeyStorageType  Type of multimap key.
 */
template <typename ListenerType, typename KeyType, typename KeyStorageType = KeyType>
struct ListenerSetBase {
	/**
	 * @brief Set of listeners (as map key - pointer).
	 * 
	 * @note  Use std::multimap, not std::unordered_multimap, because we want call listener in order of keys.
	 */
	std::multimap<KeyStorageType, ListenerType, std::less<>> listeners;
	
	/**
	 * @brief Add @a listener to listener set.
	 * 
	 * @param listener Pointer to listener object / function.
	 * @param key      Key in multimap, used to determine the order of listeners call.
	 * 
	 * @return True when listener is add to set, false when key-listener pair was already in set.
	 */
	bool addListener(ListenerType listener, KeyType key) {
		auto range = MGE::Range(listeners, key);
		for (auto iter=range.begin(); iter!=range.end(); ++iter) {
			if (iter->second == listener)
				return false;
		}
		listeners.insert(std::pair<KeyType, ListenerType>(key, listener));
		return true;
	}
	
	/**
	 * @brief Remove @a listener from listener set.
	 * 
	 * @param listener Pointer to listener object / function.
	 */
	void remListener(ListenerType listener) {
		for (auto iter=listeners.begin(); iter!=listeners.end(); ++iter) { // don't use range-based for loop here, because we want use iterator ... and don't use in other functions for homogeneity ...
			if (iter->second == listener) {
				listeners.erase(iter);
				return;
			}
		}
	}
	
	/**
	 * @brief Remove @a listener from listener set.
	 * 
	 * @param listener Pointer to listener object / function.
	 * @param key      Key in multimap (remove only for this key value).
	 */
	void remListener(ListenerType listener, KeyType key) {
		auto range = MGE::Range(listeners, key);
		for (auto iter=range.begin(); iter!=range.end(); ++iter) {
			if (iter->second == listener) {
				listeners.erase(iter);
				return;
			}
		}
	}
};

/**
 * @brief Registration static function listeners and call it.
 * 
 * \par Example
	\code{.cpp}
		typedef bool (*CmdDelegate)(int arg);
		bool xyz(int x);
		
		FunctionListenerSet<CmdDelegate> myListener;
		myListener.addListener(xyz);
		myListener.callAll(213);
	\endcode
 * 
 * @tparam ListenerType    Type of listener function pointer (should return value auto convertible to bool).
 * @tparam KeyType         Type of member functions key argument. Must be comparable with @a KeyStorageType by std::less<>, and convertible to @a KeyStorageType (for add operation).
 * @tparam KeyStorageType  Type of multimap key.
 *
 * @note Can't be used with ``ListenerType==std::function`` (e.g. for std::bind), because lack of comparisons between std::function.
 *       In that case, use @ref ClassListenerSet with @ref FunctorListenerClassBase.
 */
template <typename ListenerType, typename KeyType = unsigned char, typename KeyStorageType = KeyType>
struct FunctionListenerSet : public ListenerSetBase<ListenerType, KeyType, KeyStorageType> {
	/**
	 * @brief Call register functions with @a args for successive elements from listener set until return true.
	 * 
	 * @param args Arguments for register functions.
	 * 
	 * @return True when some listener returned true. False when all listeners returned false.
	 */
	template <typename ...Args> bool callFirst(Args ... args) {
		auto iter = this->listeners.begin();
		while (iter != this->listeners.end()) {
			if ( (iter++)->second(args...) )
				return true;
		}
		return false;
	}
	
	// in callFirst() and callAll() member functions use while loop and iter++ (instead of for and ++iter),
	// because called listener function can remove itself from listeners map,
	// so we must obtain next iterator before call listener function
	
	/**
	 * @brief Call register functions with @a args for all elements from listener set.
	 * 
	 * @param args Arguments for register functions.
	 * 
	 * @return Number of listeners returned true.
	 */
	template <typename ...Args> int callAll(Args ... args) {
		int ret = 0;
		auto iter = this->listeners.begin();
		while (iter != this->listeners.end()) {
			ret += (iter++)->second(args...);
		}
		return ret;
	}
	
	/**
	 * @brief Call register functions with @a args for successive elements from listener set with key == @a key until return true.
	 * 
	 * @param key  Key value to selection subset from listener set.
	 * @param args Arguments for register functions.
	 * 
	 * @return True when some listener returned true. False when all listeners returned false.
	 * 
	 * @attention  Called function must not modify listeners map (don't call @ref addListener, @ref remListener or manipulate @ref listeners)
	 *             If this is needed use @ref callFirst.
	 */
	template <typename ...Args> bool callFirstWithKey(KeyType key, Args&& ... args) {
		auto range = MGE::Range(this->listeners, key);
		for (auto iter=range.begin(); iter!=range.end(); ++iter) {
			if ( (iter->second)(args...) )
				return true;
		}
		return false;
	}
	
	/**
	 * @brief Call register functions with @a args for all elements from listener set with key == @a key.
	 * 
	 * @param key  Key value to selection subset from listener set
	 * @param args Arguments for register functions
	 * 
	 * @return Number of listeners returned true.
	 * 
	 * @attention  Called function must not modify listeners map (don't call @ref addListener, @ref remListener or manipulate @ref listeners).
	 *             If this is needed use @ref callAll.
	 */
	template <typename ...Args> int callAllWithKey(KeyType key, Args&& ... args) {
		int ret = 0;
		auto range = MGE::Range(this->listeners, key);
		for (auto iter=range.begin(); iter!=range.end(); ++iter) {
			ret += (iter->second)(args...);
		}
		return ret;
	}
};

/**
 * @brief Registration listeners and call arbitrary function on listeners objects.
 * 
 * \par Example
	\code{.cpp}
		struct ListenerClass {
			bool callA(int x);
			bool callB();
		};
		
		ListenerSet<ListenerClass> myListener;
		auto l = new ListenerClass();
		myListener.addListener(l);
		myListener.callAll(&ListenerClass::callA, 213);
		myListener.callFirst(&ListenerClass::callB);
	\endcode
 * 
 * @tparam ListenerType         Type of listener class.
 * @tparam ListenerStorageType  Set to ``ListenerType*`` for store pointers to listeners or to ``ListenerType`` for store listeners objects.
 * @tparam KeyType              Type of member functions key argument. Must be comparable with @a KeyStorageType by std::less<>, and convertible to @a KeyStorageType (for add operation).
 * @tparam KeyStorageType       Type of multimap key.
 */
template <typename ListenerType, typename ListenerStorageType = ListenerType*, typename KeyType = unsigned char, typename KeyStorageType = KeyType>
struct ClassListenerSet : public ListenerSetBase<ListenerStorageType, KeyType, KeyStorageType> {
	/**
	 * @brief Call @a memberFunction with @a args for successive elements from listener set until return true.
	 * 
	 * @param memberFunction   Pointer to member function in listener object to call (should return value auto convertible to bool).
	 * @param args             Arguments for @a memberFunction.
	 * 
	 * @return True when some listener returned true. False when all listeners returned false.
	 */
	template <typename FunctionType, typename ...Args> bool callFirst(FunctionType memberFunction, Args&& ... args) {
		auto iter = this->listeners.begin();
		while (iter != this->listeners.end()) {
			if ( call((iter++)->second, memberFunction, args...) )
				return true;
		}
		return false;
	}
	
	// in callFirst() and callAll() member functions use while loop and iter++ (instead of for and ++iter),
	// because called listener function can remove itself from listeners map,
	// so we must obtain next iterator before call listener function
	
	/**
	 * @brief Call @a memberFunction with @a args for all elements from listener set.
	 * 
	 * @param memberFunction   Pointer to member function in listener object to call (should return value auto convertible to bool).
	 * @param args             Arguments for @a memberFunction.
	 * 
	 * @return Number of listeners returned true.
	 */
	template <typename FunctionType, typename ...Args> int callAll(FunctionType memberFunction, Args&& ... args) {
		int ret = 0;
		auto iter = this->listeners.begin();
		while (iter != this->listeners.end()) {
			ret += call((iter++)->second, memberFunction, args...);
		}
		return ret;
	}
	
	/**
	 * @brief Call @a memberFunction with @a args for successive elements from listener set with key == @a key until return true.
	 * 
	 * @param key              Key value to selection subset from listener set.
	 * @param memberFunction   Pointer to member function in listener object to call (should return value auto convertible to bool).
	 * @param args             Arguments for @a memberFunction.
	 * 
	 * @return True when some listener returned true. False when all listeners returned false.
	 * 
	 * @attention  Called function must not modify listeners map (don't call @ref addListener, @ref remListener or manipulate @ref listeners)
	 *             If this is needed use @ref callFirst.
	 */
	template <typename FunctionType, typename ...Args> bool callFirstWithKey(KeyType key, FunctionType memberFunction, Args&& ... args) {
		auto range = MGE::Range(this->listeners, key);
		for (auto iter=range.begin(); iter!=range.end(); ++iter) {
			if ( call(iter->second, memberFunction, args...) )
				return true;
		}
		return false;
	}
	
	/**
	 * @brief Call @a memberFunction with @a args for all elements from listener set with key == @a key.
	 * 
	 * @param key              Key value to selection subset from listener set.
	 * @param memberFunction   Pointer to member function in listener object to call (should return value auto convertible to bool).
	 * @param args             Arguments for @a memberFunction.
	 * 
	 * @return Number of listeners returned true.
	 * 
	 * @attention  Called function must not modify listeners map (don't call @ref addListener, @ref remListener or manipulate @ref listeners).
	 *             If this is needed use @ref callAll.
	 */
	template <typename FunctionType, typename ...Args> int callAllWithKey(KeyType key, FunctionType memberFunction, Args&& ... args) {
		int ret = 0;
		auto range = MGE::Range(this->listeners, key);
		for (auto iter=range.begin(); iter!=range.end(); ++iter) {
			ret += call(iter->second, memberFunction, args...);
		}
		return ret;
	}
	
private:
	/// force inline function template for distinguishing whether ListenerType is pointer or object @{
	template <typename FunctionType, typename ...Args> static FORCE_INLINE bool call(ListenerType* ptr, FunctionType memberFunction, Args&& ... args) {
		return (ptr->*memberFunction)(args...);
	}
	template <typename FunctionType, typename ...Args> static FORCE_INLINE bool call(ListenerType& obj, FunctionType memberFunction, Args&& ... args) {
		return (obj.*memberFunction)(args...);
	}
	/// @}
};

/// Shortcut for @ref ClassListenerSet stored pointers to listeners.
template<typename ListenerType, typename KeyType = unsigned char, typename KeyStorageType = KeyType> using ClassPtrListenerSet = ClassListenerSet<ListenerType, ListenerType*, KeyType, KeyStorageType>;

/// Shortcut for @ref ClassListenerSet stored listeners objects.
template<typename ListenerType, typename KeyType = unsigned char, typename KeyStorageType = KeyType> using ClassObjListenerSet = ClassListenerSet<ListenerType, ListenerType,  KeyType, KeyStorageType>;

/**
 * @brief Base class for register std::function in ClassListenerSet
 * 
 * \par Example
	\code{.cpp}
		bool xyz(int x);
		bool abc(int x, int y);
		typedef FunctorListenerClassBase<bool, int> ListenerClassFunctor;
		
		ClassListenerSet<ListenerClassFunctor, ListenerClassFunctor> myListener;
		myListener.addListener(ListenerClassFunctor(xyz, (uintptr_t)&xyz));
		myListener.addListener(ListenerClassFunctor(std::bind(abc, std::placeholders::_1, 1000), 4321));
		myListener.callAll(&ListenerClassFunctor::call, 678);
	\endcode
 *
 * @tparam ReturnType   Type of returned value from listener functions. Should be auto convertible to bool.
 * @tparam Args         Arguments of listener function.
 */
template <typename ReturnType, typename ...Args> struct FunctorListenerClassBase {
	/// Function to call.
	std::function<ReturnType(Args...)> function;
	
	/// Unique function id.
	uintptr_t id;
	
	/// Constructor - set @ref function to @a _function and @ref id to @a _id.
	FunctorListenerClassBase(const std::function<ReturnType(Args...)>& _function, uintptr_t _id):
		function(_function), id(_id)
	{}
	
	/// %Compare operator need to ListenerSetBase
	bool operator ==(const FunctorListenerClassBase& x) const {
		return id == x.id;
	}
	
	/// call method
	ReturnType call(Args ... args) {
		return function(args...);
	}
};
/// @}
}
