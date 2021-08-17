/*
 * color.c <z64.me>
 *
 * color conversion functions
 *
 */

#include "common.h"

#define CLAMP01(x) ( (x) < 0 ? 0 : ( x > 1 ? 1 : (x) ) )

/* hsv to rgb implementation adapted from the code here:
 * https://github.com/stolk/hsvbench
 */
void HsvToRgb(float h, float s, float v, float *r, float *g, float *b)
{
	const float h6 = 6.0f * h;
	const float rC = fabsf( h6 - 3.0f ) - 1.0f;
	const float gC = 2.0f - fabsf( h6 - 2.0f );
	const float bC = 2.0f - fabsf( h6 - 4.0f );
	const float is = 1.0f - s;
	
	*r = v * ( s * CLAMP01(rC) + is );
	*g = v * ( s * CLAMP01(gC) + is );
	*b = v * ( s * CLAMP01(bC) + is );
}

void HsvToRgb8(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
	float rn;
	float gn;
	float bn;
	
	assert(r);
	assert(g);
	assert(b);
	
	HsvToRgb(h, s, v, &rn, &gn, &bn);
	
	*r = rn * 255;
	*g = gn * 255;
	*b = bn * 255;
}

