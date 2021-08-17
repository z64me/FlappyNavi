/*
 * file.c <z64.me>
 *
 * Flappy Navi's file loading functions reside here
 * (for static builds, a rudimentary virtual filesystem is employed)
 *
 */

#include "common.h"

#ifdef FLAPPY_STATIC_BUILD

#include "incbin.h"

struct VirtualFile
{
	const char *fn;
	const void *data;
	const unsigned int *sz;
};

#define VirtualFileDecl(NAME, FN) { \
  FN \
  , INCBIN_CONCATENATE( \
            INCBIN_CONCATENATE(INCBIN_PREFIX, NAME), \
            INCBIN_STYLE_IDENT(DATA)) \
  , &INCBIN_CONCATENATE( \
            INCBIN_CONCATENATE(INCBIN_PREFIX, NAME), \
            INCBIN_STYLE_IDENT(SIZE)) \
}

/* produce symbols for each */
INCBIN(Backgrounds, "gfx/backgrounds.png");
INCBIN(Obstacles, "gfx/obstacles.png");
INCBIN(Particles, "gfx/particles.png");
INCBIN(Sprites, "gfx/sprites.png");
INCBIN(Jabu, "gfx/jabu.png");
INCBIN(Icon, "gfx/icon.png");
INCBIN(Ui, "gfx/ui.png");

/* now reference those symbols in an array for easy lookup */
static const struct VirtualFile Filesystem[] = {
	VirtualFileDecl(Backgrounds, "gfx/backgrounds.png")
	, VirtualFileDecl(Obstacles, "gfx/obstacles.png")
	, VirtualFileDecl(Particles, "gfx/particles.png")
	, VirtualFileDecl(Sprites, "gfx/sprites.png")
	, VirtualFileDecl(Jabu, "gfx/jabu.png")
	, VirtualFileDecl(Icon, "gfx/icon.png")
	, VirtualFileDecl(Ui, "gfx/ui.png")
};

/* load a file */
void *FileLoad(const char *fn, size_t *sz)
{
	const struct VirtualFile *v;
	const struct VirtualFile *FilesystemEnd = Filesystem + ARRAY_COUNT(Filesystem);
	void *dat = 0;
	
	/* for every virtual file */
	for (v = Filesystem; v < FilesystemEnd; ++v)
		/* filename match found */
		if (!strcmp(fn, v->fn))
			break;
	
	/* rudimentary error checking returns 0 on any error */
	if (
		!fn
		|| !sz
		|| v == FilesystemEnd /* no match found */
		|| !(*sz = *v->sz)
		|| !(dat = malloc(*sz)) /* return a copy in case user wants to edit it */
		|| !memcpy(dat, v->data, *sz)
	)
		FlappyFatal("failed to load file '%s'", fn);
	
	return dat;
}

#else /* FLAPPY_STATIC_BUILD */

/* load a file */
void *FileLoad(const char *fn, size_t *sz)
{
	FILE *fp;
	void *dat = 0;
	
	/* rudimentary error checking returns 0 on any error */
	if (
		!fn
		|| !sz
		|| !(fp = fopen(fn, "rb"))
		|| fseek(fp, 0, SEEK_END)
		|| !(*sz = ftell(fp))
		|| fseek(fp, 0, SEEK_SET)
		|| !(dat = malloc(*sz))
		|| fread(dat, 1, *sz, fp) != *sz
		|| fclose(fp)
	)
		FlappyFatal("failed to load file '%s'", fn);
	
	return dat;
}

#endif /* ! FLAPPY_STATIC_BUILD */

/* clean up a data block loaded with FileLoad */
void FileFree(void *data)
{
	free(data);
}

