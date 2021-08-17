/*
 * timer.c <z64.me>
 *
 * basic game timer that supports pausing
 *
 */

#include "common.h"

struct Timer
{
	struct Flappy  *game;    /* pointer to game that created timer */
	double          ticks;   /* high resolution time calculation */
	uint64_t        prev;    /* previous time */
	uint64_t        now;     /* current time */
	uint64_t        start;   /* timer creation time */
};

/* allocate and initialize a new timer */
struct Timer *TimerNew(struct Flappy *game)
{
	struct Timer *timer = calloc(1, sizeof(*timer));
	
	if (!timer)
		return 0;
	
	timer->game = game;
	timer->start = SDL_GetPerformanceCounter();
	
	return timer;
}

void TimerFree(struct Timer *timer)
{
	assert(timer);
	
	free(timer);
}

void TimerAdvance(struct Timer *timer, int isPaused)
{
	if (!isPaused)
		timer->ticks += (double)((timer->now - timer->prev)*1000) / SDL_GetPerformanceFrequency();
	timer->prev = timer->now;
	timer->now = SDL_GetPerformanceCounter() - timer->start;
}

uint32_t TimerGetTicks(struct Timer *timer)
{
	return timer->ticks;
}

