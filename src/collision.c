/*
 * collision.c <z64.me>
 *
 * simple collision functions are organized here
 *
 */

#include "common.h"

/* rectangle-rectangle collision */
int CollisionRectRect(const SDL_Rect a, const SDL_Rect b)
{
	if (a.x + a.w < b.x
		|| a.y + a.h < b.y
		|| a.x > b.x + b.w
		|| a.y > b.y + b.h
	)
		return 0;
	
	return 1;
}

/* point-rectangle collision */
int CollisionPointRect(const int x, const int y, const SDL_Rect rect)
{
	return (x > rect.x
		&& x < rect.x + rect.w
		&& y > rect.y
		&& y < rect.y + rect.h
	);
}

