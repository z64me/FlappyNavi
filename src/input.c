/*
 * input.c <z64.me>
 *
 * input functions live here
 *
 */

#include "common.h"

/* handle input */
void InputProcess(struct Flappy *game)
{
	SDL_Event event;
	struct Input *input;
	
	assert(game);
	
	input = &game->input;
	
	/* mouse click cooldown */
	if (input->clicked != 0)
	{
		input->clicked = 0;
		input->mouseDown = 0;
	}
	
	while(SDL_PollEvent(&event))
	{
		
		switch (event.type)
		{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					/* window is shown */
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_MAXIMIZED:
						game->windowMinimized = 0;
						break;
					
					/* window is hidden */
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
						game->windowMinimized = 1;
						break;
				}
				break;
			
			/* cursor/mouse motion */
			case SDL_MOUSEMOTION:
				input->mouseX = (float)event.motion.x / game->scale;
				input->mouseY = (float)event.motion.y / game->scale;
				break;
			
			/* mouse button press */
			case SDL_MOUSEBUTTONDOWN:
				input->clickX = input->mouseX;
				input->clickY = input->mouseY;
				input->mouseDown = 1;
				input->clicked = 0;
				break;
			
			/* mouse button release */
			case SDL_MOUSEBUTTONUP:
				if (input->mouseDown)
				{
					input->clicked = 1;
				}
				break;
			
			/* key press */
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					/* shrink window */
					case SDLK_F1:
						FlappyUpdateWindowSize(game, -1);
						break;
					
					/* inflate window */
					case SDLK_F2:
						FlappyUpdateWindowSize(game, 1);
						break;
					
					/* toggle debugging */
					case SDLK_F12:
						game->debug += 1;
						if (game->debug > FLAPPY_DEBUG_ALL)
							game->debug = 0;
						break;
				}
				break;
			
			/* exit game */
			case SDL_QUIT:
				input->quit = 1;
				break;
		}
	}
}

enum FlappyMouse InputMouseInRect(struct Flappy *game, SDL_Rect rect)
{
	struct Input *input;
	enum FlappyMouse result = 0;
	
	assert(game);
	
	input = &game->input;
	
	/* user's cursor is hovering rectangle */
	if (CollisionPointRect(input->mouseX, input->mouseY, rect))
		result |= FLAPPY_MOUSE_HOVERING;
	
	/* user is in the process of clicking rectangle */
	if (input->mouseDown && CollisionPointRect(input->clickX, input->clickY, rect))
		result |= FLAPPY_MOUSE_CLICKING;
	
	/* user has successfully clicked the rectangle */
	if (input->clicked)
	{
		if ((result & FLAPPY_MOUSE_CLICKING) && (result & FLAPPY_MOUSE_HOVERING))
		{
			input->clicked = 0;
			input->mouseDown = 0;
			result |= FLAPPY_MOUSE_CLICKED;
		}
	}
	
	return result;
}

