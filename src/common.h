/*
 * common.h <z64.me>
 *
 * common Flappy Navi data structures
 * and function prototypes reside here
 *
 */

#ifndef FLAPPYNAVI_COMMON_H_INCLUDED
#define FLAPPYNAVI_COMMON_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/******************************
 *
 * common macros and constants
 *
 ******************************/

#define ARRAY_COUNT(X)    (sizeof((X)) / sizeof((X)[0]))
#define GAME_NAME         "Flappy Navi"
#define GAME_VERSION      "v1.0.0"
#define GAME_AUTHOR       "z64me <z64.me>"
#define WINDOW_TITLE      GAME_NAME
#define WINDOW_W          200
#define WINDOW_H          112
#define WINDOW_SCALE      4  /* default scaling factor */
#define SDL_ERR(X)        FlappyFatal(X " error: %s", SDL_GetError())
#define SCROLL_SPEED      32 /* how many pixels to scroll per second */
#define WORLD_SCROLL(X)   (((X) * 0.001f) * SCROLL_SPEED)
#define PLAYER_GRV        200 /* force due to gravity acting in player */
#define PLAYER_YVEL       -100 /* force applied to player when flapping */
#define PLAYER_PARTFREQ   100 /* frequency (milliseconds) to spawn particles */
#define ROUNDING(X)       roundf((float)X)
#define FLOOR_H           11 /* height of floor */
#define FLOOR_Y           (WINDOW_H - FLOOR_H) /* y loc of ground plane */
#define COLOR_WORLD       0xA0A0FF
#define COLOR_PLAYER      0x606000
#define GAMEOVER_TIME     1000 /* milliseconds before showing game over screen */
#define CLICK_BLINK       500  /* milliseconds before showing 'Click!' prompt */

/******************************
 *
 * private/opaque structures
 *
 ******************************/

struct Player;
struct Spritesheet;
struct Obstacle;
struct Particle;
struct Collider;
struct ColliderInit;
struct Timer;


/******************************
 *
 * public enumerations
 *
 ******************************/

enum FlappyTheme
{
	FLAPPY_THEME_FOREST
	, FLAPPY_THEME_MOUNTAIN
	, FLAPPY_THEME_JABU
	, FLAPPY_THEME_WATERTEMPLE
	, FLAPPY_THEME_DESERT
	, FLAPPY_THEME_TERMINA
	, FLAPPY_THEME_MAX
};

enum FlappyButton
{
	FLAPPY_BUTTON_PAUSE
	, FLAPPY_BUTTON_PLAY
	, FLAPPY_BUTTON_THEME
	, FLAPPY_BUTTON_RETRY
	, FLAPPY_BUTTON_QUIT
	, FLAPPY_BUTTON_MAX
};

enum FlappyMouse
{
	FLAPPY_MOUSE_OFF = 0
	, FLAPPY_MOUSE_HOVERING   = (1 << 0)
	, FLAPPY_MOUSE_CLICKING   = (1 << 1)
	, FLAPPY_MOUSE_CLICKED    = (1 << 3)
};

enum FlappyState
{
	FLAPPY_STATE_TITLE = 0  /* title screen */
	, FLAPPY_STATE_PLAYING  /* playing the game */
	, FLAPPY_STATE_GAMEOVER /* game over screen */
	, FLAPPY_STATE_MAX
};

enum FlappyDebug
{
	FLAPPY_DEBUG_OFF = 0
	, FLAPPY_DEBUG_GHOST = (1 << 0)
	, FLAPPY_DEBUG_COLLISION = (1 << 1)
	, FLAPPY_DEBUG_ALL = (FLAPPY_DEBUG_GHOST | FLAPPY_DEBUG_COLLISION)
};

enum ParticleType
{
	PARTICLE_SPARKLE_BLUE
	, PARTICLE_SPARKLE_GRAY
	, PARTICLE_SPARKLE_YELLOW
	, PARTICLE_SPARKLE_PURPLE
	, PARTICLE_DEATH
	, PARTICLE_MAX
};


