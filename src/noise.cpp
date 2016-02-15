#include "noise.h"

#include <math.h>

static float fade(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float t, float a, float b)
{
	return a + t * (b - a);
}

/* Value noise */
static float noise(int x, int y)
{
	float d = 12.9898 * x + 78.233 * y;
	float s = sin(d) * 43758.5453;
	return s - (int)s;
}

static float smooth_noise(int x, int y)
{
	float corners = (noise(x - 1, y - 1) + noise(x + 1, y - 1) + noise(x - 1, y + 1) + noise(x + 1, y + 1)) / 16;
	float sides = (noise(x - 1, y) + noise(x + 1, y) + noise(x, y - 1) + noise(x, y + 1)) / 8;
	float center = noise(x, y) / 4;
	return corners + sides + center;
}

static float interpolated_noise(float x, float y)
{
	int ix = int(x);
	int iy = int(y);
	float fx = x - ix;
	float fy = y - iy;

	float v1 = smooth_noise(ix, iy);
	float v2 = smooth_noise(ix + 1, iy);
	float v3 = smooth_noise(ix, iy + 1);
	float v4 = smooth_noise(ix + 1, iy + 1);

	float i1 = lerp(v1, v2, fx);
	float i2 = lerp(v3, v4, fx);

	return lerp(i1, i2, fy);
}

/* Perlin noise */
static int p[512] = { 151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };

/* Convert lo 4 bits of hash code into 12 gradient directions */
static float grad(int hash, float x, float y, float z)
{
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}


static float perlin_noise(float x, float y, float z)
{
	/* Find unit cube that contains point */
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int	Z = (int)floor(z) & 255;

	/* Find relative X, Y, Z of point in cube */
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	/* Compute fade curves for each X, Y, Z */
	float u = fade(x);
	float v = fade(y);
	float w = fade(z);

	/* Hash coordinates of the 8 cube corners */
	int A  = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B  = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	/* And add blended results from 8 corners of cube */
	return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
		grad(p[BA], x - 1, y, z)),
		lerp(u, grad(p[AB], x, y - 1, z),
		grad(p[BB], x - 1, y - 1, z))),
		lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
		grad(p[BA + 1], x - 1, y, z - 1)),
		lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
		grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

/* Worley noise */
static float worley_noise(float x, float y, float z)
{
	return 0;
}

/* Fractional Brownian motion for different noises */
float fbm_value(float x, float y, int octaves, float frequency, float lacunarity, float amplitude, float gain)
{
	float value = 0.0;
	for (int i = 0; i < octaves; i++) {
		value += interpolated_noise(x * frequency, y * frequency) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return value;
}

float fbm_perlin(float x, float y, float z, int octaves, float frequency, float lacunarity, float amplitude, float gain)
{
	float value = 0.0;
	for (int i = 0; i < octaves; i++) {
		value += perlin_noise(x * frequency, y * frequency, z * frequency) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return value;
}

float fbm_worley(float x, float y, float z, int octaves, float frequency, float lacunarity, float amplitude, float gain)
{
	float value = 0.0;
	for (int i = 0; i < octaves; i++) {
		value += worley_noise(x * frequency, y * frequency, z * frequency) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return value;
}
