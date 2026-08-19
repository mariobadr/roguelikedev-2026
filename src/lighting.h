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
 * Return a new colour blended between two colours according to brightness.
 * 
 * @param brightest  the colour at its most bright
 * @param darkest    the colour at its darkest
 * @param brightness the factor of brightness
 * 
 * @return the blended colour
 */
SDL_FColor
rl_apply_brightness(SDL_FColor brightest, SDL_FColor darkest, float brightness);


#endif // GINC_ROGUELIKE_LIGHTING_H