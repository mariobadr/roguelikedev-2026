/**
 * @file lighting.h
 */
#ifndef GINC_ROGUELIKE_LIGHTING_H
#define GINC_ROGUELIKE_LIGHTING_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

/**
 * Calculate how bright a tile would be based on its distance from source.
 *
 * @param source The source of the light
 * @param tile   The tile impacted by the light
 * @param radius The light's radius.
 *
 * @return a value between 0.0 and 1.0.
 */
float
rl_calculate_brightness(SDL_Point source, SDL_Point tile, float radius);

/**
 * @return a new colour blended between two colours according to alpha.
 */
SDL_FColor
rl_lerp_colour(SDL_FColor a, SDL_FColor b, float alpha);

/**
 * @return a value blended between two values according to alpha.
 */
float
rl_lerp_float(float a, float b, float alpha);

#endif // GINC_ROGUELIKE_LIGHTING_H