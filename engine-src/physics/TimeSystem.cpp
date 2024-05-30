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

#include "physics/TimeSystem.h"

#include "LogSystem.h"
#include "ScriptsSystem.h"
#include "ConfigParser.h"
#include "StoreRestoreSystem.h"
#include "XmlUtils.h"

#include "Engine.h"
#include "data/utils/OgreSceneObjectInfo.h"
#include "physics/GameSpeedMessages.h"

#if defined MGE_DEBUG_TIMERS
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

#include <boost/format.hpp>

/*--------------------- TimerSet::TimerInstance ---------------------*/

MGE::TimerSet::TimerInstance::TimerInstance(const std::string_view& _name, unsigned int _period, bool _catchup, const std::string_view& _scriptName, const std::string_view& _scriptArg) :
	name        (_name),
	period      (_period),
	catchup     (_catchup),
	scriptName  (_scriptName),
	callbackFun (NULL),
	scriptArg   (_scriptArg),
	functionArg (NULL)
{}

MGE::TimerSet::TimerInstance::TimerInstance(const std::string_view& _name, unsigned int _period, bool _catchup, const TimerCallbackFunction& _callbackFun, void* _functionArg) :
	name        (_name),
	period      (_period),
	catchup     (_catchup),
	callbackFun (_callbackFun),
	functionArg (_functionArg)
{}

void MGE::TimerSet::TimerInstance::storeToXML(pugi::xml_node& xmlNode) {
	xmlNode.append_child("name") << name;
	xmlNode.append_child("period") << period;
	xmlNode.append_child("catchup") << catchup;
	xmlNode.append_child("scriptName") << scriptName;
	xmlNode.append_child("scriptArg") << scriptArg;
}

MGE::TimerSet::TimerInstance::TimerInstance(const pugi::xml_node& xmlNode) :
	name       (xmlNode.child("name").text().as_string()),
	period     (xmlNode.child("period").text().as_uint()),
	catchup    (xmlNode.child("catchup").text().as_bool()),
	scriptName (xmlNode.child("scriptName").text().as_string()),
	scriptArg  (xmlNode.child("scriptArg").text().as_string())
{}

/*--------------------- TimerSet : manage (add, remove) and run timers ---------------------*/

void MGE::TimerSet::addTimerCpp(unsigned int period, const TimerCallbackFunction& callback, const std::string_view& name, bool repeat, bool catchup, void* args) {
	LOG_DEBUG("add cpp timer with period=" << period);
	
	timers.insert( std::make_pair(
		static_cast<unsigned int>((isPaused ? pauseTime : ogreTimer.getMilliseconds()) + period * reverseTimeScale),
		new TimerInstance(name, repeat ? period : 0, catchup, callback, args))
	);
}

void MGE::TimerSet::addTimer(unsigned int period, const std::string_view& scriptName, const std::string_view& name, bool repeat, bool catchup, const std::string_view& args) {
	LOG_DEBUG("add script timer: " << scriptName << " with period=" << period);
	
	timers.insert( std::make_pair(
		static_cast<unsigned int>((isPaused ? pauseTime : ogreTimer.getMilliseconds()) + period * reverseTimeScale),
		new TimerInstance(name, repeat ? period : 0, catchup, scriptName, args))
	);
}

int MGE::TimerSet::runTimer(TimerInstance* timer, int behind) {
	DEBUG2_LOG("run timer: " << timer->name << " / " << timer->scriptName);
	
	bool callbackRet = false;
	while( behind >= 0 ) {
		if (!timer->scriptName.empty()) {
			callbackRet = MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>(
				timer->scriptName.c_str(), false,
				timer->name, behind, timer->scriptArg
			);
		} else {
			callbackRet = timer->callbackFun(timer->name, behind, timer->functionArg);
		}
		
		if( !callbackRet || timer->period == 0)
			// do not repeat this timer
			return 0; 
		
		if( !timer->catchup )
			// repeat this timer in period time
			return timer->period * reverseTimeScale;
		
		behind -= timer->period * reverseTimeScale; 
	} 
	
	// repeat this timer in (period time - delay)
	return -behind; 
}

void MGE::TimerSet::update() {
		DEBUG2_LOG("update " << setName << " " << timers.size() << " " << isPaused);
		
		if (isPaused)
			return;
		
		unsigned int now = ogreTimer.getMilliseconds();
		
		for (auto i = timers.begin(); i != timers.end(); i = timers.begin()) {
			TimerInstance* timer = i->second;
			int timeout = now - i->first;
			if (timeout < 0) {
				// elements in timers (std::multimap<unsigned int, TimerInstance>) is sorted by time,
				// so if we get element with time in future, we don't need parse more elements
				break;
			}
			
			timers.erase(i);
			timeout = runTimer(timer, timeout);
			if (timeout > 0) {
				// (re)inset with time in future
				timers.insert(std::make_pair( now + timeout, timer ));
			} else {
				// or delete
				delete timer;
			}
		}
		
		counter += (now - lastUpdate) * timeScale;
		lastUpdate = now;
	}

