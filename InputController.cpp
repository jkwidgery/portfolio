/*****************************************************************//**
 * \file   InputController.cpp
 * \brief  contains input controller implementation
 *
 * \author Jasmine Widgery, Emma Lewarne
 * \date   September 2021
 * 
 * \param Copyright 2021 DigiPen (USA) Corporation.
 *********************************************************************/

#include <iostream>
#include "InputController.h"
#include <unordered_map>
#include <glm.hpp>
#include "Core.h"

std::unordered_map<SDL_Keycode, bool> currKeys_; // Hash map for identifying keys
std::unordered_map<SDL_Keycode, bool> prevKeys_; // Hash map for identifying keys (previous frame)

glm::vec2 currMousePos; // Stores the current mouse position
glm::vec2 prevMousePos;

bool butt1; // The current state of the left mouse button
bool butt2; // The current state of the right mouse button
bool butt3; // The current state of the middle mouse button
bool prevButt1;
bool prevButt2;
bool prevButt3;

float wheelY;

bool buttA;
bool buttB;
bool buttX;
bool buttY;
bool back;
bool start;
bool rShoulder;
bool lShoulder;
bool dPadUp;
bool dPadDown;
bool dPadRight;
bool dPadLeft;


namespace InputController
{	
	/**
	 * Handles input event using SDL library
	 * \param event
	 */
	void handleInputEvent(SDL_Event event)
	{
		// If event is a keyboard event, call the handler
		if (event.type >= SDL_KEYDOWN && event.type <= SDL_KEYMAPCHANGED)
		{
			handleKeyboardEvent(event);
		}

		// If event is a mouse event, call the handler
		if (event.type >= 1024 && event.type <= 1027)
		{
			handleMouseEvent(event);
		}

		// If event is a joystick event, call the handler
		if (event.type >= 1536 && event.type <= 1542)
		{
			//handleJoystickEvent
		}
	}


	/**
	 * stores keys constantly
	 * 
	 */
	void Update()
	{
		prevKeys_ = currKeys_;
		prevButt1 = butt1;
		prevButt2 = butt2;
		prevButt3 = butt3;
		prevMousePos = currMousePos;
		wheelY = 0;
	}

	/**
	 * Call this function if you want to know if a certain button on a keyboard is pressed.
	 * \param key
	 * \return bool
	 */
	bool keyCodePressed(SDL_Keycode key)
	{
		std::unordered_map<SDL_Keycode, bool>::iterator it = currKeys_.find(key);
		if (it != currKeys_.end())
		{
			if (it->second == true)
			{
				return true;
			}
		}
		return false;
	}


