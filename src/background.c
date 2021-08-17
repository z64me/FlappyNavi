/*
 * obstacle.c <z64.me>
 *
 * the obstacles the player must clear
 *
 */

#include "common.h"

struct Background
{
	unsigned index;   /* which frame in backgrounds.png to display */
	unsigned anim;    /* second animation frame in backgrounds.png */
	uint32_t time;    /* animation frame duration in milliseconds */
};

#define STRIDE  2    /* number of images per row */
#define WIDTH   200  /* width of a background image */
#define HEIGHT  112  /* height of a background image */

void BackgroundDrawFloor(struct Flappy *game)
{
	SDL_Rect clip;
	float scroll;
	
	assert(game);
	
	clip = game->bgClip;
	
	clip.y += clip.h;
	clip.h = FLOOR_H;
	clip.y -= clip.h;
	scroll = WORLD_SCROLL(game->ticks);
	scroll = fmodf(scroll, WIDTH);
	scroll = -scroll;
	TextureDraw(game, game->backgrounds, clip, scroll, HEIGHT - clip.h);
	TextureDraw(game, game->backgrounds, clip, scroll + WIDTH, HEIGHT - clip.h);
}

void BackgroundDraw(struct Flappy *game)
{
	const struct Background *bg;
	const struct Background bgArray[] = {
		[FLAPPY_THEME_FOREST] = {
			.index = 0
		}
		, [FLAPPY_THEME_MOUNTAIN] = {
			.index = 1
		}
		, [FLAPPY_THEME_JABU] = {
			.index = 2
		}
		, [FLAPPY_THEME_WATERTEMPLE] = {
			.index = 3
			, .anim = 4
			, .time = 500
		}
		, [FLAPPY_THEME_DESERT] = {
			.index = 5
			, .anim = 6
			, .time = 500
		}
		, [FLAPPY_THEME_TERMINA] = {
			.index = 7
		}
	};
	SDL_Rect clip;
	enum FlappyTheme theme;
	unsigned index;
	
	assert(game);
	assert(game->backgrounds);
	assert(game->theme < FLAPPY_THEME_MAX);
	
	theme = game->theme;
	
	/* select image index from array */
	bg = &bgArray[theme];
	index = bg->index;
	
	/* for animated backgrounds, use alternate frame when needed */
	if (bg->time && (game->ticks % (bg->time * 2)) >= bg->time)
		index = bg->anim;
	
	/* derive clipping rectangle */
	clip.x = WIDTH * index;
	clip.y = (clip.x / (STRIDE * WIDTH)) * HEIGHT;
	clip.x %= STRIDE * WIDTH;
	clip.w = WIDTH;
	clip.h = HEIGHT;
	
	/* display background onto screen */
	TextureDraw(game, game->backgrounds, clip, 0, 0);
	
	game->bgClip = clip;
}