/******************************
 *
 * public structures and types
 *
 ******************************/

struct Input
{
	unsigned  quit:1;       /* user wishes to exit */
	unsigned  space:1;      /* spacebar */
	unsigned  mouseDown:1;  /* mouse button press */
	unsigned  clicked:1;    /* mouse clicked (was pressed and released) */
	float     mouseX;       /* most recent cursor coordinates */
	float     mouseY;
	float     clickX;       /* cursor coordinates on press */
	float     clickY;
};

struct Flappy
{
	SDL_Window         *window;           /* window */
	SDL_Renderer       *renderer;         /* rendering context */
	SDL_Texture        *backgrounds;      /* backgrounds.png */
	SDL_Texture        *obstacles;        /* obstacles.png */
	SDL_Texture        *particles;        /* particles.png */
	SDL_Texture        *jabu;             /* jabu.png */
	struct Spritesheet *sprites;          /* sprites.png */
	struct Spritesheet *ui;               /* ui.png */
	struct Player      *player;           /* player game instance */
	struct Timer       *timer;            /* high resolution game timer */
	struct Obstacle    *obstacleList;     /* linked list of obstacles */
	struct Particle    *particleList;     /* linked list of particles */
	struct Collider    *colliderList;     /* linked list of colliders */
	void               *rnd_pcg;          /* randomness */
	struct Input        input;            /* game input structure */
	enum   FlappyTheme  theme;            /* selected game theme */
	enum   FlappyState  state;            /* game state */
	enum   FlappyDebug  debug;            /* debugging options */
	SDL_Rect            bgClip;           /* background clipping rectangle */
	unsigned            paused;           /* boolean value for pausing */
	unsigned            scale;            /* window scaling factor */
	unsigned            highscoreNew;     /* boolean 'New!' indicator */
	unsigned            highscore;        /* player's high score */
	unsigned            score;            /* player's current score */
	unsigned            buttonhover;      /* boolean tracking UI button hover */
	unsigned            playerflapped;    /* boolean whether player flapped yet */
	unsigned            jabuHazardActive; /* boolean tracking jabu stage hazard */
	unsigned            windowMinimized;  /* boolean tracking is window minimized */
	uint32_t            ticks;            /* milliseconds game has been running */
	uint32_t            stateTicks;       /* milliseconds game has been in current state */
	uint32_t            stateStartTime;   /* time of last state change */
	uint32_t            themeTicks;       /* milliseconds game using current theme */
	uint32_t            themeStartTime;   /* time of last theme change */
};

typedef void ColliderCallback(struct Flappy *game, void *instance);


/******************************
 *
 * public functions
 *
 ******************************/

/* files */
void *FileLoad(const char *fn, size_t *sz);
void FileFree(void *data);

/* surface image functions */
SDL_Surface *SurfaceFromPixels(struct Flappy *game, const void *pix, int w, int h);
SDL_Surface *SurfaceLoadFrom(struct Flappy *game, void *data, size_t sz);
SDL_Surface *SurfaceLoad(struct Flappy *game, const char *filename);

/* texture image functions */
SDL_Texture *TextureFromPixels(struct Flappy *game, const void *pix, int w, int h);
SDL_Texture *TextureLoadFrom(struct Flappy *game, void *data, size_t sz);
SDL_Texture *TextureLoad(struct Flappy *game, const char *filename);
void TextureDraw(struct Flappy *game, SDL_Texture *tex, SDL_Rect clip, float x, float y);
void TextureFree(struct Flappy *game, SDL_Texture *tex);

/* spritesheets */
struct Spritesheet *SpritesheetFromPixels(struct Flappy *game, const void *pix, int w, int h);
struct Spritesheet *SpritesheetLoadFrom(struct Flappy *game, void *data, size_t sz);
struct Spritesheet *SpritesheetLoad(struct Flappy *game, const char *filename);
void SpritesheetFree(struct Flappy *game, struct Spritesheet *sheet);
void SpritesheetDraw(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, float x, float y);
void SpritesheetDrawScaled(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, float x, float y, float scale);
void SpritesheetDrawCentered(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, float x, float y);
SDL_Texture *SpritesheetGetTexture(struct Spritesheet *sheet);
SDL_Rect SpritesheetGetCentered(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, int x, int y);

