/*
 * world.c <z64.me>
 *
 * displays game world
 *
 */

#include "common.h"

/******************************
 *
 * private types and functions
 *
 ******************************/

/* values are in milliseconds */
#define JABU_FREQ   10000 /* hazard frequency */
#define JABU_SPEED  1500  /* time for it to rise/fall */
#define JABU_TIME   1500  /* how long it stays */
#define JABU_CYCLE  (JABU_FREQ + JABU_SPEED * 2 + JABU_TIME) /* length of one cycle */

static float JabuHazardEaseIn(float p)
{
	return sin(p * M_PI_2);
}

static float JabuHazardEaseOut(float p)
{
	return sin((p - 1) * M_PI_2) + 1;
}

static float JabuHazardHeight(struct Flappy *game)
{
	uint32_t ticks;
	float lo = WINDOW_H;
	float hi = 48;
	float diff = lo - hi;
	
	game->jabuHazardActive = 0;
	
	/* game over screen seamless logic */
	if (game->state == FLAPPY_STATE_GAMEOVER)
		ticks = game->themeTicks;
	
	/* regular gameplay */
	else if (game->state == FLAPPY_STATE_PLAYING)
		ticks = fmin(game->themeTicks, game->stateTicks);
	
	/* ignore hazard on any other screen */
	else
		return lo;
	
	/* isolate timer to one cycle */
	ticks %= JABU_CYCLE;
	
	/* ticks < start time */
	if (ticks < JABU_FREQ)
	{
		/* stage hazard predictor */
		if (ticks >= JABU_FREQ / 2)
			game->jabuHazardActive = 1;
		
		return lo;
	}
	
	/* now start <= ticks <= end */
	ticks -= JABU_FREQ;
	
	/* apex */
	if (ticks >= JABU_SPEED && ticks <= JABU_SPEED + JABU_TIME)
		return hi;
	
	/* lowering */
	if (ticks >= JABU_SPEED + JABU_TIME)
		return hi + diff * JabuHazardEaseOut(((float)ticks - (JABU_SPEED + JABU_TIME)) / JABU_SPEED);
	
	/* rising */
	game->jabuHazardActive = 1;
	return lo - diff * JabuHazardEaseIn((float)ticks / JABU_SPEED);
}

/* jabu stage gimmick */
static void JabuHazard(struct Flappy *game)
{
	SDL_Rect clip;
	int index;
	
	assert(game);

	#define JABU_STRIDE  2    /* number of images per row */
	#define JABU_WIDTH   200  /* hazard frame width */
	#define JABU_HEIGHT  112  /* hazard frame height */
	
	index = (game->ticks / 100) % 8;
	
	/* derive clipping rectangle */
	clip.x = JABU_WIDTH * index;
	clip.y = (clip.x / (JABU_STRIDE * JABU_WIDTH)) * JABU_HEIGHT;
	clip.x %= JABU_STRIDE * JABU_WIDTH;
	clip.w = JABU_WIDTH;
	clip.h = JABU_HEIGHT;
	
	SDL_SetTextureAlphaMod(game->jabu, 0x95);
	TextureDraw(game, game->jabu, clip, 0, JabuHazardHeight(game));
	
	#undef JABU_STRIDE
	#undef JABU_WIDTH
	#undef JABU_HEIGHT
}


/******************************
 *
 * public functions
 *
 ******************************/

/* process stage-specific hazards */
void WorldDoHazards(struct Flappy *game)
{
	/* jabu hazard hitbox */
	if (game->theme == FLAPPY_THEME_JABU)
		ColliderArenaPush(game, 0, 0, COLOR_WORLD, ColliderInitRect(game, 0, JabuHazardHeight(game) + 4, WINDOW_W, WINDOW_H));
}

/* draw the game world */
void WorldDraw(struct Flappy *game)
{
	assert(game);
	
	/* draw the main background */
	BackgroundDraw(game);
	
	/* then obstacles */
	ObstacleDrawAll(game);
	
	/* the scrolling floor is drawn over the obstacles */
	BackgroundDrawFloor(game);
	
	/* jabu stage gimmick */
	if (game->theme == FLAPPY_THEME_JABU)
		JabuHazard(game);
	
	/* draw particles */
	ParticleDrawAll(game);
	
	/* draw the player */
	PlayerDraw(game, game->player);
}