void MGE::TimerSet::stopTimer(const std::string_view& name) { 
	auto i = timers.begin();  // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
	while ( i != timers.end() ) {
		if( i->second->name == name ) { 
			delete i->second;
			timers.erase(i++); 
		} else {
			++i;
		}
	}
}


/*--------------------- TimerSet : pause, unpause, set time scale ---------------------*/

void MGE::TimerSet::pause() {
	if (isPaused)
		return;
	pauseTime = ogreTimer.getMilliseconds();
	isPaused = true;
}

void MGE::TimerSet::unpause() {
	if (! isPaused)
		return;
	
	int pause_len = ogreTimer.getMilliseconds() - pauseTime;
	
	LOG_DEBUG(" old is: ");
	for (auto& iter : timers) {
		LOG_DEBUG(" * " << iter.first);
	}
	
	LOG_VERBOSE("timer recalc after " << pause_len << "ms pause; "
		<< " with lastUpdate=" << lastUpdate << " with pauseTime=" << pauseTime
	);
	std::multimap<unsigned int, TimerInstance*> old_timers = timers;
	timers.clear();
	
	for (auto& iter : old_timers) {
		timers.insert(std::make_pair( iter.first + pause_len, iter.second )); 
	}
	
	LOG_DEBUG(" now is: " << ogreTimer.getMilliseconds() );
	for (auto& iter : timers) {
		LOG_DEBUG(" * " << iter.first);
	}
	
	lastUpdate += pause_len;
	if (lastUpdate > ogreTimer.getMilliseconds()) {
		LOG_WARNING("lastUpdate > now");
		lastUpdate = ogreTimer.getMilliseconds();
	}
	isPaused = false;
}

void MGE::TimerSet::setTimeScale(float scale) {
	if (scale == 0.0f) {
		pause();
	} else {
		if (scale != timeScale) {
			if (timeScale == 0.0f) {
				unpause();
			} else {
				LOG_INFO("change time scale from: " << timeScale << " to: " << scale);
				std::multimap<unsigned int, TimerInstance*> old_timers = timers;
				timers.clear();
				
				reverseTimeScale = 1.0 / scale;
				unsigned int now = ogreTimer.getMilliseconds();
				
				typename std::multimap<unsigned int, TimerInstance*>::iterator i; 
				for (i = old_timers.begin(); i != old_timers.end(); ++i) {
					unsigned int newTime = (i->first - now) * timeScale * reverseTimeScale;
					LOG_DEBUG("Update timer: old_time=" << (i->first - now) << " new_time=" << newTime);
					timers.insert(std::make_pair( newTime + now, i->second ));
				}
			}
			timeScale = scale;
		}
	}
}

/*--------------------- TimerSet : constructor, destructor ---------------------*/

MGE::TimerSet::TimerSet(const std::string_view& name) :
	setName(name), timeScale(1.0), reverseTimeScale(1.0), isPaused(true), counter(0), pauseTime(0), lastUpdate(0)
{
	if (!setName.empty()) {
		MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
		MGE::ScriptsSystem::getPtr()->getGlobalsDict()["MGE"].attr(setName.c_str()) = this;
	}
}

MGE::TimerSet::~TimerSet() {
	unload();
}


/*--------------------- TimerSet : strore(), restore() and unload() ---------------------*/

bool MGE::TimerSet::unload() {
	LOG_INFO("unset TimerSet data");
	
	for (auto i = timers.begin(); i != timers.end(); ++i) {
		delete i->second;
	}
	timers.clear();
	
	isPaused = true;
	pauseTime = 0;
	reverseTimeScale = 1.0;
	timeScale = 1.0;
	counter = 0;
	lastUpdate = 0;
	
	return true;
}

const std::string_view MGE::TimerSet::getXMLTagName() const {
	return setName;
};


bool MGE::TimerSet::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	LOG_INFO("store TimerSet data");
	
	int now = isPaused ? pauseTime : ogreTimer.getMilliseconds();
	
	xmlNode.append_child("counter") <<  counter;
	auto xmlSubNode = xmlNode.append_child("timers");
	for (auto iter : timers) {
		auto xmlSubSubNode = xmlSubNode.append_child("timer");
		xmlSubSubNode.append_child("timeout") << (iter.first - now);
		iter.second->storeToXML(xmlSubSubNode);
	}
	return true;
}

