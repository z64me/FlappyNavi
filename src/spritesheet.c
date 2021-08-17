/*
 * spritesheet.c <z64.me>
 *
 * this is my minimalistic sprite sheet implementation
 *
 * sprites are organized in rows, and control pixels
 * within the image itself are used to specify each
 * sprite's clipping rectangle
 *
 */

#include "common.h"
#include "stb_image.h"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	#define ALPHA(X) ((X) & 0xff000000) /* abgr8888 */
#else
	#define ALPHA(X) ((X) & 0x000000ff) /* rgba8888 */
#endif

struct Sprite
{
	unsigned x;             /* x clip within row */
	unsigned w;             /* width of sprite */
	unsigned h;             /* height of sprite */
};

struct Row
{
	struct Sprite *sprite;  /* array of sprites */
	unsigned spriteNum;     /* number of sprites in row */
	unsigned y;             /* y clip for all sprites within row */
	unsigned h;             /* height of row */
};

struct Spritesheet
{
	SDL_Texture *tex;       /* texture image containing graphics */
	struct Row  *row;       /* array of rows */
	unsigned     rowNum;    /* number of rows */
};

/* get a sprite's clipping rectangle */
static void SpriteGetClipRect(struct Row *row, struct Sprite *sprite, int *x, int *y, int *w, int *h)
{
	*x = sprite->x;
	*w = sprite->w;
	*y = row->y;
	*h = sprite->h;
}

/* derive a sprite sheet from raw pixel data */
struct Spritesheet *SpritesheetFromPixels(struct Flappy *game, const void *pix, int w, int h)
{
	struct Spritesheet *sheet;
	struct Sprite *sprite;
	struct Row *row;
	const uint32_t *pix32 = pix;
	const uint32_t *p = pix;
	unsigned lastRowY = 0;
	
	assert(game);
	assert(p);
	assert(w);
	assert(h);
	
	/* allocate output structure */
	if (!(sheet = calloc(1, sizeof(*sheet))))
		FlappyFatal("memory error");
	
	sheet->tex = TextureFromPixels(game, pix, w, h);
	
	/* first control pixel should have a transparent
	 * pixel to its right, and below it; otherwise,
	 * it isn't following my sprite sheet spec
	 */
	if (!ALPHA(*p) || ALPHA(*(p+1)) || ALPHA(*(p + w)))
		FlappyFatal("invalid sprite sheet (pixel values %08x %08x %08x)", *p, p[1], p[w]);
	
	/* count number of rows (opaque pixels on left edge) */
	for (p = pix32; p < pix32 + w * h; p += w)
		if (ALPHA(*p))
			sheet->rowNum += 1;
	
	/* allocate rows */
	sheet->row = calloc(sheet->rowNum, sizeof(*sheet->row));
	if (!sheet->row)
		FlappyFatal("memory error");
	
	/* each row contains at least one sprite */
	for (row = sheet->row; row < sheet->row + sheet->rowNum; ++row)
		row->spriteNum = 1;
	
	/* collect information about each row */
	for (row = sheet->row, p = pix32; row < sheet->row + sheet->rowNum; )
	{
		/* first pixel on row */
		if (!((p - pix32) % w))
		{
			/* control pixel is present */
			if (ALPHA(*p))
			{
				int thisRowY = (p - pix32) / w;
				
				/* advance row except for first */
				if (p != pix32)
				{
					row->h = thisRowY - lastRowY;
					++row;
					if (row >= sheet->row + sheet->rowNum)
						break;
					row->y = thisRowY;
				}
				lastRowY = row->y;
				++p;
			}
			else
			{
				/* skip row */
				p += w;
				continue;
			}
		}
		
		/* control pixels specifying widths */
		else if (ALPHA(*p))
		{
			row->spriteNum += 1;
		}
		++p;
	}
	
	/* the last row's height is the remainder of the image */
	--row;
	row->h = h - row->y;
	
	/* allocate sprites for each row */
	for (row = sheet->row; row < sheet->row + sheet->rowNum; ++row)
		if (!(row->sprite = calloc(row->spriteNum, sizeof(*row->sprite))))
			FlappyFatal("memory error");
	
