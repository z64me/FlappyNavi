/*
 * primitive.c <z64.me>
 *
 * functions for displaying basic primitives
 *
 */

#include "common.h"

void PrimitiveRect(struct Flappy *game, SDL_Rect r)
{
	assert(game);
	
	SDL_RenderFillRect(game->renderer, &r);
}

void PrimitiveRectOutline(struct Flappy *game, SDL_Rect r)
{
	assert(game);
	
	SDL_RenderDrawRect(game->renderer, &r);
}

