/*
 * obstacle.c <z64.me>
 *
 * the obstacles the player must clear
 *
 */

#include "common.h"

#define OB_W    16   /* width of an obstacle sprite */
#define OB_H    64   /* height of an obstacle sprite */
#define OB_GAP  24   /* size of gap player must get through */
#define OB_DIST 48   /* distance between obstacles */

struct Obstacle
{
	void     *next;   /* next in list */
	SDL_Rect upper;   /* rectangle of upper sprite */
	SDL_Rect lower;   /* rectangle of lower sprite */
	float    x;       /* x position of the obstacle's left edge */
	float    y;       /* y position of the obstacle's center */
	uint32_t ticks;   /* the time at which this one was spawned */
	int      expired; /* mark available for reuse */
	int      cleared; /* player made it through obstacle */
};

void ObstaclePush(struct Flappy *game)
{
	struct Obstacle *ob;
	enum Height
	{
		LOW = 0
		, MEDIUM
		, HIGH
	};
	const int yArray[] = {
		/* low    */  76
		/* medium */, 50
		/* high   */, 25
	};
	static unsigned last[16]; /* store a few previous heights */
	unsigned this;
	
	/* look for potentially unused obstacle */
	for (ob = game->obstacleList; ob; ob = ob->next)
		if (ob->expired)
			break;
	
	/* no unused obstacles; allocate a new one and link into list */
	if (!ob)
	{
		ob = calloc(1, sizeof(*ob));
		ob->next = game->obstacleList;
		game->obstacleList = ob;
	}
	
	ob->upper = (SDL_Rect){-100, -100, OB_W, OB_H};
	ob->lower = ob->upper;
	ob->expired = 0;
	ob->ticks = game->ticks;
	ob->cleared = 0;
	this = FlappyRand(game) % ARRAY_COUNT(yArray);
	
	/* don't accept the same value more than twice in a row */
	if (this == last[0] && this == last[1])
	{
		/* select whichever has been least common so far */
		int common[3] = {0};
		unsigned *i;
		
		/* count how common each value has been lately */
		for (i = last; i < last + ARRAY_COUNT(last); ++i)
			if (*i < ARRAY_COUNT(common))
				common[*i] += 1;
		
		/* 0 is least common */
		if (common[0] < common[1] && common[0] < common[2])
			this = common[0];
		
		/* 1 is least common */
		else if (common[1] < common[0] && common[1] < common[2])
			this = common[1];
		
		/* 2 is least common */
		else
			this = common[2];
		
		/* failsafe in case it still selected the same index */
		if (this == last[0])
		{
			switch (this)
			{
				case HIGH: /* upper -> lower */
					this = LOW;
					break;
				case LOW: /* lower -> upper */
					this = HIGH;
					break;
				default: /* center -> either upper or lower */
					this = (FlappyRand(game) & 1) ? HIGH : LOW;
					break;
			}
		}
		
		this %= ARRAY_COUNT(yArray);
	}
	
	/* jabu-specific gimmick */
	if (game->theme == FLAPPY_THEME_JABU)
	{
		/* when jabu stage hazard is active, select only upper */
		if (game->jabuHazardActive)
			this = HIGH;
		/* make lower more common to offset frequency of upper */
		else
			this = (FlappyRand(game) & 1) ? MEDIUM : LOW;
	}
	
	memmove(last + 1, last, sizeof(last) - sizeof(last[0]));
	last[0] = this;
	
	ob->y = yArray[this];
}

void ObstacleResetAll(struct Flappy *game)
{
	struct Obstacle *ob;
	
	for (ob = game->obstacleList; ob; ob = ob->next)
		ob->expired = 1;
}

void ObstacleUpdateAll(struct Flappy *game)
{
	struct Obstacle *ob;
	int rightmost = 0;
	
	for (ob = game->obstacleList; ob; ob = ob->next)
	{
		SDL_Rect hi;
		SDL_Rect lo;
		
		/* skip any that aren't being used */
		if (ob->expired)
			continue;
		
		ob->x = WINDOW_W + OB_W;
		ob->x -= WORLD_SCROLL(game->ticks - ob->ticks);
		
		if (ob->x > rightmost)
			rightmost = ob->x;
		
		/* reuse any that scroll off the screen */
		if (ob->x < -OB_W)
		{
			ob->expired = 1;
			continue;
		}
		
		/* player made it through this obstacle */
		if (game->state == FLAPPY_STATE_PLAYING
			&& !ob->cleared
			&& ob->x + OB_W < PlayerGetX(game->player)
		)
		{
			ob->cleared = 1;
			game->score += 1;
		}
		
		hi = (SDL_Rect){ob->x, ob->y - (OB_GAP / 2 + OB_H), OB_W, OB_H};
		lo = (SDL_Rect){ob->x, ob->y + OB_GAP / 2, OB_W, OB_H};
		
		ob->upper = hi;
		ob->lower = lo;
		
		/* collision */
		ColliderArenaPush(game, 0, 0, COLOR_WORLD, ColliderInitRect(game, ob->x, hi.y, hi.w, hi.h));
		ColliderArenaPush(game, 0, 0, COLOR_WORLD, ColliderInitRect(game, ob->x, lo.y, lo.w, FLOOR_Y - lo.y));
	}
	
	/* spawn another */
	if (rightmost < WINDOW_W - OB_DIST)
		ObstaclePush(game);
}

void ObstacleDrawAll(struct Flappy *game)
{
	struct Obstacle *ob;
	
	for (ob = game->obstacleList; ob; ob = ob->next)
	{
		SDL_Rect clip = {game->theme * OB_W, 0, OB_W, OB_H};
		
		/* skip any that aren't being used */
		if (ob->expired)
			continue;
		
		/* display, easy */
		TextureDraw(game, game->obstacles, clip, ob->x, ob->lower.y);
		
		/* adjust clipping rectangle and display mirrored version */
		clip.y += OB_H;
		TextureDraw(game, game->obstacles, clip, ob->x, ob->upper.y);
	}
}

void ObstacleCleanup(struct Flappy *game)
{
	struct Obstacle *ob;
	struct Obstacle *next = 0;
	
	for (ob = game->obstacleList; ob; ob = next)
	{
		next = ob->next;
		free(ob);
	}
	
	game->obstacleList = 0;
}