	/* derive width of each sprite in each row */
	for (row = sheet->row; row < sheet->row + sheet->rowNum; ++row)
	{
		unsigned lastSpriteX = 0;
		
		/* step through control pixel row, deriving widths */
		sprite = row->sprite;
		sprite->x = 1;
		for (p = pix32 + row->y * w + 1; p < pix32 + (row->y + 1) * w; ++p)
		{
			if (ALPHA(*p))
			{
				int thisSpriteX = (p - pix32) % w;
				
				sprite->w = thisSpriteX - lastSpriteX;
				++sprite;
				if (sprite >= row->sprite + row->spriteNum)
					break;
				sprite->x = thisSpriteX + 1;
				lastSpriteX = thisSpriteX;
			}
			
			/* no control pixel found; last sprite's width = remaining image width */
			if (!sprite->w)
				sprite->w = w - sprite->x;
		}
		
		/* update pixel coordinates to reflect pixel data in row */
		row->y += 1;
		row->h -= 1;
	}
	
	/* derive height of each sprite in each row */
	for (row = sheet->row; row < sheet->row + sheet->rowNum; ++row)
	{
		for (sprite = row->sprite; sprite < row->sprite + row->spriteNum; ++sprite)
		{
			unsigned y;
			
			/* process each row, starting with the lowest and working upwards */
			for (y = row->y + (row->h - 1); y > row->y; --y)
			{
				const uint32_t *pEnd;
				int rowIsBlank = 1;
				
				/* step to first pixel of sprite */
				p = pix32 + y * w + sprite->x;
				
				/* look for solid pixels in row */
				for (pEnd = p + sprite->w; p < pEnd; ++p)
				{
					if (ALPHA(*p))
					{
						rowIsBlank = 0;
						break;
					}
				}
				
				/* found a row containing solid pixels */
				if (!rowIsBlank)
				{
					++y;
					break;
				}
			}
			
			sprite->h = y - row->y;
		}
	}
	
	#ifndef NDEBUG
	/*fprintf(stderr, "sprite sheet %p contains %d rows:\n", sheet, sheet->rowNum);
	for (row = sheet->row; row < sheet->row + sheet->rowNum; ++row)
	{
		fprintf(
			stderr
			, "  row %d (y=%d, h=%d) contains %d sprites:\n"
			, (int)(row - sheet->row)
			, row->y
			, row->h
			, row->spriteNum
		);
		
		for (sprite = row->sprite; sprite < row->sprite + row->spriteNum; ++sprite)
		{
			int clipX;
			int clipY;
			int clipW;
			int clipH;
			
			SpriteGetClipRect(row, sprite, &clipX, &clipY, &clipW, &clipH);
			fprintf(stderr, "    sprite %d : ", (int)(sprite - row->sprite));
			fprintf(stderr, "{ %d, %d, %d, %d }\n", clipX, clipY, clipW, clipH);
		}
	}*/
	#endif
	
	return sheet;
}

/* convert a sprite sheet from a loaded image file */
struct Spritesheet *SpritesheetLoadFrom(struct Flappy *game, void *data, size_t sz)
{
	struct Spritesheet *sheet;
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
	
	sheet = SpritesheetFromPixels(game, pix, w, h);
	
	/* cleanup */
	free(pix);
	
	return sheet;
}

/* load a sprite sheet from an image */
struct Spritesheet *SpritesheetLoad(struct Flappy *game, const char *filename)
{
	struct Spritesheet *sheet;
	size_t sz;
	void *data;
	
	assert(game);
	assert(filename);
	
	/* load, process, and free image data */
	data = FileLoad(filename, &sz);
	sheet = SpritesheetLoadFrom(game, data, sz);
	FileFree(data);
	
	return sheet;
}

/* deallocate a sprite sheet */
void SpritesheetFree(struct Flappy *game, struct Spritesheet *sheet)
{
	struct Row *row;
	
	assert(game);
	assert(sheet);
	
	/* cleanup each row's sprite array */
	for (row = sheet->row; row < sheet->row + sheet->rowNum; ++row)
		free(row->sprite);
	
	/* and then everything else */
	SDL_DestroyTexture(sheet->tex);
	free(sheet->row);
	free(sheet);
	
	(void)game;
}

