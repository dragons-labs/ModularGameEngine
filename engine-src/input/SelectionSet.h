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

#include "config.h"

#include "input/Selection.h"
#include "physics/Raycast.h"

namespace MGE {

/// @addtogroup Input
/// @{
/// @file

/**
 * @brief base template class for "selected object set"
 */
template <typename ObjectType, uint32_t SearchMask, typename SelectionClass> struct SelectionSetTemplate : public MGE::Selection::SelectionSetBase {
	/// set of selected objects
	std::set<ObjectType>  selection;
	
	/// @copydoc MGE::Selection::SelectionSetBas::getSearchMask
	virtual int getSearchMask() override {
		return SearchMask;
	}
	
	/**
	 * @brief select selectable object
	 * 
	 * @param     obj   selectable object to select
	 * @param[in] mode  selection mode passed to SelectionClass::markSelection() and SelectionClass::canSelect()
	 * @param[in] force ignore result of canSelect() / do not call canSelect()
	 */
	inline void select(ObjectType obj, int mode = 0, bool force = false) {
		if (force || SelectionClass::canSelect(obj, mode)) {
			selection.insert(obj);
			SelectionClass::markSelection(obj, true, mode);
			if (selectionChanged == ONE_SHOT)
				SelectionClass::onSelectionChanged();
		}
	}
	using SelectionSetBase::select;
	
	/**
	 * @brief unselect selectable object
	 * 
	 * @param     obj   selectable object to unselect
	 * @param[in] mode  selection mode passed to SelectionClass::markSelection()
	 */
	inline int unselect(ObjectType obj, int mode = 0) {
		SelectionClass::markSelection(obj, false, mode);
		int retVal = selection.erase(obj);
		if (selectionChanged == ONE_SHOT && retVal > 0)
			SelectionClass::onSelectionChanged();
		return retVal;
	}
	
	/**
	 * @brief unselect selectable object by iterator
	 * 
	 * @param     iter  iterator to unselect
	 * @param[in] mode  selection mode passed to SelectionClass::markSelection()
	 */
	inline int unselect(typename std::set<ObjectType>::iterator iter, int mode = 0) {
		SelectionClass::markSelection(*iter, false, mode);
		selection.erase(iter);
		if (selectionChanged == ONE_SHOT)
			SelectionClass::onSelectionChanged();
		return 1;
	}
	
	/**
	 * @brief unselect all selectable object
	 * 
	 * @param[in] mode  selection mode passed to SelectionClass::markSelection()
	 */
	inline int unselectAll(int mode = 0) {
		int retVal = selection.size();
		auto iter=selection.begin();
		while(iter != selection.end()) {
			unselect(iter++, mode);
		}
		if (selectionChanged == ONE_SHOT && retVal > 0)
			SelectionClass::onSelectionChanged();
		return retVal;
	}
	
	/**
	 * @brief check if object is selected
	 * 
	 * @param[in] obj   selectable object to check
	 * 
	 * @return
	 *   @li true  - when object is selected
	 *   @li false - when object is not selected
	 */
	inline bool isSelected(ObjectType obj) {
		return selection.find(obj) != selection.end();
	}
	
	/**
	 * @brief switch selection state of object
	 * 
	 * @param     obj   selectable object to select
	 * @param[in] mode  selection mode passed to SelectionClass::markSelection() and SelectionClass::canSelect()
	 */
	inline void switchSelection(ObjectType obj, int mode = 0) {
		auto iter = selection.find(obj);
		if (iter != selection.end()) {
			unselect(iter, mode);
		} else {
			select(obj, mode);
		}
	}
	
protected:
	/// constructor
	SelectionSetTemplate() {
		selectionChanged = ONE_SHOT;
	}
	
	/**
	 * @brief initialization of selection, for internal use in @ref SelectionSetBase::select
	 * 
	 * @param _selectSwitchMode operation mode to perform on objects (select, switch, etc), see @ref SelectionSwitchModes
	 * @param _selectionMode    mode of selection operation, see @ref SelectionModes
	 */
	inline void initSelect(int _selectSwitchMode, int _selectionMode) {
		selectionMode    = _selectionMode;
		
		if (_selectSwitchMode == MGE::Selection::RESET_SELECTION) {
			unselectAll(selectionMode);
			selectionChanged = CHANGED;
			selectSwitchMode = MGE::Selection::ADD_TO_SELECTION;
		} else {
			selectionChanged = NOT_CHANGED;;
			selectSwitchMode = _selectSwitchMode;
		}
	}
	
	/**
	 * @brief finish selection, for internal use in @ref SelectionSetBase::select
	 */
	inline bool finishSelect() {
		bool retVal = (selectionChanged == CHANGED);
		selectionChanged = ONE_SHOT;
		
		if (retVal) {
			SelectionClass::onSelectionChanged();
		}
		
		return retVal;
	}
	
	/**
	 * @brief do selection / unselection of single object, for internal use in @ref SelectionSetBase::select
	 * 
	 * @param obj object to operate on it
	 */
	inline void doSelect(ObjectType obj) {
		switch(selectSwitchMode) {
			case MGE::Selection::SWITCH_SELECTION: 
				switchSelection(obj, selectionMode);
				break;
			case MGE::Selection::REMOVE_FROM_SELECTION: 
				unselect(obj, selectionMode);
				break;
			default:
				select(obj, selectionMode);
		}
		selectionChanged = CHANGED;
	}
	
	/// flag for determinate return value from @ref SelectionSetBase::select and check if call onSelectionChanged is need
	enum { ONE_SHOT, NOT_CHANGED, CHANGED } selectionChanged;
	
	/// mode of selection operation, see @ref SelectionModes
	int     selectionMode;
	
	/// operation mode to perform on objects (select, switch, etc), see @ref SelectionSwitchModes
	int     selectSwitchMode;
	
	/**
	 * @brief check selection possiblity
	 * 
	 * @param      obj    object to operate
	 * @param[in]  mode   selection mode (cause of selection - e.g. selecting action target)
	 * 
	 * @return true when can select, false otherwise
	 */
	static bool canSelect(ObjectType obj, int mode) {
		return true;
	}
	
	/**
	 * @brief switch selection status / visualization
	 * 
	 * @param      obj         object to operate
	 * @param[in]  selectionState   true == selected, false == unselected
	 * @param[in]  mode             selection mode (cause of selection - e.g. selecting action target)
	 */
	static void markSelection(ObjectType obj, bool selection, int mode) {
		LOG_DEBUG("(fake) select: " << obj);
	}
	
	/**
	 * @brief call when @ref set is changed
	 *        - after finish modify @ref set in @ref finishSelect
	 *        - after one shoot modify @ref set in @ref unselectAll and object args variants of @ref unselect and @ref select
	 */
	static void onSelectionChanged() {}
};

/**
 * @brief "selected object set" type
 * 
 * @tparam SetElementType type of std::set elements
 * @tparam SearchMask     value to return by @ref MGE::Selection::SelectionSetBas::getSearchMask
 * @tparam SelectionClass class used to selecting object, must have three static member functions:
 *       (see @ref SelectionSetTemplate::canSelect, @ref SelectionSetTemplate::markSelection and @ref SelectionSetTemplate::onSelectionChanged):
 *       - \code{.cpp} static bool canSelect(ObjectType obj, int mode);           \endcode
 *       - \code{.cpp} static void markSelection(ObjectType obj, bool selection, int mode); \endcode
 *       - \code{.cpp} static void onSelectionChanged(); \endcode
 * @tparam AnyValueType when:
 *      - @a void       direct get GameObject from raycast results instand of recive Ogre::Any (SetElementType should be MGE::BaseActor*)
 *      - otherwise     type of object in Ogre::Any recive from getUserObjectBindings().getUserAny(@a filterID), must be castable to @a SetElementType via MGE::Any::Cast
 * @tparam UseFilterID  when:
 *      - @a true       use @a filterID to get any from bindings
 *      - @a false      get default any from bindings
 * 
 * @note specialization derived from @ref SelectionSetTemplate
 */
template <typename SetElementType, uint32_t SearchMask, typename SelectionClass, typename AnyValueType = void, bool UseFilterID = false> class SelectionSet;

#ifndef __DOCUMENTATION_GENERATOR__
// SelectionSet template specialisation for:
//   1. get ogreObject (Ogre::MovableObject) from RayCast::Results
//   2. and get SetElementType object from its MGE::Any (using filterID)
template <typename SetElementType, uint32_t SearchMask, typename SelectionClass, typename AnyValueType>
class SelectionSet<SetElementType, SearchMask, SelectionClass, AnyValueType, true> :
	public MGE::SelectionSetTemplate<SetElementType, SearchMask, SelectionClass>
{
public:
	SelectionSet(const std::string& _filterID) : filterID(_filterID) { }
	
	using MGE::SelectionSetTemplate<SetElementType, SearchMask, SelectionClass>::select;
	
	bool select( MGE::RayCast::ResultsPtr searchResults, int _selectSwitchMode, int _selectionMode ) override {
		LOG_DEBUG("SelectionSet::select() with selectSwitchMode=" << _selectSwitchMode);
		
		this->initSelect(_selectSwitchMode, _selectionMode);
		for (auto& iter : searchResults->hitObjects) {
			if (iter.ogreObject) {
				const MGE::Any& tmpAny = MGE::Any::getFromBindings(iter.ogreObject->getParentSceneNode(), filterID);
				if (!tmpAny.isEmpty()) {
					this->doSelect(MGE::Any::Cast<SetElementType, AnyValueType>::getValue(tmpAny));
				}
			}
		}
		return this->finishSelect();
	}
	
private:
	Ogre::String   filterID;
};

// SelectionSet template specialisation for:
//   1. get ogreObject (Ogre::MovableObject) from RayCast::Results
//   2. and get SetElementType object from its MGE::Any (without filter)
template <typename SetElementType, uint32_t SearchMask, typename SelectionClass, typename AnyValueType>
class SelectionSet<SetElementType, SearchMask, SelectionClass, AnyValueType, false> :
	public MGE::SelectionSetTemplate<SetElementType, SearchMask, SelectionClass>
{
public:
	SelectionSet(const std::string& _ignored = MGE::EMPTY_STRING) { }
	
	using MGE::SelectionSetTemplate<SetElementType, SearchMask, SelectionClass>::select;
	
	bool select( MGE::RayCast::ResultsPtr searchResults, int _selectSwitchMode, int _selectionMode ) override {
		LOG_DEBUG("SelectionSet::select() with selectSwitchMode=" << _selectSwitchMode);
		
		this->initSelect(_selectSwitchMode, _selectionMode);
		for (auto& iter : searchResults->hitObjects) {
			if (iter.ogreObject) {
				const MGE::Any& tmpAny = MGE::Any::getFromBindings(iter.ogreObject->getParentSceneNode());
				if (!tmpAny.isEmpty()) {
					this->doSelect(MGE::Any::Cast<SetElementType, AnyValueType>::getValue(tmpAny));
				}
			}
		}
		return this->finishSelect();
	}
};

// SelectionSet template specialisation for:
//   1. get gameObject (MGE::BaseActor) from RayCast::Results
template <typename SelectionClass>
class SelectionSet<MGE::BaseActor*, MGE::QueryFlags::GAME_OBJECT, SelectionClass, void> :
	public MGE::SelectionSetTemplate<MGE::BaseActor*, MGE::QueryFlags::GAME_OBJECT, SelectionClass>
{
public:
	SelectionSet(const std::string& _ignored = MGE::EMPTY_STRING) { }
	
	using MGE::SelectionSetTemplate<MGE::BaseActor*, MGE::QueryFlags::GAME_OBJECT, SelectionClass>::select;
	
	bool select( MGE::RayCast::ResultsPtr searchResults, int _selectSwitchMode, int _selectionMode ) override {
		LOG_DEBUG("SelectionSet::select() with selectSwitchMode=" << _selectSwitchMode);
		
		this->initSelect(_selectSwitchMode, _selectionMode);
		for (auto& iter : searchResults->hitObjects) {
			if (iter.gameObject) {
				this->doSelect(iter.gameObject);
			}
		}
		return this->finishSelect();
	}
};
#endif

/// @}

}
