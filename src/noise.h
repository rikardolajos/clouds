#ifndef NOISE_H
#define NOISE_H

float fbm_value(float x, float y, int octaves, float frequency, float lacunarity, float amplitude, float gain);
float fbm_perlin(float x, float y, float z, int octaves, float frequency, float lacunarity, float amplitude, float gain);
float fbm_worley(float x, float y, float z, int octaves, float frequency, float lacunarity, float amplitude, float gain);



#endif