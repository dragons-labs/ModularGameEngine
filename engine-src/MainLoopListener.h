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

namespace MGE {

/// @addtogroup EngineMain
/// @{
/// @file

/**
 * @brief Base class for main loop listener.
 */
struct MainLoopListener {
	/// @ref MGE::Engine::mainLoopListeners key values for typical engine class.
	enum StandardLevels {
		PHYSICS_ACTIONS     = 10,  ///< physics calculation and actions
		
		TIME_ACTIONS        = 50,  ///< time processing and actions
		INPUT_ACTIONS       = 60,  ///< input processing and action event
		
		PRE_RENDER_ACTIONS  = 100, ///< user pre rendering
		PRE_RENDER_GUI      = 230, ///< gui processing (for this frame!)
		PRE_RENDER          = 240, ///< camera, animation, texture render
		
		GRAPHICS_RENDER     = 250, ///< ogre graphics render
		
		POST_RENDER         = 260, ///< audio processing
		POST_RENDER_ACTIONS = 300, ///< user post rendering (for next frame!)
		POST_RENDER_GUI     = 350, ///< gui processing (for next frame!)
	};
	
	/**
	 * @brief Method to execute in main loop while game is running or in active pause mode, to update engine element for rendering next frame.
	 * 
	 * @param gameTimeStep  Game (influenced by the game speed) time since last call of this function in seconds.
	 * @param realTimeStep  Real (not influenced by the game speed) time since last call of this function in seconds.
	 * 
	 * @return
	 *   True on success false on fail / error (standard ClassListenerSet interface).
	 *   Currently returned value is ignored.
	 */
	virtual bool update(float gameTimeStep, float realTimeStep) = 0;
	
	/**
	 * @brief Method to execute in main loop while game is in full pause mode (e.g. in main menu), to update engine element for rendering next frame.
	 *        By default (if not override) do nothing.
	 * 
	 * @param realTimeStep  Real (not influenced by the game speed) time since last call of this function in seconds.
	 * 
	 * @return
	 *   True on success false on fail / error (standard ClassListenerSet interface).
	 *   Currently returned value is ignored.
	 */
	virtual bool updateOnFullPause(float realTimeStep) { return true; }
	
	/// Empty virtual destructor.
	virtual ~MainLoopListener() = default;
};

/// @}

}
