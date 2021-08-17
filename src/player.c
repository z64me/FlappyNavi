/*
 * player.c <z64.me>
 *
 * player logic and rendering
 *
 */

#include "common.h"

/******************************
 *
 * private types and functions
 *
 ******************************/

#define GHOST_MAX    64     /* for deriving earlier player positions */
#define GHOST_SPEED  20000  /* milliseconds for ghost to catch up */

enum FairyType
{
	FAIRY_NAVI = 0
	, FAIRY_SHADOW = 1
	, FAIRY_TATL = 2
	, FAIRY_TAEL = 3
	, FAIRY_MAX
};

struct FairySprite
{
	enum FairyType      type;
	enum ParticleType   particle;          /* type of particle this fairy emits */
	uint32_t            particleTime;      /* time last particle was spawned */
};

/* quadratic equation parameters */
struct Parabola
{
	float               y;                 /* initial y position */
	uint32_t            ticks;             /* initial time */
};

struct Player
{
	float               x;                 /* position */
	float               y;
	struct Parabola     ghost[GHOST_MAX];  /* earlier player parabolas */
	struct FairySprite  sprite[FAIRY_MAX]; /* Navi's friends */
	struct Parabola     parabola;          /* player position calculations */
	int                 mouseUp;           /* mouse state tracking */
	int                 isDead;            /* boolean player is dead */
};

/* player movement is accomplished using a simple quadratic equation;
 * this solution keeps the game framerate-independent without having
 * to introduce frame step logic
 * https://www.slideshare.net/snewgas/applications-of-the-vertex-formula-edit-8191421
 */
static float ParabolaMotion(struct Parabola p, uint32_t milliseconds)
{
	float seconds;
	
	seconds = milliseconds * 0.001f;
	
	return PLAYER_GRV * pow(seconds, 2) + PLAYER_YVEL * seconds + p.y;
}

/* linearly interpolate from `hi` to `lo` across `total` milliseconds */
static float creep(float lo, float hi, uint32_t total, uint32_t now)
{
	if (now >= total)
		return hi;
	
	return lo + (hi - lo) * ((float)now / total);
}

/* this callback fires when the player collides with the world */
static void OnTouchWorld(struct Flappy *game, void *instance)
{
	struct Player *player = instance;
	float x;
	float y;
	
	assert(game);
	assert(player);
	
	player->isDead = 1;
	PlayerGetCenter(player, &x, &y);
	ParticlePush(game, PARTICLE_DEATH, x, y);
	FlappyGameOver(game);
}

static void ConvertCenter(float *x, float *y)
{
	assert(x);
	assert(y);
	
	*x += 16 / 2;
	*y += 8 / 2;
}

static void DrawPlayerSprite(struct Flappy *game, struct FairySprite *fairy, uint32_t along, float x, float y)
{
	unsigned sprite;
	int yofs = 0;
	
	assert(game);
	assert(fairy);
	
	/* hovering in place */
	if (!game->playerflapped)
	{
		sprite = game->ticks / 150;
		sprite %= 3;
		
		/* shift the sprite down a little on this frame */
		if (sprite == 2)
			yofs = 1;
	}
	
	/* player is in motion */
	else {
		unsigned flapRate = 100;
		int arr[] = {1, 0, 1, 2};
		
		sprite = along;
		sprite /= flapRate;
		if (sprite >= ARRAY_COUNT(arr))
			sprite = ARRAY_COUNT(arr) - 1;
		sprite = arr[sprite];
	}
	
	/* draw fairy sprite */
	SpritesheetDraw(game, game->sprites, fairy->type, sprite, x, y + yofs);
	
	/* spawn particles */
	if (game->ticks - fairy->particleTime >= PLAYER_PARTFREQ)
	{
		fairy->particleTime = game->ticks;
		
		y += FlappyRand(game) % 16;
		x += FlappyRand(game) % 16;
		ParticlePush(game, fairy->particle, x, y);
	}
}

/* get earlier player y position and time since flap */
static float GhostY(struct Flappy *game, struct Player *player, float x, uint32_t *since)
{
	struct Parabola *p;
	struct Parabola *ghostEnd;
	uint32_t ago;
	uint32_t when;
	uint32_t along;
	
	assert(game);
	assert(player);
	
	/* time since flap */
	if (since)
		*since = 0;
	
	/* calculate how many milliseconds ago player was at x position */
	ago = ((player->x - x) / SCROLL_SPEED) * 1000;
	
	/* last ghost in list */
	ghostEnd = player->ghost + GHOST_MAX;
	
	when = game->ticks - ago;
	
	/* special case: it could be in the first parabola */
	p = player->ghost;
	if (when < p->ticks)
	{
		/* it isn't, so now test each */
		for (p = player->ghost + 1; p != ghostEnd; ++p)
		{
			/* in parabola window */
			if (when >= (p)->ticks && when < (p-1)->ticks)
				break;
		}
	}
	
	/* no ghost found; return off-screen position */
	if (p == ghostEnd || !p->ticks)
		return WINDOW_H * 2;
	
	/* time since flap */
	along = when - p->ticks;
	if (since)
		*since = along;
	
	return ParabolaMotion(*p, along);
}

