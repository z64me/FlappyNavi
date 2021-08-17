/*
 * collider.c <z64.me>
 *
 * a simple collider arena implementation
 *
 */

#include "common.h"


/******************************
 *
 * private types and functions
 *
 ******************************/

enum ColliderType
{
	COLLIDER_TYPE_RECT
	, COLLIDER_TYPE_MAX
};

struct ColliderInit
{
	enum ColliderType      type;     /* type of collider */
	union
	{
		SDL_Rect            rect;
	} shape;
};

struct Collider
{
	struct Collider      *next;      /* next collider in list */
	struct Flappy        *game;      /* main gameplay context */
	void                 *instance;  /* gameplay entity/instance */
	struct ColliderInit   init;      /* union representing various shapes */
	int                   expired;   /* for reusing colliders */
	int                   touched;   /* already touched another collider */
	uint32_t              color;     /* color rgb888 (used when drawing) */
	uint32_t              group;     /* ignore colliders of the same group */
	ColliderCallback     *cb;        /* collider callback */
};

static uint32_t ColorTweak(uint32_t color)
{
	return color ^ 0xffffffff;
}

static struct Collider *Touch(struct Flappy *game, struct Collider *this)
{
	struct Collider *c;
	
	assert(game);
	assert(this);
	
	/* expired colliders */
	if (this->expired || this->touched)
		return 0;
	
	/* for each collider in list */
	for (c = game->colliderList; c; c = c->next)
	{
		if (c->expired /* ignore expired colliders */
			|| c == this /* ignore self collisions */
			|| c->group == this->group /* ignore colliders of the same group */
		)
			continue;
		
		/* rect / --- */
		if (this->init.type == COLLIDER_TYPE_RECT)
		{
			/* rect / rect */
			if (c->init.type == COLLIDER_TYPE_RECT
				&& CollisionRectRect(this->init.shape.rect, c->init.shape.rect)
			)
				break;
		}
	}
	
	return c;
}


/******************************
 *
 * public functions
 *
 ******************************/

/* constructs a quick init parameter for use as an argument to ColliderPush() */
const struct ColliderInit *ColliderInitRect(struct Flappy *game, const float x, const float y, const float w, const float h)
{
	static struct ColliderInit collider;
	
	collider.type = COLLIDER_TYPE_RECT;
	collider.shape.rect = (SDL_Rect){
		ROUNDING(x * game->scale)
		, ROUNDING(y * game->scale)
		, ROUNDING(w * game->scale)
		, ROUNDING(h * game->scale)
	};
	
	return &collider;
}

/* initialize collision arena */
void ColliderArenaInit(struct Flappy *game)
{
	struct Collider *c;
	
	assert(game);
	
	/* mark all colliders as expired */
	for (c = game->colliderList; c; c = c->next)
		c->expired = 1;
}

/* register a collider into a collision frame */
void ColliderArenaPush(struct Flappy *game, void *instance, ColliderCallback cb, const uint32_t color, const struct ColliderInit *init)
{
	struct Collider *c;
	
	assert(game);
	assert(init->type < COLLIDER_TYPE_MAX);
	
	/* look for potentially unused particle */
	for (c = game->colliderList; c; c = c->next)
		if (c->expired)
			break;
	
	/* no unused colliders; allocate a new one and link into list */
	if (!c)
	{
		c = malloc(sizeof(*c));
		c->next = game->colliderList;
		game->colliderList = c;
	}
	
	/* now set it up */
	c->cb = cb;
	c->game = game;
	c->instance = instance;
	c->color = color;
	c->group = color;
	c->expired = 0;
	c->touched = 0;
	c->init = *init;
}

/* draw collider arena's contents */
void ColliderArenaDraw(struct Flappy *game, uint32_t bgcolor, uint32_t outlinecolor, const int opacity)
{
	struct Collider *c;
	uint8_t r, g, b, a;
	SDL_Rect full = {0, 0, WINDOW_W * game->scale, WINDOW_H * game->scale};
	
	assert(game);
	
	SDL_GetRenderDrawColor(game->renderer, &r, &g, &b, &a);
	SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
	
	/* clear background */
	SDL_SetRenderDrawColor(
		game->renderer
		, bgcolor >> 24
		, bgcolor >> 16
		, bgcolor >> 8
		, bgcolor
	);
	PrimitiveRect(game, full);
	
	for (c = game->colliderList; c; c = c->next)
	{
		if (c->expired)
			continue;
		
		SDL_SetRenderDrawColor(game->renderer, c->color >> 16, c->color >> 8, c->color, opacity);
		
		switch (c->init.type)
		{
			case COLLIDER_TYPE_RECT:
				PrimitiveRect(game, c->init.shape.rect);
				if (outlinecolor)
				{
					SDL_SetRenderDrawColor(
						game->renderer
						, outlinecolor >> 24
						, outlinecolor >> 16
						, outlinecolor >> 8
						, outlinecolor
					);
					PrimitiveRectOutline(game, c->init.shape.rect);
				}
				break;
			
			case COLLIDER_TYPE_MAX:
				break;
		}
	}
	
	SDL_SetRenderDrawColor(game->renderer, r, g, b, a);
}

/* execute collider frame by testing every collider against every other */
void ColliderArenaProcess(struct Flappy *game)
{
	struct Collider *c;
	struct Collider *touch;
	
	assert(game);
	
	/* test every collider against every other collider */
	for (c = game->colliderList; c; c = c->next)
	{
		/* a collision happened */
		if ((touch = Touch(game, c)))
		{
			c->color = ColorTweak(c->color);
			touch->color = ColorTweak(touch->color);
			
			c->touched = touch->touched = 1;
			
			if (touch->cb)
				touch->cb(game, touch->instance);
			
			if (c->cb)
				c->cb(game, c->instance);
		}
	}
}

