/*
 * ui.c <z64.me>
 *
 * user interface magic happens here
 *
 */

#include "common.h"

#define BTN_PLAY_OFS  2    /* amount to add to play button icon to make it center better */
#define NEW_BLINK     1000 /* milliseconds for blinking 'New!' high score indicator */

/* handles both processing and drawing a clickable interface button */
void UiDrawButton(struct Flappy *game, int x, int y, enum FlappyButton icon)
{
	/* rgb888 button color list */
	enum FlappyMouse mouse;
	SDL_Rect rect;
	uint32_t color = 0x9dd47d;
	uint8_t rOld, gOld, bOld;
	uint8_t r, g, b;
	int row = 0; /* which row in sprite sheet contains button sprites */
	int sprite = 1; /* which sprite in row */
	int clicking = 0; /* button using 'pressed' sprite */
	int clicked;
	
	assert(game);
	assert(game->theme < FLAPPY_THEME_MAX);
	
	rect = SpritesheetGetCentered(game, game->ui, row, sprite, x, y);
	mouse = InputMouseInRect(game, rect);
	if (mouse == (FLAPPY_MOUSE_CLICKING | FLAPPY_MOUSE_HOVERING))
		color -= 0x202020;
	else if (mouse & FLAPPY_MOUSE_HOVERING)
		color += 0x202020;
	if (mouse & FLAPPY_MOUSE_HOVERING)
		game->buttonhover = 1;
	
	/* display clicked version of button */
	if (mouse & FLAPPY_MOUSE_CLICKING)
	{
		++y;
		sprite += 1;
		clicking = 1;
	}
	r = color >> 16;
	g = color >>  8;
	b = color;
	
	/* draw button with color */
	SDL_GetTextureColorMod(SpritesheetGetTexture(game->ui), &rOld, &gOld, &bOld);
	SDL_SetTextureColorMod(SpritesheetGetTexture(game->ui), r, g, b);
	SpritesheetDrawCentered(game, game->ui, row, sprite, x, y);
	SDL_SetTextureColorMod(SpritesheetGetTexture(game->ui), rOld, gOld, bOld);
	
	/* draw icon on button */
	clicked = mouse & FLAPPY_MOUSE_CLICKED;
	if (!clicking) /* if button not using 'pressed' sprite, shift sprite up */
		y -= 1;
	switch (icon)
	{
		case FLAPPY_BUTTON_PAUSE:
			if (game->paused)
				SpritesheetDrawCentered(game, game->ui, 1, 0, x + BTN_PLAY_OFS, y);
			else
				SpritesheetDrawCentered(game, game->ui, 1, 1, x, y);
			if (clicked)
				FlappyGamePause(game);
			break;
		case FLAPPY_BUTTON_PLAY:
			SpritesheetDrawCentered(game, game->ui, 1, 0, x + BTN_PLAY_OFS, y);
			if (clicked)
				FlappyStartGame(game);
			break;
		
		case FLAPPY_BUTTON_THEME:
			SpritesheetDrawCentered(game, game->ui, 1, 2, x, y);
			if (clicked)
				FlappyNextTheme(game);
			break;
		
		case FLAPPY_BUTTON_RETRY:
			SpritesheetDrawCentered(game, game->ui, 1, 3, x, y);
			if (clicked)
				FlappyStartGame(game);
			break;
		
		case FLAPPY_BUTTON_QUIT:
			SpritesheetDrawCentered(game, game->ui, 1, 4, x, y);
			if (clicked)
				FlappyGoTitle(game);
			break;
		
		case FLAPPY_BUTTON_MAX:
			break;
	}
}

/* display a score */
int UiDrawScore(struct Flappy *game, unsigned score, int x, int y, int display)
{
	const char *c;
	char str[32];
	int totalWidth = 0;
	
	assert(game);
	
	if (score > 9999)
		score = 9999;
	
	snprintf(str, sizeof(str), "%d", score);
	
	for (c = str; *c; ++c)
	{
		int w;
		
		if (*c == '1')
			w = 10;
		else
			w = 15;
		
		if (display)
			SpritesheetDraw(game, game->ui, 1, 6 + (*c - '0'), x + totalWidth, y);
		
		totalWidth += w + 2;
	}
	
	return totalWidth;
}

/* get width (in pixels) of the score counter */
int UiGetScorePixelWidth(struct Flappy *game, unsigned score)
{
	return UiDrawScore(game, score, 0, 0, 0);
}

