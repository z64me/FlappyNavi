/*
 * main.c <z64.me>
 *
 * Flappy Navi main entry point
 *
 */

#include "common.h"

/* main loop
 * returns 0 when game is finished
 */
static int loop(struct Flappy *game)
{
	FlappyInput(game);
	
	/* game exit condition met */
	if (game->input.quit)
		return 0;
	
	/* update game state and entities */
	FlappyUpdate(game);
	
	/* draw everything */
	FlappyDraw(game);
	
	return 1;
}

int main(int argc, char *argv[])
{
	struct Flappy *game;
	
	/* initialize gameplay  */
	if (!(game = FlappyNew()))
		return -1;
	
	/* main loop */
	while (1)
	{
		if (!loop(game))
			break;
	}
	
	/* cleanup */
	if (FlappyFree(game))
		return -1;
	
	return 0;
	
	(void)argc;
	(void)argv;
}