/* world */
void WorldDraw(struct Flappy *game);
void WorldDoHazards(struct Flappy *game);

/* backgrounds */
void BackgroundDrawFloor(struct Flappy *game);
void BackgroundDraw(struct Flappy *game);

/* obstacles */
void ObstaclePush(struct Flappy *game);
void ObstacleUpdateAll(struct Flappy *game);
void ObstacleResetAll(struct Flappy *game);
void ObstacleDrawAll(struct Flappy *game);
void ObstacleCleanup(struct Flappy *game);

/* particles */
void ParticlePush(struct Flappy *game, enum ParticleType, float x, float y);
void ParticleDrawAll(struct Flappy *game);
void ParticleCleanup(struct Flappy *game);

/* player */
struct Player *PlayerNew(struct Flappy *game);
void PlayerFree(struct Player *player);
void PlayerUpdate(struct Flappy *game, struct Player *player);
void PlayerDraw(struct Flappy *game, struct Player *player);
void PlayerSetPos(struct Player *player, float x, float y);
void PlayerInit(struct Flappy *game, struct Player *player);
float PlayerGetX(struct Player *player);
void PlayerGetCenter(struct Player *player, float *x, float *y);

/* user interface */
void UiDrawButton(struct Flappy *game, int x, int y, enum FlappyButton icon);
void UiDrawTitle(struct Flappy *game);
void UiDraw(struct Flappy *game);

/* input */
void InputProcess(struct Flappy *game);
enum FlappyMouse InputMouseInRect(struct Flappy *game, SDL_Rect rect);

/* primitive geometry */
void PrimitiveRect(struct Flappy *game, SDL_Rect r);
void PrimitiveRectOutline(struct Flappy *game, SDL_Rect r);

/* timer */
struct Timer *TimerNew(struct Flappy *game);
void TimerFree(struct Timer *timer);
void TimerAdvance(struct Timer *timer, int isPaused);
uint32_t TimerGetTicks(struct Timer *timer);

/* colors */
void HsvToRgb(float h, float s, float v, float *r, float *g, float *b);
void HsvToRgb8(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

/* collision */
int CollisionRectRect(const SDL_Rect a, const SDL_Rect b);
int CollisionPointRect(const int x, const int y, const SDL_Rect rect);

/* colliders */
void ColliderArenaInit(struct Flappy *game);
void ColliderArenaProcess(struct Flappy *game);
void ColliderArenaPush(struct Flappy *game, void *instance, ColliderCallback cb, const uint32_t color, const struct ColliderInit *init);
void ColliderArenaDraw(struct Flappy *game, uint32_t bgcolor, uint32_t outlinecolor, const int opacity);
const struct ColliderInit *ColliderInitRect(struct Flappy *game, const float x, const float y, const float w, const float h);

/* flappy game context */
void FlappyFatal(const char *fmt, ...);
struct Flappy *FlappyNew(void);
int FlappyFree(struct Flappy *game);
void FlappyUpdate(struct Flappy *game);
void FlappyInput(struct Flappy *game);
void FlappyDraw(struct Flappy *game);
unsigned FlappyGetWindowMaxSize(struct Flappy *game);
void FlappyUpdateWindowSize(struct Flappy *game, int n);
uint32_t FlappyRand(struct Flappy *game);
void FlappyStartGame(struct Flappy *game);
void FlappyGoTitle(struct Flappy *game);
void FlappyGameOver(struct Flappy *game);
void FlappyGamePause(struct Flappy *game);
void FlappyNextTheme(struct Flappy *game);

#endif /* FLAPPYNAVI_COMMON_H_INCLUDED */