/* display all earlier player positions */
static void GhostDebug(struct Flappy *game, struct Player *player)
{
	float x;
	int thickness = 3;
	
	for (x = player->x; x >= -32; x -= 0.25f)
	{
		float xP;
		float yP;
		float h, s, v;
		SDL_Rect rect;
		uint8_t r, g, b;
		
		xP = x;
		yP = GhostY(game, player, x, 0);
		ConvertCenter(&xP, &yP);
		rect.x = xP * game->scale;
		rect.y = yP * game->scale;
		rect.w = thickness * game->scale;
		rect.h = thickness * game->scale;
		rect.x -= rect.w / 2;
		rect.y -= rect.h / 2;
		
		h = fabs(fmod(WORLD_SCROLL(x * 50) / WINDOW_W, 1.0));
		s = 1;
		v = 1;
		HsvToRgb8(h, s, v, &r, &g, &b);
		SDL_SetRenderDrawColor(game->renderer, r, g, b, -1);
		
		PrimitiveRect(game, rect);
	}
}

/* draw a ghost fairy at earlier player position `x` */
static void Ghost(struct Flappy *game, struct Player *player, struct FairySprite *fairy, float x)
{
	float y;
	uint32_t since;
	
	assert(game);
	assert(player);
	
	y = GhostY(game, player, x, &since);
	
	DrawPlayerSprite(game, fairy, since, x, y);
}

static void GhostPush(struct Player *player, struct Parabola parabola)
{
	struct Parabola *p;
	
	assert(player);
	
	/* shift every array element along */
	for (p = &player->ghost[GHOST_MAX - 1]; p > player->ghost; --p)
		*p = *(p-1);
	
	/* new value at head of array */
	*p = parabola;
}


/******************************
 *
 * public functions
 *
 ******************************/

struct Player *PlayerNew(struct Flappy *game)
{
	struct Player *player = calloc(1, sizeof(*player));
	const int fairyParticle[] = {
		[FAIRY_NAVI] = PARTICLE_SPARKLE_BLUE
		, [FAIRY_SHADOW] = PARTICLE_SPARKLE_GRAY
		, [FAIRY_TATL] = PARTICLE_SPARKLE_YELLOW
		, [FAIRY_TAEL] = PARTICLE_SPARKLE_PURPLE
	};
	int i;
	
	if (!player)
		return 0;
	
	/* initialize all fairy sprite types */
	for (i = 0; i < FAIRY_MAX; ++i)
		player->sprite[i] = (struct FairySprite){
			.type = i
			, .particle = fairyParticle[i]
			, .particleTime = 0
		};
	
	return player;
	
	(void)game;
}

void PlayerSetPos(struct Player *player, float x, float y)
{
	assert(player);
	
	player->x = x;
	player->y = y;
}


void PlayerGetCenter(struct Player *player, float *x, float *y)
{
	assert(player);
	assert(x);
	assert(y);
	
	*x = player->x;
	*y = player->y;
	
	ConvertCenter(x, y);
}


float PlayerGetX(struct Player *player)
{
	assert(player);
	
	return player->x;
}

void PlayerInit(struct Flappy *game, struct Player *player)
{
	assert(game);
	assert(player);
	
	PlayerSetPos(player, (WINDOW_W - 16) / 2, (WINDOW_H - 20) / 2);
	player->isDead = 0;
	
	(void)game;
}

void PlayerFree(struct Player *player)
{
	assert(player);
	
	free(player);
}

void PlayerUpdate(struct Flappy *game, struct Player *player)
{
	
	assert(game);
	assert(player);
	
	if (player->isDead)
		return;
	
	if (game->state != FLAPPY_STATE_PLAYING)
		return;
	
	/* only apply gravity if player has started flapping */
	if (game->playerflapped)
		player->y = ParabolaMotion(player->parabola, game->ticks - player->parabola.ticks);
	
	/* mouse click = flap your wings */
	if (!game->buttonhover && player->mouseUp && game->input.mouseDown)
	{
		/* initial flap */
		if (!game->playerflapped)
		{
			memset(player->ghost, 0, sizeof(player->ghost));
			game->playerflapped = 1;
		}
		
		/* set up current flap */
		player->parabola.ticks = game->ticks;
		player->parabola.y = player->y;
		
		/* store new flap as ghost flap */
		GhostPush(player, player->parabola);
	}
	player->mouseUp = !game->input.mouseDown;
	
	/* collider */
	ColliderArenaPush(game, player, OnTouchWorld, COLOR_PLAYER, ColliderInitRect(game, player->x + 4, player->y + 4, 8, 4));
}

void PlayerDraw(struct Flappy *game, struct Player *player)
{
	assert(game);
	assert(player);
	
	if (player->isDead)
		return;
	
	/* player is in motion: draw ghosts */
	if (game->playerflapped)
	{
		uint32_t ticks = fmin(game->themeTicks, game->stateTicks);
		float fairy1 = creep(-50, 50, GHOST_SPEED, ticks);
		float fairy2 = creep(-75, 25, GHOST_SPEED, ticks);
		
		if (game->debug & FLAPPY_DEBUG_GHOST)
			GhostDebug(game, player);
		
		if (game->theme == FLAPPY_THEME_WATERTEMPLE)
		{
			Ghost(game, player, &player->sprite[FAIRY_SHADOW], fairy1);
		}
		
		else if (game->theme == FLAPPY_THEME_TERMINA)
		{
			Ghost(game, player, &player->sprite[FAIRY_TATL], fairy1);
			Ghost(game, player, &player->sprite[FAIRY_TAEL], fairy2);
		}
	}
	
	DrawPlayerSprite(game, &player->sprite[FAIRY_NAVI], game->ticks - player->parabola.ticks, player->x, player->y);
}