/* display one of the sprites from a sprite sheet */
void SpritesheetDraw(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, float x, float y)
{
	struct Row *rowP;
	struct Sprite *spriteP;
	SDL_Rect dst;
	SDL_Rect clip;
	
	assert(game);
	assert(sheet);
	assert(row < sheet->rowNum);
	assert(col < sheet->row[row].spriteNum);
	
	rowP = &sheet->row[row];
	spriteP = &rowP->sprite[col];
	
	SpriteGetClipRect(rowP, spriteP, &clip.x, &clip.y, &clip.w, &clip.h);
	
	dst.x = ROUNDING(x * game->scale);
	dst.y = ROUNDING(y * game->scale);
	dst.w = ROUNDING(clip.w * game->scale);
	dst.h = ROUNDING(clip.h * game->scale);
	
	SDL_RenderCopy(game->renderer, sheet->tex, &clip, &dst);
}

/* display one of the sprites from a sprite sheet, with a custom scale */
void SpritesheetDrawScaled(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, float x, float y, float scale)
{
	struct Row *rowP;
	struct Sprite *spriteP;
	SDL_Rect dst;
	SDL_Rect clip;
	
	assert(game);
	assert(sheet);
	assert(row < sheet->rowNum);
	assert(col < sheet->row[row].spriteNum);
	
	rowP = &sheet->row[row];
	spriteP = &rowP->sprite[col];
	
	SpriteGetClipRect(rowP, spriteP, &clip.x, &clip.y, &clip.w, &clip.h);
	
	dst.x = ROUNDING(x * game->scale);
	dst.y = ROUNDING(y * game->scale);
	dst.w = ROUNDING(clip.w * scale);
	dst.h = ROUNDING(clip.h * scale);
	
	SDL_RenderCopy(game->renderer, sheet->tex, &clip, &dst);
}

/* guess the center and get the world positioning info for a sprite before drawing */
SDL_Rect SpritesheetGetCentered(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, int x, int y)
{
	struct Row *rowP;
	struct Sprite *spriteP;
	SDL_Rect dst;
	SDL_Rect clip;
	
	assert(game);
	assert(sheet);
	assert(row < sheet->rowNum);
	assert(col < sheet->row[row].spriteNum);
	
	rowP = &sheet->row[row];
	spriteP = &rowP->sprite[col];
	
	SpriteGetClipRect(rowP, spriteP, &clip.x, &clip.y, &clip.w, &clip.h);
	
	dst.x = x - clip.w / 2;
	dst.y = y - clip.h / 2;
	dst.w = clip.w;
	dst.h = clip.h;
	
	return dst;
	
	(void)game;
}

/* display one of the sprites from a sprite sheet, guessing the center */
void SpritesheetDrawCentered(struct Flappy *game, struct Spritesheet *sheet, unsigned row, unsigned col, float x, float y)
{
	struct Row *rowP;
	struct Sprite *spriteP;
	SDL_Rect dst;
	SDL_Rect clip;
	
	assert(game);
	assert(sheet);
	assert(row < sheet->rowNum);
	assert(col < sheet->row[row].spriteNum);
	
	rowP = &sheet->row[row];
	spriteP = &rowP->sprite[col];
	
	SpriteGetClipRect(rowP, spriteP, &clip.x, &clip.y, &clip.w, &clip.h);
	
	dst.x = ROUNDING((x - clip.w / 2) * game->scale);
	dst.y = ROUNDING((y - clip.h / 2) * game->scale);
	dst.w = ROUNDING(clip.w * game->scale);
	dst.h = ROUNDING(clip.h * game->scale);
	
	SDL_RenderCopy(game->renderer, sheet->tex, &clip, &dst);
}

/* returns pointer to texture image that a sprite sheet uses */
SDL_Texture *SpritesheetGetTexture(struct Spritesheet *sheet)
{
	assert(sheet);
	
	return sheet->tex;
}