bool MGE::TimerSet::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("restore TimerSet data");
	
	// clear before restore
	unload();
	
	// restore always in pause mode
	isPaused = true;
	lastUpdate = pauseTime = ogreTimer.getMilliseconds();
	
	// restore counter value
	counter = xmlNode.child("counter").text().as_uint();
	LOG_DEBUG("restored counter for " << setName << " is " << counter);
	
	// restore timers map
	for (auto xmlSubNode : xmlNode.child("timers")) {
		unsigned int tmpTimeout = xmlSubNode.child("timeout").text().as_uint();
		LOG_DEBUG("timeout=" << tmpTimeout << " now=" << pauseTime);
		timers.insert(std::make_pair( tmpTimeout + pauseTime, new TimerInstance(xmlSubNode) ));
	}
	
	#ifdef MGE_DEBUG
	LOG_DEBUG("restored timers in set " << setName << ":");
	for (auto& iter : timers) {
		LOG_DEBUG(" * " << iter.first << " -> " << iter.second->name << "/" << iter.second->scriptName);
	}
	#endif
	return true;
}

/*--------------------- TimerSet : utils ---------------------*/

void MGE::TimerSet::printTimers() {
	LOG_DEBUG("TIMERS: ");
	for (auto& ti : timers) {
		LOG_DEBUG("  time = " << ti.first);
		TimerInstance* t = ti.second;
		LOG_DEBUG("    name:       " << t->name);
		LOG_DEBUG("    period:     " << t->period);
		LOG_DEBUG("    catchup:    " << t->catchup);
		LOG_DEBUG("    scriptName: " << t->scriptName);
		LOG_DEBUG("    scriptArg:  " << t->scriptArg);
	}
}

const std::string MGE::TimerSet::getCounterStr(int offset, MGE::null_end_string format) {
	int val = counter - offset;
	unsigned int s = (val / 1000) % 60;
	unsigned int m = (val / 60000) % 60;
	unsigned int h = (val / 3600000);
	return (boost::format(format) % h % m % s).str();
}



/*--------------------- TimeSystem ---------------------*/

MGE::TimeSystem::TimeSystem() : MGE::SaveableToXML<TimeSystem>(301, 401) {
	LOG_HEADER("Create TimeSystem");
	
	gameTimer      = new MGE::TimerSet("gameTimer");
	realtimeTimer  = new MGE::TimerSet("realtimeTimer");
	isPaused       = true;
	pauseKey       = 0;
	
	// register main loop listener for update timers
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, TIME_ACTIONS);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_TimeSystem \<TimeSystem\>

@c \<TimeSystem\> is used for setup <b>Time System</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(TimeSystem) {
	return new MGE::TimeSystem();
}


bool MGE::TimeSystem::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	if (context->preLoad)
		return false;
	
	LOG_INFO("restore TimeSystem data");
	gameTimer->restoreFromXML(
		xmlNode.child(gameTimer->getXMLTagName().data()),
		context
	);
	realtimeTimer->restoreFromXML(
		xmlNode.child(realtimeTimer->getXMLTagName().data()),
		context
	);
	realtimeTimer->unpause();
	isPaused = true;
	
	return true;
}

bool MGE::TimeSystem::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	LOG_INFO("store TimeSystem data");
	
	auto xmlSubNode = xmlNode.append_child( gameTimer->getXMLTagName().data() );
	gameTimer->storeToXML(xmlSubNode, onlyRef);
	
	xmlSubNode = xmlNode.append_child( realtimeTimer->getXMLTagName().data() );
	realtimeTimer->storeToXML(xmlSubNode, onlyRef);
	
	return true;
}

MGE::TimeSystem::~TimeSystem() {
	LOG_INFO("Destroy TimeSystem");
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	delete gameTimer;
	delete realtimeTimer;
}

bool MGE::TimeSystem::unload() {
	LOG_INFO("unload TimeSystem data");
	isPaused = true;
	pauseKey = 0;
	gameTimer->unload();
	realtimeTimer->unload();
	return true;
}

bool MGE::TimeSystem::update(float gameTimeStep, float realTimeStep) {
	gameTimer->update();
	realtimeTimer->update();
	return true;
}

void MGE::TimeSystem::pause(int key) {
	if(isPaused)
		return;
	
	LOG_INFO("PAUSE GAME");
	isPaused = true;
	pauseKey = key;
	gameTimer->pause();
	
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::GameSpeedChangeEventMsg(gameTimer->getTimeScale(), isPaused) );
}

void MGE::TimeSystem::unpause(int key) {
	if (pauseKey != key && pauseKey != 0) {
		LOG_WARNING("Can't unpause - call with diffrent pause key");
		return;
	}
	
	LOG_INFO("UNPAUSE GAME");
	isPaused = false;
	gameTimer->unpause();
	
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::GameSpeedChangeEventMsg(gameTimer->getTimeScale(), isPaused) );
}

void MGE::TimeSystem::setSpeed(float s) {
	gameTimer->setTimeScale(s);
	
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::GameSpeedChangeEventMsg(gameTimer->getTimeScale(), isPaused) );
}