/* draw the game's user interface during the 'title' game state */
void UiDrawTitle(struct Flappy *game)
{
	int winCenterX = WINDOW_W / 2;
	int buttonY = 80;
	
	assert(game);
	
	/* logo */
	SpritesheetDrawCentered(game, game->ui, 0, 0, winCenterX, WINDOW_H / 5);
	
	/* author */
	SpritesheetDrawCentered(game, game->ui, 2, 0, winCenterX, WINDOW_H - 11);
	
	/* play button */
	UiDrawButton(game, winCenterX - 24, buttonY, FLAPPY_BUTTON_PLAY);
	
	/* theme button */
	UiDrawButton(game, winCenterX + 24, buttonY, FLAPPY_BUTTON_THEME);
}

/* draw the game's user interface during the 'playing' game state */
void UiDrawPlaying(struct Flappy *game)
{
	int width;
	int buttonY = WINDOW_H - 17;
	
	assert(game);
	
	/* score */
	width = UiGetScorePixelWidth(game, game->score);
	UiDrawScore(game, game->score, WINDOW_W - (width + 15), 15, 1);
	
	/* pause button */
	UiDrawButton(game, 23, buttonY, FLAPPY_BUTTON_PAUSE);
	
	/* theme button */
	UiDrawButton(game, WINDOW_W - 23, buttonY, FLAPPY_BUTTON_THEME);
	
	/* 'paused' overlay */
	if (game->paused)
		SpritesheetDrawCentered(game, game->ui, 2, 1, WINDOW_W / 2, WINDOW_H / 2);
	
	/* 'click!' overlay */
	else if (!game->playerflapped
		&& (game->stateTicks % (CLICK_BLINK * 2)) >= CLICK_BLINK
	)
		SpritesheetDrawCentered(game, game->ui, 2, 3, WINDOW_W / 2, WINDOW_H * 0.75f);
}

/* draw the game's user interface during the 'game over' game state */
void UiDrawGameOver(struct Flappy *game)
{
	int winCenterX = WINDOW_W / 2;
	int buttonY = 95;
	int scoreX = winCenterX + 57;
	
	assert(game);
	
	/* wait before showing game over screen */
	if (game->stateTicks < GAMEOVER_TIME)
		return;
	
	/* game over */
	SpritesheetDrawCentered(game, game->ui, 0, 3, winCenterX, 14);
	
	/* score */
	SpritesheetDraw(game, game->ui, 2, 2, winCenterX - 57, 32);
	UiDrawScore(game, game->score, scoreX - UiGetScorePixelWidth(game, game->score), 32, 1);
	
	/* best */
	SpritesheetDraw(game, game->ui, 0, 4, winCenterX - 57, 57);
	UiDrawScore(game, game->highscore, scoreX - UiGetScorePixelWidth(game, game->highscore), 57, 1);
	/* new! */
	if (game->highscoreNew && ((game->stateTicks - GAMEOVER_TIME) % (NEW_BLINK * 2)) < NEW_BLINK)
		SpritesheetDraw(game, game->ui, 2, 4, scoreX + 2, 57);
	
	/* retry button */
	UiDrawButton(game, winCenterX + 48, buttonY, FLAPPY_BUTTON_RETRY);
	
	/* theme button */
	UiDrawButton(game, winCenterX, buttonY, FLAPPY_BUTTON_THEME);
	
	/* quit button */
	UiDrawButton(game, winCenterX - 48, buttonY, FLAPPY_BUTTON_QUIT);
}

/* draw the game's user interface */
void UiDraw(struct Flappy *game)
{
	unsigned cursorScale = 1;
	
	assert(game);
	
	/* every UI redraw, this says "no buttons being hovered",
	 * and then as buttons are drawn, its value is updated,
	 * and by the end of the function, it is known whether
	 * the cursor is hovering over any UI buttons
	 */
	game->buttonhover = 0;
	
	switch (game->state)
	{
		case FLAPPY_STATE_TITLE:
			UiDrawTitle(game);
			break;
		
		case FLAPPY_STATE_PLAYING:
			UiDrawPlaying(game);
			break;
		
		case FLAPPY_STATE_GAMEOVER:
			UiDrawGameOver(game);
			break;
		
		case FLAPPY_STATE_MAX:
			break;
	}
	
	/* the game cursor is less distracting if it's smaller than other UI elements */
	cursorScale = game->scale / 2;
	if (!cursorScale)
		cursorScale = 1;
	SpritesheetDrawScaled(game, game->ui, 1, 5, game->input.mouseX, game->input.mouseY, cursorScale);
}

