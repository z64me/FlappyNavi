/*
 * surface.c <z64.me>
 *
 * image loading geared towards SDL_Surfaces
 *
 */

#include "common.h"
#include "stb_image.h"

/* convert raw pixel data to a surface */
SDL_Surface *SurfaceFromPixels(struct Flappy *game, const void *pix, int w, int h)
{
	SDL_Surface *surf;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	const uint32_t fmt = SDL_PIXELFORMAT_RGBA32;
#else
	const uint32_t fmt = SDL_PIXELFORMAT_ABGR32;
#endif
	
	surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, fmt);
	if (!surf)
		FlappyFatal("SDL_CreateRGBSurfaceWithFormat error: %s", SDL_GetError());
	
	if (SDL_MUSTLOCK(surf))
		SDL_LockSurface(surf);
	
	/* copy pixel data */
	memcpy(surf->pixels, pix, w * h * 4);
	
	if (SDL_MUSTLOCK(surf))
		SDL_UnlockSurface(surf);
	
	return surf;
	
	(void)game;
}

/* create a surface from a loaded image file */
SDL_Surface *SurfaceLoadFrom(struct Flappy *game, void *data, size_t sz)
{
	SDL_Surface *surf;
	void *pix;
	int w;
	int h;
	int comp;
	
	assert(game);
	assert(data);
	assert(sz);
	
	/* get rgba8888 pixel data from image */
	pix = stbi_load_from_memory(data, sz, &w, &h, &comp, STBI_rgb_alpha);
	if (!pix)
		FlappyFatal("image processing error");
	
	/* convert to texture image */
	surf = SurfaceFromPixels(game, pix, w, h);
	
	/* cleanup */
	free(pix);
	
	return surf;
	
	(void)game;
}

/* load a surface from a filename */
SDL_Surface *SurfaceLoad(struct Flappy *game, const char *filename)
{
	SDL_Surface *surf;
	size_t sz;
	void *data;
	
	assert(game);
	assert(filename);
	
	/* load, process, and free image data */
	data = FileLoad(filename, &sz);
	surf = SurfaceLoadFrom(game, data, sz);
	FileFree(data);
	
	return surf;
	
	(void)game;
}