	/**
	 * Call this function if you want to know if a certain button on a keyboard has been pressed in the last frame.
	 * \param key
	 * \return bool
	 */
	bool keyCodeTriggered(SDL_Keycode key)
	{
		std::unordered_map<SDL_Keycode, bool>::iterator it = currKeys_.find(key);
		std::unordered_map<SDL_Keycode, bool>::iterator itPrev = prevKeys_.find(key);

		if (it != currKeys_.end())
		{
			if ((itPrev == prevKeys_.end() || itPrev->second == false) && it->second == true)
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * Call this function if you want to know if a certain button on a keyboard has been released in this frame.
	 * \param key
	 * \return bool
	 */
	bool keyCodeUp(SDL_Keycode key)//first frame of being released
	{
		std::unordered_map<SDL_Keycode, bool>::iterator it = currKeys_.find(key);
		std::unordered_map<SDL_Keycode, bool>::iterator itPrev = prevKeys_.find(key);

		if (it != currKeys_.end())
		{
			if (itPrev != prevKeys_.end() && (itPrev->second == true) && it->second == false)
			{
				return true;
			}
		}
		return false;
	}


	/**
	 * Call this function if you want to know if a certain button on a keyboard has been released.
	 * \param key
	 * \return bool
	 */
	bool keyCodeReleased(SDL_Keycode key)
	{
		std::unordered_map<SDL_Keycode, bool>::iterator it = currKeys_.find(key);
		if (it != currKeys_.end())
		{
			if (it->second == false)
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * Get current mouse position.
	 * \return vec2
	 */
	glm::vec2 getMousePos()
	{
		return currMousePos;
	}

	/**
	 * Get change in mouse position.
	 * \return vec2
	 */
	glm::vec2 getDeltaMousePos()
	{
		return currMousePos - prevMousePos;
	}

	/**
	 * Get normalized mouse position.
	 * \return vec2
	 */
	glm::vec2 getNormalizedMousePos()
	{
		return glm::vec2(currMousePos.x / CloudEngine::Engine::Core::WindowWidth(), currMousePos.y / CloudEngine::Engine::Core::WindowHeight());
	}

	/**
	 * Get if left mouse button has been pressed.
	 * \return bool
	 */
	bool leftMouseButtonPressed()
	{
		if (butt1 == true)
		{
			return butt1;
		}
		return false;
	}

	/**
	 * Get if left mouse button has been triggered.
	 * \return bool
	 */
	bool leftMouseButtonTriggered()
	{
		if (prevButt1 == false && butt1 == true)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if left mouse button has been released.
	 * \return bool
	 */
	bool leftMouseButtonReleased()
	{
		if (butt1 == false)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if left mouse button is up (first frame).
	 * \return bool
	 */
	bool leftMouseButtonUp()
	{
		if (prevButt1 == true && butt1 == false)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if right mouse button has been pressed.
	 * \return bool
	 */
	bool rightMouseButtonPressed()
	{
		if (butt2 == true)
		{
			return butt2;
		}
		return false;
	}

	/**
	 * Get if right mouse button has been triggered.
	 * \return bool
	 */
	bool rightMouseButtonTriggered()
	{
		if (prevButt2 == false && butt2 == true)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if right mouse button has been released.
	 * \return bool
	 */
	bool rightMouseButtonReleased()
	{
		if (butt2 == false)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if right mouse button has been released (first frame).
	 * \return bool
	 */
	bool rightMouseButtonUp()
	{
		if (prevButt2 == true && butt2 == false)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if middle mouse button has been pressed.
	 * \return bool
	 */
	bool middleMouseButtonPressed()
	{
		if (butt3 == true)
		{
			return butt3;
		}
		return false;
	}

	/**
	 * Get if middle mouse button has been triggered.
	 * \return bool
	 */
	bool middleMouseButtonTriggered()
	{
		if (prevButt3 == false && butt3 == true)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if middle mouse button has been released.
	 * \return bool
	 */
	bool middleMouseButtonReleased()
	{
		if (butt3 == false)
		{
			return true;
		}
		return false;
	}

	/**
	 * Get if middle mouse button has been released (first frame).
	 * \return bool
	 */
	bool middleMouseButtonUp()
	{
		if (prevButt3 == true && butt3 == false)
		{
			return true;
		}
		return false;
	}

	/**
	 * How much the scroll wheel has scrolled as float.
	 * \return float
	 */
	float getWheelY()
	{
		return wheelY; //wheel scroll value, scrolling up is pos, scrolling down is neg
	}

	//state keyCodeGetState(SDL_KeyboardEvent event)
	//type keyCodeGetType(SDL_KeyboardEvent event)

	/**
	 * Storing keys in a hash map in order to use the info later.
	 * \param event
	 */
	void handleKeyboardEvent(SDL_Event event)
	{
		// Updating unordered map
		if (event.type == SDL_KEYDOWN)
		{
			//std::cout << (char)(event.key.keysym.sym) << ": pressed" << std::endl;
			// Set bool in map to true
			currKeys_[event.key.keysym.sym] = true;
		}
		// Updating unordered map
		if (event.type == SDL_KEYUP)
		{
			//std::cout << (char)(event.key.keysym.sym) << ": Released" << std::endl;
			// Set bool in map to false
			currKeys_[event.key.keysym.sym] = false;
		}
	}

	/**
	 * Storing important mouse info for later.
	 * \param event
	 */
	void handleMouseEvent(SDL_Event event)
	{
		if (event.type == SDL_MOUSEMOTION)
		{
			currMousePos.x = (float)event.motion.x;
			currMousePos.y = (float)event.motion.y;
			//std::cout << currMousePos.x << "," << currMousePos.y << std::endl;
		}
		
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				butt1 = true;
				std::cout << "LEFT BUTTON CLICKED!" << std::endl;
			}
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				butt2 = true;
				std::cout << "RIGHT BUTTON CLICKED!" << std::endl;
			}
			if (event.button.button == SDL_BUTTON_MIDDLE)
			{
				butt3 = true;
				std::cout << "MIDDLE BUTTON CLICKED!" << std::endl;
			}
			
		}

		if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				butt1 = false;
				std::cout << "LEFT BUTTON RELEASED!" << std::endl;
			}
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				butt2 = false;
				std::cout << "RIGHT BUTTON RELEASED!" << std::endl;
			}
			if (event.button.button == SDL_BUTTON_MIDDLE)
			{
				butt3 = false;
				std::cout << "MIDDLE BUTTON RELEASED!" << std::endl;
			}

		}

		if (event.type == SDL_MOUSEWHEEL)
		{
			//The amount scrolled vertically, positive away from the user and negative toward the user
			wheelY = (float)event.wheel.y;
		}
	}
}


