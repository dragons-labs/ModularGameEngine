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

#include "BaseClasses.h"
#include "StringTypedefs.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include <OgreTimer.h>

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief implement timers functionality
 * 
 * @note we support store / restore only timers triggering python script
 *       (don't store / restore timers calling C++ function or member function)
 */
class TimerSet : MGE::NoCopyableNoMovable, MGE::SaveableToXMLInterface, MGE::UnloadableInterface {
public:
	/**
	 * @brief type of function to implement timer command
	 * 
	 * This is 3 argument function or class member function. As arguments receives:
	 *  @li timer name
	 *  @li timer execution delay (int)
	 *  @li pointer void* submitted during registration function
	 * 
	 * return value is used to determinate repeat of timer (if repeat is enabled via addTimer): if function return false timer don't be repeat
	 */
	typedef std::function<bool(const std::string&, int, void*)> TimerCallbackFunction;
	
	/**
	 * @brief function to register timer command
	 * 
	 * @param[in] period   timer tick period in ms
	 * @param[in] callback callback function (see @ref TimerCallbackFunction)
	 * @param[in] name     optional timer name (passed to callback, used for remove timer, ...)
	 * @param[in] repeat   repeat timer (if false timer will be execute only one, if true timer wilbe execute every @a period ms)
	 * @param[in] catchup  if true the timer handler can be called multiple times on one tick ...
	 * @param     args     void* pointer passed as last arument to callback function
	 */
	void addTimerCpp(
		unsigned int period,
		const TimerCallbackFunction& callback,
		const std::string_view& name = MGE::EMPTY_STRING,
		bool  repeat = true,
		bool  catchup = false,
		void* args = NULL
	);
	
	/**
	 * @brief function to register timer command
	 * 
	 * @param[in] period     timer tick period in ms
	 * @param[in] scriptName name of callback script
	 * @param[in] name       optional timer name (passed to callback, used for remove timer, ...)
	 * @param[in] repeat     repeat timer (if false timer will be execute only one, if true timer wilbe execute every @a period ms)
	 * @param[in] catchup    if true the timer handler can be called multiple times on one tick ...
	 * @param[in] args       string passed as last arument to callback function
	 */
	void addTimer(
		unsigned int period,
		const std::string_view& scriptName,
		const std::string_view& name = MGE::EMPTY_STRING,
		bool  repeat = true,
		bool  catchup = false,
		const std::string_view& args = MGE::EMPTY_STRING
	);
	
	/**
	 * @brief remove all timers whith specified @a name
	 */
	void stopTimer(const std::string_view& name);
	
	/**
	 * @brief get time counter value
	 */
	unsigned int getCounter() const {
		return counter;
	}
	
	/**
	 * @brief get time counter value
	 * 
	 * @param offset offset to be subtracted from current counter value
	 * @param format printf type format for returned string (argument order, hours, minuts, seconds)
	 */
	const std::string getCounterStr(int offset = 0, MGE::null_end_string format = "%02d:%02d:%02d");
	
	/**
	 * @brief pause all timers in this set
	 */
	void pause();
	
	/**
	 * @brief unpause all timers in this set
	 */
	void unpause();
	
	/**
	 * @brief return true if timer is paused
	 */
	inline bool timerIsPaused(void) const {
		return isPaused;
	}
	
	/**
	 * @brief set time scale (time speed for timers in this set)
	 */
	void setTimeScale(float scale);
	
	/**
	 * @brief get time scale (time speed for timers in this set)
	 */
	float getTimeScale() const {
		return timeScale;
	}
	
	/// @copydoc MGE::SaveableToXMLInterface::getXMLTagName
	virtual const std::string_view getXMLTagName() const override;
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override;
	
protected:
	friend class TimeSystem;
	
	/**
	 * @brief constructor - initialize internal variables, and (if need screipt interface)
	 * 
	 * @param[in] name     name of timer used as script object name
	 *                     when empty don't try initialize script interface and don't expose this timer to script system
	 */
	TimerSet(const std::string_view& name = MGE::EMPTY_STRING);
	
	/// destructor
	~TimerSet();
	
	/**
	 * @brief Method to execute in FrameListener function, to update / execute timers.
	 */
	void update();
	
	/// single timer struct
	struct TimerInstance {
		/// name of timer
		std::string            name;
		
		/// repeated period of timer (when equal zero timer will not be repeated)
		unsigned int           period;
		
		/// when true  repeated with constant value of @a period,
		/// when false repeated with {period} + {time of callback execution}
		bool                   catchup;
		
		/// name of script callback
		std::string            scriptName;
		
		/// std::function callback funtion
		TimerCallbackFunction  callbackFun;
		
		/// (optional) argument for script callback
		std::string            scriptArg;
		
		/// (optional) argument for std::function and static function callback
		void*                  functionArg;
		
		/// constructor with script callback
		TimerInstance(const std::string_view& _name, unsigned int _period, bool _catchup, const std::string_view& _scriptName, const std::string_view& _scriptArg);
		
		/// constructor with c++ function callback
		TimerInstance(const std::string_view& _name, unsigned int _period, bool _catchup, const TimerCallbackFunction& _callbackFun, void* _functionArg);
		
		/// constructor from xml save
		TimerInstance(const pugi::xml_node& xmlNode);
		
		/// store to xml save
		void storeToXML(pugi::xml_node& xmlNode);
		
		private:
		template<class Archive> inline void store_restore(Archive& ar);
	};
	
	/// {time to execution} to {timer struct} map of all timers
	std::multimap<unsigned int, TimerInstance*> timers;
	
	/// name of timers set (used for store / restore)
	std::string             setName;
	
	/// ogre timer object
	mutable Ogre::Timer     ogreTimer;
	
	/// time speed factor for tis time setName
	/// {time length for timers} = timeScale * {real time length}
	float                   timeScale;
	/// reversed value of timeScale (== 1.0/timeScale)
	float                   reverseTimeScale;
	
	/// paused status of this set
	bool                    isPaused;
	
	/// timer set milliseconds (respecting timeScale, it changes and pauses) from timer set init
	unsigned int            counter;
	
	
	/// milliseconds value of ogreTimer when init pause state
	unsigned int            pauseTime;
	
	/// value of ogreTimer on last timer update
	unsigned int            lastUpdate;
	
	/// run single timer
	int runTimer(TimerInstance* timer, int behind);
	
	/// print to log info about all timers
	void printTimers();
};


/**
 * @brief implement game time control functionality
 */
class TimeSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::SaveableToXML<TimeSystem>,
	public MGE::Singleton<TimeSystem>
{
public:
	/**
	 * @brief set game speed
	 */
	void setSpeed(float s);
	
	/**
	 * @brief get scaled (by actual speed) time delta value (numeric 0.0f when paused)
	 * 
	 * @param[in] timeDelta time duration to scale
	 */
	inline float getScaledTime(float timeDelta) {
		if (isPaused)
			return 0.0f;
		else
			return gameTimer->getTimeScale() * timeDelta;
	}
	
	/**
	 * @brief get game speed
	 * 
	 * @param[in] actual if true (deafault) return 0 when game is paused
	 */
	inline float getSpeed(bool actual = true) {
		if (isPaused && actual)
			return 0.0f;
		else
			return gameTimer->getTimeScale();
	}
	
	/**
	 * @brief pause gamne
	 * 
	 * @param[in] key             key to protect unpase by other module, 0 = no key
	 */
	void pause(int key = 0);
	
	/**
	 * @brief unpause gamne
	 * 
	 * @param[in] key  key to protect unpase by other module, 0 = no key
	 */
	void unpause(int key = 0);
	
	/**
	 * @brief if @a p is true pause game if false unpause game
	 * 
	 * @param[in] p    operation mode (pause/unpause)
	 */
	inline void switchPause(bool p) {
		if (p)
			pause();
		else
			unpause();
	}
	
	/**
	 * @brief pause/unpause game - switch current state
	 */
	inline void switchPause(void) {
		switchPause(!isPaused);
	}
	
	/**
	 * @brief return true if game is paused
	 */
	inline bool gameIsPaused(void) const {
		return isPaused;
	}
	
	/**
	 * @brief return gloabl milliseconds counter (from init of TimeSystem, not store/restore)
	 *        can be used for updete control
	 */
	inline unsigned long getMilliseconds() const {
		return ogreTimer.getMilliseconds();
	}
	
	/**
	 * @brief   update realtime and gametime timres
	 * 
	 * @copydoc MGE::MainLoopListener::update
	 */
	bool update(float gameTimeStep, float realTimeStep) override;
	
	
	/// game timer (don't work on active pause)
	MGE::TimerSet* gameTimer;
	
	/// realtime timer (work on active pause, don't chane speed when change game speed)
	MGE::TimerSet* realtimeTimer;
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "TimeSystem";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override;
	
	/// constructor - create gameTimer and realtimeTimer
	TimeSystem();
	
protected:
	/// destructor - delete gameTimer and realtimeTimer
	~TimeSystem();
	
	/// game is paused
	bool isPaused;
	
	/// key used to pause game (requied this same key for unpause)
	int  pauseKey;
	
	/// global Ogre Timer
	mutable Ogre::Timer  ogreTimer;
};

/// @}

}
