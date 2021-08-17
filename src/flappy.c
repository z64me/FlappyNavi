/*
 * flappy.c <z64.me>
 *
 * main gameplay functions reside here
 *
 */

#include "common.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* _WIN32 */

#include <time.h>
#include "rnd.h"

/******************************
 *
 * private types and functions
 *
 ******************************/

/* set window icon */
static void SetWindowIcon(struct Flappy *game)
{
	assert(game);
	
#ifdef _WIN32
	SDL_SysWMinfo info;
	HWND win;
	
	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(game->window, &info))
		return;
	
	if (!(win = info.info.win.window))
		return;
	
	SendMessage(
		win
		, WM_SETICON
		, ICON_BIG
		, (LPARAM)LoadImage(
				GetModuleHandle(NULL)
				, MAKEINTRESOURCE(1) /* 1 is from my icon.rc */
				, IMAGE_ICON
				, 32
				, 32
				, 0
			)
	);
	SendMessage(
		win
		, WM_SETICON
		, ICON_SMALL
		, (LPARAM)LoadImage(
				GetModuleHandle(NULL)
				, MAKEINTRESOURCE(1) /* 1 is from my icon.rc */
				, IMAGE_ICON
				, 16
				, 16
				, 0
			)
	);
#else
	SDL_SetWindowIcon(game->window, SurfaceLoad(game, "gfx/icon.png"));
#endif
}

/******************************
 *
 * public functions
 *
 ******************************/

/* display a fatal error message in a popup window */
void FlappyFatal(const char *fmt, ...)
{
	char buf[256];
	va_list args;
	
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	fprintf(stderr, "%s\n", buf);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Flappy Navi Error", buf, 0);
	va_end(args);
	
	exit(EXIT_FAILURE);
}

/* returns the maximum scaling factor a window should use */
unsigned FlappyGetWindowMaxSize(struct Flappy *game)
{
	SDL_DisplayMode dm;
	
	assert(game);
	assert(game->window);
	
	if (SDL_GetCurrentDisplayMode(0, &dm))
		SDL_ERR("SDL_GetCurrentDisplayMode");
	
	if (dm.w > dm.h)
		return dm.w / WINDOW_W - 1;
	else
		return dm.h / WINDOW_H - 1;
	
	(void)game;
}

/* increment (1) or decrement (-1) window size (0 = simple refresh) */
void FlappyUpdateWindowSize(struct Flappy *game, int n)
{
	struct Input *input;
	unsigned scaleMax;
	unsigned scale;
	
	assert(game);
	assert(n == 1 || n == -1 || n == 0);
	
	scaleMax = FlappyGetWindowMaxSize(game);
	
	input = &game->input;
	
	game->scale += n;
	
	/* keep values in this range */
	if (game->scale < 1)
		game->scale = 1;
	else if (game->scale > scaleMax)
		game->scale = scaleMax;
	
	scale = game->scale;
	
	/* synchronize cursor across window resizes */
	SDL_WarpMouseInWindow(game->window, input->mouseX * scale, input->mouseY * scale);
	
	SDL_SetWindowSize(game->window, WINDOW_W * scale, WINDOW_H * scale);
}

/* allocate and initialize a gameplay state */
struct Flappy *FlappyNew(void)
{
	struct Flappy *game = calloc(1, sizeof(*game));
	
	/* release mode welcome message */
	#ifdef NDEBUG
	SDL_ShowSimpleMessageBox(
		SDL_MESSAGEBOX_INFORMATION
		, GAME_NAME
		, GAME_NAME
		"\n - Join Navi the fairy for an adventure across Hyrule!"
		"\n - by "GAME_AUTHOR
		"\n - "GAME_VERSION
		"\n\n""Controls:"
		"\n - F1 : Shrink Window"
		"\n - F2 : Expand Window"
		"\n - F12 : Toggle Debugging Options"
		"\n - Mouse Click : Flap"
		"\n\n""Many thanks to Master Yoshi for the awesome Navi sprites!"
		"\n\n""Find the source code on my website: https://z64.me/"
		, 0
	);
	#endif
	
	if (!game)
		FlappyFatal("memory error");
	
	if (SDL_Init(SDL_INIT_EVERYTHING))
		SDL_ERR("SDL_Init");
	
	if (!(game->window = SDL_CreateWindow(
		WINDOW_TITLE
		, SDL_WINDOWPOS_CENTERED
		, SDL_WINDOWPOS_CENTERED
		, WINDOW_W * WINDOW_SCALE
		, WINDOW_H * WINDOW_SCALE
		, SDL_WINDOW_SHOWN
	)))
		SDL_ERR("SDL_CreateWindow");
	
	if (!(game->renderer = SDL_CreateRenderer(
		game->window
		, -1
		, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	)))
		SDL_ERR("SDL_CreateRenderer");
	
	game->backgrounds = TextureLoad(game, "gfx/backgrounds.png");
	game->obstacles = TextureLoad(game, "gfx/obstacles.png");
	game->particles = TextureLoad(game, "gfx/particles.png");
	game->jabu = TextureLoad(game, "gfx/jabu.png");
	game->sprites = SpritesheetLoad(game, "gfx/sprites.png");
	game->ui = SpritesheetLoad(game, "gfx/ui.png");
	
