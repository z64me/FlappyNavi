/*
 * texture.c <z64.me>
 *
 * image loading geared towards SDL_Textures
 *
 */

#include "common.h"
#include "stb_image.h"

/* convert raw pixel data to a texture */
SDL_Texture *TextureFromPixels(struct Flappy *game, const void *pix, int w, int h)
{
	SDL_Surface *surf;
	SDL_Texture *tex;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	const uint32_t fmt = SDL_PIXELFORMAT_RGBA32;
#else
	const uint32_t fmt = SDL_PIXELFORMAT_ABGR32;
#endif
	
	/* convert */
	surf = SDL_CreateRGBSurfaceWithFormatFrom((void*)pix, w, h, 32, 4 * w, fmt);
	if (!surf)
		FlappyFatal("SDL_CreateRGBSurfaceWithFormatFrom error: %s", SDL_GetError());
	tex = SDL_CreateTextureFromSurface(game->renderer, surf);
	if (!tex)
		FlappyFatal("SDL_CreateTextureFromSurface error: %s", SDL_GetError());
	
	/* set flags */
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	
	/* cleanup */
	SDL_FreeSurface(surf);
	
	return tex;
}

/* create a texture from a loaded image file */
SDL_Texture *TextureLoadFrom(struct Flappy *game, void *data, size_t sz)
{
	SDL_Texture *tex;
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
	tex = TextureFromPixels(game, pix, w, h);
	
	/* cleanup */
	free(pix);
	
	return tex;
}

/* load a texture from a filename */
SDL_Texture *TextureLoad(struct Flappy *game, const char *filename)
{
	SDL_Texture *tex;
	size_t sz;
	void *data;
	
	assert(game);
	assert(filename);
	
	/* load, process, and free image data */
	data = FileLoad(filename, &sz);
	tex = TextureLoadFrom(game, data, sz);
	FileFree(data);
	
	return tex;
}

/* deallocate a texture */
void TextureFree(struct Flappy *game, SDL_Texture *tex)
{
	assert(game);
	assert(tex);
	
	SDL_DestroyTexture(tex);
	
	(void)game;
}

/* display a texture onto the screen */
void TextureDraw(struct Flappy *game, SDL_Texture *tex, SDL_Rect clip, float x, float y)
{
	SDL_Rect dst;
	
	assert(game);
	assert(tex);
	
	dst.x = ROUNDING(x * game->scale);
	dst.y = ROUNDING(y * game->scale);
	dst.w = ROUNDING(clip.w * game->scale);
	dst.h = ROUNDING(clip.h * game->scale);
	
	SDL_RenderCopy(game->renderer, tex, &clip, &dst);
}

