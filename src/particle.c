/*
 * particle.c <z64.me>
 *
 * basic particle functions
 *
 */

#include "common.h"

#define WIDTH  32  /* width of animation frame */
#define HEIGHT 32  /* height of animation frame */

struct Frame
{
	int       row;  /* row/column in particles.png */
	int       col;
	uint32_t  dur;  /* duration of frame in milliseconds */
};

struct Anim
{
	const struct Frame *array;  /* array ends with .row value < 0 */
};

struct Particle
{
	void     *next;             /* next in list */
	uint32_t  ticks;            /* the time at which it was spawned */
	float     x;                /* spawn point */
	float     y;
	int       expired;          /* expired particles are reused */
	const struct Anim *anim;    /* particle animation */
};

/* particle animation database */
#define SPARKLE_SPEED (PLAYER_PARTFREQ * 2)
#define DEATH_SPEED 125
static const struct Anim anim[] = {
	[PARTICLE_SPARKLE_BLUE] = {(const struct Frame[]) {
		{ 1, 0, SPARKLE_SPEED }
		, { 1, 1, SPARKLE_SPEED }
		, { 1, 2, SPARKLE_SPEED }
		, { -1, -1, -1 }
	}}
	, [PARTICLE_SPARKLE_GRAY] = {(const struct Frame[]) {
		{ 2, 0, SPARKLE_SPEED }
		, { 2, 1, SPARKLE_SPEED }
		, { 2, 2, SPARKLE_SPEED }
		, { -1, -1, -1 }
	}}
	, [PARTICLE_SPARKLE_YELLOW] = {(const struct Frame[]) {
		{ 3, 0, SPARKLE_SPEED }
		, { 3, 1, SPARKLE_SPEED }
		, { 1, 2, SPARKLE_SPEED } /* reuse last blue frame */
		, { -1, -1, -1 }
	}}
	, [PARTICLE_SPARKLE_PURPLE] = {(const struct Frame[]) {
		{ 3, 2, SPARKLE_SPEED }
		, { 3, 3, SPARKLE_SPEED }
		, { 1, 2, SPARKLE_SPEED } /* reuse last blue frame */
		, { -1, -1, -1 }
	}}
	, [PARTICLE_DEATH] = {(const struct Frame[]) {
		{ 0, 0, DEATH_SPEED }
		, { 0, 1, DEATH_SPEED }
		, { 0, 2, DEATH_SPEED }
		, { 0, 3, DEATH_SPEED }
		, { -1, -1, -1 }
	}}
};

/* spawn a new particle */
void ParticlePush(struct Flappy *game, enum ParticleType type, float x, float y)
{
	struct Particle *p;
	
	assert(game);
	assert(type < PARTICLE_MAX);
	
	/* look for potentially unused particle */
	for (p = game->particleList; p; p = p->next)
		if (p->expired)
			break;
	
	/* no unused particles; allocate a new one and link into list */
	if (!p)
	{
		p = malloc(sizeof(*p));
		p->next = game->particleList;
		game->particleList = p;
	}
	
	/* now set it up */
	p->ticks = game->ticks;
	p->x = x;
	p->y = y;
	p->expired = 0;
	p->anim = &anim[type];
}

/* draw all active particles */
void ParticleDrawAll(struct Flappy *game)
{
	struct Particle *p;
	
	assert(game);
	
	for (p = game->particleList; p; p = p->next)
	{
		const struct Frame *f;
		SDL_Rect clip;
		uint32_t ticks = game->ticks - p->ticks;
		uint32_t walk = 0;
		float x = p->x - WIDTH / 2 - WORLD_SCROLL(ticks);
		float y = p->y - HEIGHT / 2;
		
		/* don't draw expired particles */
		if (p->expired)
			continue;
		
		/* for each animation frame in array */
		for (f = p->anim->array; f->row >= 0; ++f)
		{
			/* particle time is inside frame window */
			if (ticks >= walk && ticks < walk + f->dur)
				break;
			walk += f->dur;
		}
		
		/* particle reached end of animation */
		if (f->row < 0)
		{
			p->expired = 1;
			continue;
		}
		
		/* derive clipping rectangle and display onto screen */
		clip = (SDL_Rect){f->col * WIDTH, f->row * HEIGHT, WIDTH, HEIGHT};
		TextureDraw(game, game->particles, clip, x, y);
	}
}

/* clean up game particles */
void ParticleCleanup(struct Flappy *game)
{
	struct Particle *p;
	struct Particle *next = 0;
	
	for (p = game->particleList; p; p = next)
	{
		next = p->next;
		free(p);
	}
	
	game->particleList = 0;
}