	if (!(game->player = PlayerNew(game)))
		FlappyFatal("memory error");
	
	/* default window size */
	game->scale = WINDOW_SCALE;
	FlappyUpdateWindowSize(game, 0);
	
	/* random seed */
	if (!(game->rnd_pcg = malloc(sizeof(rnd_pcg_t))))
		FlappyFatal("memory error");
	rnd_pcg_seed(game->rnd_pcg, time(0));
	
	/* create timer */
	if (!(game->timer = TimerNew(game)))
		FlappyFatal("memory error");
	
	/* set up cursor */
	SDL_WarpMouseInWindow(game->window, WINDOW_W * WINDOW_SCALE * 0.75f, (WINDOW_H / 2) * WINDOW_SCALE);
	SDL_ShowCursor(0);
	
	/* set window icon */
	SetWindowIcon(game);
	
	return game;
}

uint32_t FlappyRand(struct Flappy *game)
{
	assert(game);
	assert(game->rnd_pcg);
	
	return rnd_pcg_next(game->rnd_pcg);
}

/* deallocate a gameplay state */
int FlappyFree(struct Flappy *game)
{
	assert(game);
	
	TextureFree(game, game->backgrounds);
	TextureFree(game, game->obstacles);
	TextureFree(game, game->particles);
	TextureFree(game, game->jabu);
	SpritesheetFree(game, game->sprites);
	SpritesheetFree(game, game->ui);
	
	ObstacleCleanup(game);
	ParticleCleanup(game);
	PlayerFree(game->player);
	TimerFree(game->timer);
	
	SDL_DestroyRenderer(game->renderer);
	SDL_DestroyWindow(game->window);
	SDL_Quit();
	
	free(game);
	
	return 0;
}

/* update a gameplay state */
void FlappyUpdate(struct Flappy *game)
{
	assert(game);
	
	/* reduce CPU usage when window is minimized */
	if (game->windowMinimized)
		SDL_Delay(10);
	
	/* handle game timers */
	TimerAdvance(game->timer, game->paused);
	game->ticks = TimerGetTicks(game->timer);
	game->stateTicks = game->ticks - game->stateStartTime;
	game->themeTicks = game->ticks - game->themeStartTime;
	
	/* lock player position on title screen */
	if (game->state == FLAPPY_STATE_TITLE)
		PlayerInit(game, game->player);
	
	/* initialize collision arena */
	ColliderArenaInit(game);
	
	/* ceiling */
	ColliderArenaPush(game, 0, 0, COLOR_WORLD, ColliderInitRect(game, 0, -WINDOW_H, WINDOW_W, WINDOW_H));
	
	/* floor */
	ColliderArenaPush(game, 0, 0, COLOR_WORLD, ColliderInitRect(game, 0, FLOOR_Y, WINDOW_W, WINDOW_H));
	
	/* other hazards */
	WorldDoHazards(game);
	
	/* obstacles appear in every mode except title screen */
	if (game->state != FLAPPY_STATE_TITLE)
		ObstacleUpdateAll(game);
	
	/* run main player function */
	PlayerUpdate(game, game->player);
	
	/* process all colliders registered during this frame */
	ColliderArenaProcess(game);
}

/* input wrapper */
void FlappyInput(struct Flappy *game)
{
	InputProcess(game);
}

/* display current gameplay frame */
void FlappyDraw(struct Flappy *game)
{
	/* draw the game world */
	WorldDraw(game);
	
	/* display colliders */
	if (game->debug & FLAPPY_DEBUG_COLLISION)
		ColliderArenaDraw(game, 0xffaaaaaa, 0x000000ff, 0xff);
	
	/* draw the title screen */
	UiDraw(game);
	
	/* display result to screen */
	SDL_RenderPresent(game->renderer);
}

/* (re)initialize gameplay */
void FlappyStartGame(struct Flappy *game)
{
	game->playerflapped = 0;
	game->paused = 0;
	game->score = 0;
	game->jabuHazardActive = 0;
	game->state = FLAPPY_STATE_PLAYING;
	PlayerInit(game, game->player);
	ObstacleResetAll(game);
	
	game->themeStartTime = game->stateStartTime = game->ticks;
	game->stateTicks = 0;
}

/* return to title screen */
void FlappyGoTitle(struct Flappy *game)
{
	FlappyStartGame(game);
	game->state = FLAPPY_STATE_TITLE;
	
	game->stateStartTime = game->ticks;
	game->stateTicks = 0;
}

/* set game over state */
void FlappyGameOver(struct Flappy *game)
{
	game->highscoreNew = 0;
	if (game->score > game->highscore)
	{
		game->highscore = game->score;
		game->highscoreNew = 1;
	}
	game->state = FLAPPY_STATE_GAMEOVER;
	game->paused = 0;
	
	game->stateStartTime = game->ticks;
	game->stateTicks = 0;
}

/* toggle pause/unpause */
void FlappyGamePause(struct Flappy *game)
{
	game->paused = !game->paused;
}

/* themes */
void FlappyNextTheme(struct Flappy *game)
{
	game->jabuHazardActive = 0;
	game->themeStartTime = game->ticks;
	game->theme += 1;
	game->theme %= FLAPPY_THEME_MAX;
}

